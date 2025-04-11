#include "deck-recommend/base-deck-recommend.h"
#include "card-priority/card-priority-filter.h"
#include "common/timer.h"

template<typename T>
bool containsAny(const std::vector<T>& collection, const std::vector<T>& contains) {
    for (const auto& item : collection) {
        if (std::find(contains.begin(), contains.end(), item) != contains.end()) {
            return true;
        }
    }
    return false;
}


int piapro_unit_enum = mapEnum(EnumMap::unit, "piapro");
int challenge_live_type_enum = mapEnum(EnumMap::liveType, "challenge");



void BaseDeckRecommend::findBestCards(
    const std::vector<CardDetail> &cardDetails, 
    const std::vector<CardDetail> &allCards, 
    const std::function<int(const DeckDetail &)> &scoreFunc, 
    RecommendDeckDfsInfo& dfsInfo,
    int limit, 
    bool isChallengeLive, 
    int member, 
    int honorBonus, 
    std::optional<int> eventType,
    std::optional<int> eventId
)
{
    auto& deckCards = dfsInfo.deckCards;
    auto& deckCharacters = dfsInfo.deckCharacters;

    // 防止挑战Live卡的数量小于允许上场的数量导致无法组队
    if (isChallengeLive) {
        member = std::min(member, int(cardDetails.size()));
    }
    // 已经是完整卡组，计算当前卡组的值
    if (int(deckCards.size()) == member) {

        auto deckDetail = deckCalculator.getDeckDetailByCards(deckCards, allCards, honorBonus, eventType, eventId);

        auto score = scoreFunc(deckDetail);
        auto cards = deckDetail.cards;
        // 寻找加分效果最高的卡牌
        int bestScoreUp = cards[0].skill.scoreUp;
        int bestScoreIndex = 0;
        for (size_t i = 0; i < cards.size(); ++i) {
            if (cards[i].skill.scoreUp > bestScoreUp) {
                bestScoreUp = cards[i].skill.scoreUp;
                bestScoreIndex = i;
            }
        }
        // 如果现在C位已经对了（加分技能最高的卡牌在C位）
        if (bestScoreIndex == 0) {
            dfsInfo.update(RecommendDeck{deckDetail, score}, limit);
            return;
        }
        // 不然就重新算调整过C位后的分数
        std::swap(deckCards[0], deckCards[bestScoreIndex]);
        return findBestCards(
            cardDetails, allCards, scoreFunc, dfsInfo,
            limit, isChallengeLive, member, honorBonus, eventType, eventId
        );
    }

    // 非完整卡组，继续遍历所有情况
    std::optional<CardDetail> preCard = std::nullopt;

    for (const auto& card : cardDetails) {
        // 跳过已经重复出现过的卡牌
        bool has_card = false;
        for (const auto& deckCard : deckCards) {
            if (deckCard.cardId == card.cardId) {
                has_card = true;
                break;
            }
        }
        if (has_card) continue;
        // 跳过重复角色
        if (!isChallengeLive && deckCharacters.count(card.characterId)) continue;
        // C位一定是技能最好的卡牌，跳过技能比C位还好的
        if (deckCards.size() >= 1 && deckCards[0].skill.isCertainlyLessThan(card.skill)) continue;
        // 为了优化性能，必须和C位同色或同组
        if (deckCards.size() >= 1 && card.attr != deckCards[0].attr && !containsAny(deckCards[0].units, card.units)) {
            continue;
        }
        // 要求生成的卡组后面4个位置按强弱排序、同强度按卡牌ID排序
        // 如果上一张卡肯定小，那就不符合顺序；在旗鼓相当的前提下（因为两两组合有四种情况，再排除掉这张卡肯定小的情况，就是旗鼓相当），要ID大
        if (deckCards.size() >= 2 && this->cardCalculator.isCertainlyLessThan(deckCards[deckCards.size() - 1], card)) continue;
        if (deckCards.size() >= 2 && !this->cardCalculator.isCertainlyLessThan(card, deckCards[deckCards.size() - 1]) &&
            card.cardId < deckCards[deckCards.size() - 1].cardId) {
            continue;
        }
        // 如果肯定比上一次选定的卡牌要弱，那么舍去，让这张卡去后面再选
        if (preCard.has_value() && this->cardCalculator.isCertainlyLessThan(card, preCard.value())) continue;
        preCard = card;

        // 递归，寻找所有情况
        deckCards.push_back(card);
        deckCharacters.insert(card.characterId);
        findBestCards(
            cardDetails, allCards, scoreFunc, dfsInfo,
            limit, isChallengeLive, member, honorBonus, eventType, eventId
        );
        deckCards.pop_back();
        deckCharacters.erase(card.characterId);
    }
}

std::vector<RecommendDeck> BaseDeckRecommend::recommendHighScoreDeck(
    const std::vector<UserCard> &userCards, 
    ScoreFunction scoreFunc, 
    const DeckRecommendConfig &config, 
    int liveType, 
    const EventConfig &eventConfig
)
{
    auto musicMeta = this->liveCalculator.getMusicMeta(config.musicId, config.musicDiff);

    auto areaItemLevels = areaItemService.getAreaItemLevels();

    auto cards = cardCalculator.batchGetCardDetail(userCards, config.cardConfig, eventConfig, areaItemLevels);

    // 过滤箱活的卡，不上其它组合的
    if (eventConfig.eventUnit) {
        std::vector<CardDetail> newCards{};
        for (const auto& card : cards) {
            if ((card.units.size() == 1 && card.units[0] == piapro_unit_enum) || 
                std::find(card.units.begin(), card.units.end(), eventConfig.eventUnit) != card.units.end()) {
                newCards.push_back(card);
            }
        }
        cards = std::move(newCards);
    }
    
    auto honorBonus = deckCalculator.getHonorBonusPower();

    // 为了优化性能，会根据活动加成和卡牌稀有度优先级筛选卡牌
    std::vector<RecommendDeck> ans{};
    std::vector<CardDetail> preCardDetails{};
    RecommendDeckDfsInfo dfsInfo{};

    while (true) {
        auto cardDetails = filterCardPriority(liveType, eventConfig.eventType, cards, preCardDetails, config.member);
        if (cardDetails.size() == preCardDetails.size()) {
            // 如果所有卡牌都上阵了还是组不出队伍，就报错
            if (ans.empty())
                throw std::runtime_error("Cannot recommend any deck in " + std::to_string(cards.size()) + " cards");
            else
                return ans; // 返回上次组出的队伍
        }
        preCardDetails = cardDetails;
        auto cards0 = cardDetails;
        std::sort(cards0.begin(), cards0.end(), [](const CardDetail& a, const CardDetail& b) { return a.cardId < b.cardId; });

        dfsInfo.reset();

        findBestCards(
            cards0, cards, [&](const DeckDetail& deckDetail) { return scoreFunc(musicMeta, deckDetail); },
            dfsInfo,
            config.limit, liveType == challenge_live_type_enum, config.member, honorBonus, 
            eventConfig.eventType, eventConfig.eventId
        );

        ans.clear();
        while (dfsInfo.deckQueue.size()) {
            ans.emplace_back(dfsInfo.deckQueue.top());
            dfsInfo.deckQueue.pop();
        }
        std::reverse(ans.begin(), ans.end());
        if (int(ans.size()) >= config.limit) 
            return ans;
    }
}
