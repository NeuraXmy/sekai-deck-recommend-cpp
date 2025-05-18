#include "deck-recommend/base-deck-recommend.h"


template<typename T>
bool containsAny(const std::vector<T>& collection, const std::vector<T>& contains) {
    for (const auto& item : collection) {
        if (std::find(contains.begin(), contains.end(), item) != contains.end()) {
            return true;
        }
    }
    return false;
}


void BaseDeckRecommend::findBestCardsDFS(
    int liveType,
    const DeckRecommendConfig& cfg,
    const std::vector<CardDetail> &cardDetails, 
    const std::vector<CardDetail> &allCards, 
    const std::function<int(const DeckDetail &)> &scoreFunc, 
    RecommendCalcInfo& dfsInfo,
    int limit, 
    bool isChallengeLive, 
    int member, 
    int honorBonus, 
    std::optional<int> eventType,
    std::optional<int> eventId,
    const std::vector<CardDetail>& fixedCards
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
        dfsInfo.update(
            getBestPermutation(
                this->deckCalculator, deckCards, allCards, scoreFunc, 
                honorBonus, eventType, eventId, cfg.target, liveType
            ), limit
        );
        return;
    }

    // 超时
    if (dfsInfo.isTimeout()) {
        return;
    }

    // 非完整卡组，继续遍历所有情况
    std::optional<CardDetail> preCard = std::nullopt;

    for (const auto& card : cardDetails) {
        // 跳过已经重复出现过的卡牌
        bool has_card = false;
        for (const auto& deckCard : deckCards) {
            if (deckCard->cardId == card.cardId) {
                has_card = true;
                break;
            }
        }
        if (has_card) continue;
        // 跳过重复角色
        if (!isChallengeLive && deckCharacters.count(card.characterId)) continue;
        
        // C位相关优化，如果使用固定卡牌，则认为C位是第一个不固定的位置，后面的同理（即固定卡牌不参加剪枝）
        auto cIndex = fixedCards.size();
        // C位一定是技能最好的卡牌，跳过技能比C位还好的
        if (deckCards.size() >= cIndex + 1 && deckCards[cIndex]->skill.isCertainlyLessThan(card.skill)) continue;
        // 为了优化性能，必须和C位同色或同组
        if (deckCards.size() >= cIndex + 1 && card.attr != deckCards[cIndex]->attr && !containsAny(deckCards[cIndex]->units, card.units)) {
            continue;
        }

        if (deckCards.size() >= cIndex + 2) {
            auto& last = *deckCards.back();
            bool lessThan = false;
            bool greaterThan = false;
            if (cfg.target == RecommendTarget::Score) {
                lessThan = this->cardCalculator.isCertainlyLessThan(card, last);
                greaterThan = this->cardCalculator.isCertainlyLessThan(last, card);
            } else if(cfg.target == RecommendTarget::Power) {
                lessThan = card.power.isCertainlyLessThan(last.power);
                greaterThan = last.power.isCertainlyLessThan(card.power);
            } else if (cfg.target == RecommendTarget::Skill) {
                lessThan = card.skill.isCertainlyLessThan(last.skill);
                greaterThan = last.skill.isCertainlyLessThan(card.skill);
            }
            // 要求生成的卡组后面4个位置按强弱排序、同强度按卡牌ID排序
            // 如果上一张卡肯定小，那就不符合顺序；在旗鼓相当的前提下（因为两两组合有四种情况，再排除掉这张卡肯定小的情况，就是旗鼓相当），要ID大
            if (lessThan || (!greaterThan && card.cardId > last.cardId)) {
                continue;
            }
        }
        
        if (preCard.has_value()) {
            auto& pre = preCard.value();
            bool lessThan = false;
            if (cfg.target == RecommendTarget::Score) {
                lessThan = this->cardCalculator.isCertainlyLessThan(card, pre);
            } else if (cfg.target == RecommendTarget::Power) {
                lessThan = card.power.isCertainlyLessThan(pre.power);
            } else if (cfg.target == RecommendTarget::Skill) {
                lessThan = card.skill.isCertainlyLessThan(pre.skill);
            }
            // 如果肯定比上一次选定的卡牌要弱，那么舍去，让这张卡去后面再选
            if (lessThan) {
                continue;
            }
        }
        preCard = card;

        // 递归，寻找所有情况
        deckCards.push_back(&card);
        deckCharacters.insert(card.characterId);
        findBestCardsDFS(
            liveType, cfg, cardDetails, allCards, scoreFunc, dfsInfo,
            limit, isChallengeLive, member, honorBonus, eventType, eventId, fixedCards
        );
        deckCards.pop_back();
        deckCharacters.erase(card.characterId);
    }
}