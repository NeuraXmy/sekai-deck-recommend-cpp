#include "deck-recommend/base-deck-recommend.h"
#include "card-priority/card-priority-filter.h"
#include "common/timer.h"
#include <chrono>
#include <random>
#include "base-deck-recommend.h"


static int piapro_unit_enum = mapEnum(EnumMap::unit, "piapro");
static int challenge_live_type_enum = mapEnum(EnumMap::liveType, "challenge");
static int multi_live_type_enum = mapEnum(EnumMap::liveType, "multi");
static int cheerful_live_type_enum = mapEnum(EnumMap::liveType, "cheerful");

static int not_doing_special_training_status = mapEnum(EnumMap::specialTrainingStatus, "not_doing");

static int world_bloom_type_enum = mapEnum(EnumMap::eventType, "world_bloom");


// 计算技能实效
double calcExpectSkillBonus(const DeckDetail& deckDetail, int liveType) {
    double ret = 0;
    auto& cards = deckDetail.cards;
    if (liveType == multi_live_type_enum || liveType == cheerful_live_type_enum) {
        // 多人Live实效
        ret += cards[0].skill.scoreUp;
        for (int i = 1; i < int(cards.size()); ++i) 
            ret += cards[i].skill.scoreUp * 0.2;
    } else {
        // 单人Live实效
        for (int i = 0; i < int(cards.size()); ++i) 
            ret += cards[i].skill.scoreUp;
    }
    return ret;
}


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
    std::optional<int> eventId,
    int liveType,
    const DeckRecommendConfig& config
) const {
    auto deck = deckCards;
    // 后几位按照cardId从小到大排序
    std::sort(deck.begin() + 1, deck.end(), [&](const CardDetail* a, const CardDetail* b) {
        return a->cardId < b->cardId;
    });
    // 计算当前卡组的分数
    auto deckDetail = deckCalculator.getDeckDetailByCards(
        deck, allCards, honorBonus, eventType, eventId, 
        config.skillReferenceChooseStrategy, config.keepAfterTrainingState
    );
    auto score = scoreFunc(deckDetail);
    auto cards = deckDetail.cards;
    // 寻找加分效果最高的卡牌
    int bestScoreIndex = std::max_element(cards.begin(), cards.end(), [](const DeckCardDetail& a, const DeckCardDetail& b) {
        return std::tuple(a.skill.scoreUp, a.cardId) < std::tuple(b.skill.scoreUp, b.cardId);
    }) - cards.begin();
    // 如果现在C位已经对了（加分技能最高的卡牌在C位）
    if (bestScoreIndex == 0) {
        double expectSkillBonus = calcExpectSkillBonus(deckDetail, liveType);
        return RecommendDeck(deckDetail, config.target, score, expectSkillBonus);
    }
    // 不然就重新算调整过C位后的分数
    std::swap(deck[0], deck[bestScoreIndex]);
    // 重新排序
    std::sort(deck.begin() + 1, deck.end(), [&](const CardDetail* a, const CardDetail* b) {
        return a->cardId < b->cardId;
    });
    deckDetail = deckCalculator.getDeckDetailByCards(
        deck, allCards, honorBonus, eventType, eventId, 
        config.skillReferenceChooseStrategy, config.keepAfterTrainingState
    );
    score = scoreFunc(deckDetail);
    double expectSkillBonus = calcExpectSkillBonus(deckDetail, liveType);
    return RecommendDeck(deckDetail, config.target, score, expectSkillBonus);
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

    auto cards = cardCalculator.batchGetCardDetail(
        userCards, config.cardConfig, config.singleCardConfig, 
        eventConfig, areaItemLevels
    );

    auto& cardEpisodes = this->dataProvider.masterData->cardEpisodes;

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

    // 获取固定卡牌
    std::vector<CardDetail> fixedCards{};
    for (auto card_id : config.fixedCards) {
        // 从当前卡牌中找到对应的卡牌
        auto it = std::find_if(cards.begin(), cards.end(), [&](const CardDetail& card) {
            return card.cardId == card_id;
        });
        if (it != cards.end()) {
            fixedCards.push_back(*it);
        } else {
            // 找不到的情况下，生成一个初始养成情况的卡牌
            UserCard uc;
            uc.cardId = card_id;
            uc.level = 1;
            uc.skillLevel = 1;
            uc.masterRank = 0;
            uc.specialTrainingStatus = not_doing_special_training_status;
            
            auto& c = findOrThrow(this->dataProvider.masterData->cards, [&](const Card& c) {return c.id == card_id;});
            bool hasSpecialTraining = c.cardRarityType == mapEnum(EnumMap::cardRarityType, "rarity_3")
                                    || c.cardRarityType == mapEnum(EnumMap::cardRarityType, "rarity_4");
            uc.defaultImage = mapEnum(EnumMap::defaultImage, hasSpecialTraining ? "special_training" : "original");

            for (auto& ep : cardEpisodes) 
                if (ep.cardId == card_id) {
                    UserCardEpisodes uce{};
                    uce.cardEpisodeId = ep.id;
                    uce.scenarioStatus = 0;
                    uc.episodes.push_back(uce);
                }
            auto card = cardCalculator.batchGetCardDetail(
                {uc}, config.cardConfig, config.singleCardConfig, 
                eventConfig, areaItemLevels
            );
            if (card.size() > 0) {
                fixedCards.push_back(card[0]);
            } else {
                throw std::runtime_error("Failed to generate virtual card for fixed card id: " + std::to_string(card_id));
            }
        }
    }
    // 检查固定卡牌是否有效
    if (fixedCards.size()) {
        std::set<int> fixedCardIds{};
        std::set<int> fixedCardCharacterIds{};
        for (const auto& card : fixedCards) {
            fixedCardIds.insert(card.cardId);
            fixedCardCharacterIds.insert(card.characterId);
        }
        if (int(fixedCards.size()) > config.member) {
            throw std::runtime_error("Fixed cards size is larger than member size");
        }
        if (fixedCardIds.size() != fixedCards.size()) {
            throw std::runtime_error("Fixed cards have duplicate cards");
        }
        if (liveType == challenge_live_type_enum) {
            if (fixedCardCharacterIds.size() != 1 || fixedCards[0].characterId != cards[0].characterId) {
                throw std::runtime_error("Fixed cards have invalid characters");
            }
        } else {
            if (fixedCardCharacterIds.size() != fixedCards.size()) {
                throw std::runtime_error("Fixed cards have duplicate characters");
            }
        }
    }

    auto honorBonus = deckCalculator.getHonorBonusPower();

    std::vector<RecommendDeck> ans{};
    std::vector<CardDetail> cardDetails{};
    std::vector<CardDetail> preCardDetails{};
    auto sf = [&scoreFunc, &musicMeta](const DeckDetail& deckDetail) { return scoreFunc(musicMeta, deckDetail); };

    RecommendCalcInfo calcInfo{};
    calcInfo.start_ts = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    calcInfo.timeout = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(config.timeout_ms)).count();

    // 指定活动加成组卡
    if (config.target == RecommendTarget::Bonus) {
        if (eventConfig.eventType == 0) 
            throw std::runtime_error("Bonus target requires event");
        if (config.algorithm != RecommendAlgorithm::DFS) 
            throw std::runtime_error("Bonus target only supports DFS algorithm");

        // WL和普通活动采用不同代码
        if (eventConfig.eventType != world_bloom_type_enum) {
            findTargetBonusCardsDFS(
                liveType, config, cards, sf, calcInfo,
                config.limit, config.member, eventConfig.eventType, eventConfig.eventId
            );
        }
        else {
            findWorldBloomTargetBonusCardsDFS(
                liveType, config, cards, sf, calcInfo,
                config.limit, config.member, eventConfig.eventType, eventConfig.eventId
            );
        }

        while (calcInfo.deckQueue.size()) {
            ans.emplace_back(calcInfo.deckQueue.top());
            calcInfo.deckQueue.pop();
        }
        // 按照活动加成从小到大排序，同加成按分数从小到大排序
        std::sort(ans.begin(), ans.end(), [](const RecommendDeck& a, const RecommendDeck& b) {
            return std::tuple(-a.eventBonus.value_or(0), a.targetValue) > std::tuple(-b.eventBonus.value_or(0), b.targetValue);
        });
        return ans;
    }

    // 最优化组卡
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
                break;
        }
        preCardDetails = cardDetails;
        auto cards0 = cardDetails;

        // 卡牌大致按强度排序，保证dfs先遍历强度高的卡组
        if (config.target == RecommendTarget::Skill) {
            std::sort(cards0.begin(), cards0.end(), [](const CardDetail& a, const CardDetail& b) { 
                return std::make_tuple(a.skill.max, a.skill.min, a.cardId) > std::make_tuple(b.skill.max, b.skill.min, b.cardId);
            });
        } else {
            std::sort(cards0.begin(), cards0.end(), [](const CardDetail& a, const CardDetail& b) { 
                return std::make_tuple(a.power.max, a.power.min, a.cardId) > std::make_tuple(b.power.max, b.power.min, b.cardId);
            });
        }

        if (config.algorithm == RecommendAlgorithm::SA) {
            // 使用模拟退火
            long long seed = config.saSeed;
            if (seed == -1) 
                seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    
            auto rng = Rng(seed);
            for (int i = 0; i < config.saRunCount && !calcInfo.isTimeout(); i++) {
                findBestCardsSA(
                    liveType, config, rng, cards0, cards, sf,
                    calcInfo,
                    config.limit, liveType == challenge_live_type_enum, config.member, honorBonus,
                    eventConfig.eventType, eventConfig.eventId, fixedCards
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
                liveType, config, rng, cards0, cards, sf,
                calcInfo,
                config.limit, liveType == challenge_live_type_enum, config.member, honorBonus,
                eventConfig.eventType, eventConfig.eventId, fixedCards
            );
        }
        else if (config.algorithm == RecommendAlgorithm::DFS) {
            // 使用DFS
            calcInfo.deckCards.clear();
            calcInfo.deckCharacters.clear();

            // 插入固定卡牌
            for (const auto& card : fixedCards) {
                calcInfo.deckCards.push_back(&card);
                calcInfo.deckCharacters.insert(card.characterId);
            }

            findBestCardsDFS(
                liveType, config, cards0, cards, sf,
                calcInfo,
                config.limit, liveType == challenge_live_type_enum, config.member, honorBonus, 
                eventConfig.eventType, eventConfig.eventId, fixedCards
            );
        }
        else {
            throw std::runtime_error("Unknown algorithm: " + std::to_string(int(config.algorithm)));
        }
        
        ans.clear();
        auto q = calcInfo.deckQueue;
        while (q.size()) {
            ans.emplace_back(q.top());
            q.pop();
        }
        std::reverse(ans.begin(), ans.end());
        if (int(ans.size()) >= config.limit || calcInfo.isTimeout()) 
            break;
    }

    // 获取live分数
    for (auto& deck : ans) 
        deck.liveScore = liveCalculator.getLiveScoreByDeck(
            deck, musicMeta, liveType,
            config.multiTeammateScoreUp, config.multiTeammatePower
        );
    return ans;
}
