#include "deck-recommend/base-deck-recommend.h"
#include "card-priority/card-priority-filter.h"
#include "common/timer.h"
#include <chrono>
#include <random>
#include "base-deck-recommend.h"


int piapro_unit_enum = mapEnum(EnumMap::unit, "piapro");
int challenge_live_type_enum = mapEnum(EnumMap::liveType, "challenge");


long long BaseDeckRecommend::calcDeckHash(const std::vector<const CardDetail*>& deck) {
    std::vector<int> v{};
    for(auto& card : deck) 
        v.push_back(card->cardId);
    std::sort(v.begin() + 1, v.end());
    long long hash = 0;
    constexpr long long base = 10007;
    constexpr long long mod = 1e9 + 7;
    for (auto& x : v) {
        hash = (hash * base + x) % mod;
    }
    return hash;
};


/*
获取当前卡组的最佳排列
*/
RecommendDeck BaseDeckRecommend::getBestPermutation(
    DeckCalculator& deckCalculator,
    const std::vector<const CardDetail*> &deckCards,
    const std::vector<CardDetail> &allCards,
    const std::function<int(const DeckDetail &)> &scoreFunc,
    int honorBonus,
    std::optional<int> eventType,
    std::optional<int> eventId
) const {
    auto deck = deckCards;
    // 后几位按照cardId从小到大排序
    std::sort(deck.begin() + 1, deck.end(), [&](const CardDetail* a, const CardDetail* b) {
        return a->cardId < b->cardId;
    });
    // 计算当前卡组的分数
    auto deckDetail = deckCalculator.getDeckDetailByCards(deck, allCards, honorBonus, eventType, eventId);
    auto score = scoreFunc(deckDetail);
    auto cards = deckDetail.cards;
    // 寻找加分效果最高的卡牌
    int bestScoreIndex = std::max_element(cards.begin(), cards.end(), [](const DeckCardDetail& a, const DeckCardDetail& b) {
        return std::tuple(a.skill.scoreUp, a.cardId) < std::tuple(b.skill.scoreUp, b.cardId);
    }) - cards.begin();
    // 如果现在C位已经对了（加分技能最高的卡牌在C位）
    if (bestScoreIndex == 0) 
        return RecommendDeck{ deckDetail, score };
    // 不然就重新算调整过C位后的分数
    std::swap(deck[0], deck[bestScoreIndex]);
    // 重新排序
    std::sort(deck.begin() + 1, deck.end(), [&](const CardDetail* a, const CardDetail* b) {
        return a->cardId < b->cardId;
    });
    deckDetail = deckCalculator.getDeckDetailByCards(deck, allCards, honorBonus, eventType, eventId);
    score = scoreFunc(deckDetail);
    return RecommendDeck{ deckDetail, score };
}


std::vector<RecommendDeck> BaseDeckRecommend::recommendHighScoreDeck(
    const std::vector<UserCard> &userCards,
    ScoreFunction scoreFunc,
    const DeckRecommendConfig &config,
    int liveType,
    const EventConfig &eventConfig)
{
    auto musicMeta = this->liveCalculator.getMusicMeta(config.musicId, config.musicDiff);

    auto areaItemLevels = areaItemService.getAreaItemLevels();

    auto cards = cardCalculator.batchGetCardDetail(userCards, config.cardConfig, eventConfig, areaItemLevels);

    // 过滤箱活的卡，不上其它组合的
    if (eventConfig.eventUnit && config.filterOtherUnit) {
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

    std::vector<RecommendDeck> ans{};
    std::vector<CardDetail> cardDetails{};
    std::vector<CardDetail> preCardDetails{};
    auto sf = [&scoreFunc, &musicMeta](const DeckDetail& deckDetail) { return scoreFunc(musicMeta, deckDetail); };

    RecommendCalcInfo calcInfo{};
    calcInfo.start_ts = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    calcInfo.timeout = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(config.timeout_ms)).count();

    while (true) {
        if (config.algorithm == RecommendAlgorithm::DFS) {
            // DFS 为了优化性能，会根据活动加成和卡牌稀有度优先级筛选卡牌
            cardDetails = filterCardPriority(liveType, eventConfig.eventType, cards, preCardDetails, config.member);
        } else {
            // 如果使用随机化算法不需要过滤
            cardDetails = cards;
        }
        if (cardDetails.size() == preCardDetails.size()) {
            if (ans.empty())    // 如果所有卡牌都上阵了还是组不出队伍，就报错
                throw std::runtime_error("Cannot recommend any deck in " + std::to_string(cards.size()) + " cards");
            else    // 返回上次组出的队伍
                return ans; 
        }
        preCardDetails = cardDetails;
        auto cards0 = cardDetails;
        // 卡牌大致按强度排序，保证dfs先遍历强度高的卡组
        std::sort(cards0.begin(), cards0.end(), [](const CardDetail& a, const CardDetail& b) { 
            return std::make_tuple(a.power.max, a.power.min, a.cardId) > std::make_tuple(b.power.max, b.power.min, b.cardId);
        });

        if (config.algorithm == RecommendAlgorithm::SA) {
            // 使用模拟退火
            long long seed = config.saSeed;
            if (seed == -1) 
                seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    
            auto rng = Rng(seed);
            for (int i = 0; i < config.saRunCount && !calcInfo.isTimeout(); i++) {
                findBestCardsSA(
                    config, rng, cards0, cards, sf,
                    calcInfo,
                    config.limit, liveType == challenge_live_type_enum, config.member, honorBonus,
                    eventConfig.eventType, eventConfig.eventId
                );
            }
        } 
        else if (config.algorithm == RecommendAlgorithm::GA) {
            // 使用遗传算法
            long long seed = config.gaSeed;
            if (seed == -1) 
                seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();

            auto rng = Rng(seed);
            findBestCardsGA(
                config, rng, cards0, cards, sf,
                calcInfo,
                config.limit, liveType == challenge_live_type_enum, config.member, honorBonus,
                eventConfig.eventType, eventConfig.eventId
            );
        }
        else if (config.algorithm == RecommendAlgorithm::DFS) {
            // 使用DFS
            calcInfo.deckCards.clear();
            calcInfo.deckCharacters.clear();

            findBestCards(
                cards0, cards, sf,
                calcInfo,
                config.limit, liveType == challenge_live_type_enum, config.member, honorBonus, 
                eventConfig.eventType, eventConfig.eventId
            );
        }
        else {
            throw std::runtime_error("Unknown algorithm: " + std::to_string(int(config.algorithm)));
        }

        ans.clear();
        while (calcInfo.deckQueue.size()) {
            ans.emplace_back(calcInfo.deckQueue.top());
            calcInfo.deckQueue.pop();
        }
        std::reverse(ans.begin(), ans.end());
        if (int(ans.size()) >= config.limit || calcInfo.isTimeout()) 
            return ans;
    }
}
