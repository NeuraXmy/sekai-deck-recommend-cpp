#include "deck-recommend/base-deck-recommend.h"
#include "card-priority/card-priority-filter.h"
#include "common/timer.h"
#include <chrono>
#include <random>


uint64_t BaseDeckRecommend::calcDeckHash(const std::vector<const CardDetail*>& deck) {
    int card_num = (int)deck.size();
    std::array<int, 5> v{};
    for (int i = 0; i < card_num; ++i)
        v[i] = deck[i]->cardId;
    std::sort(v.begin() + 1, v.begin() + card_num);
    constexpr uint64_t base = 10007;
    uint64_t hash = 0;
    for (int i = 0; i < card_num; ++i) 
        hash = hash * base + v[i];
    return hash;
};


/*
获取当前卡组的最佳排列
*/
BestPermutationResult BaseDeckRecommend::getBestPermutation(
    DeckCalculator& deckCalculator,
    const std::vector<const CardDetail*> &deckCards,
    std::map<int, std::vector<SupportDeckCard>>& supportCards,
    const std::function<Score(const DeckDetail &)> &scoreFunc,
    int honorBonus,
    std::optional<int> eventType,
    std::optional<int> eventId,
    int liveType,
    const DeckRecommendConfig& config
) const {
    bool bestSkillAsLeader = config.bestSkillAsLeader;
    // 存在固定队长角色则不允许把技能最强的换到队长
    if (config.fixedCharacters.size()) bestSkillAsLeader = false;
    // 终章活动不允许把技能最强的换到队长
    if (eventId.has_value() && eventId.value() == finalChapterEventId) bestSkillAsLeader = false;
    // 获取当前卡组的详情
    auto deckDetails = deckCalculator.getDeckDetailByCards(
        deckCards, supportCards, honorBonus, eventType, eventId, 
        config.skillReferenceChooseStrategy, config.keepAfterTrainingState, bestSkillAsLeader
    );
    // 获取最高分的卡组
    double maxValue{};
    BestPermutationResult ret{};
    for (auto& deckDetail : deckDetails) {
        auto score = scoreFunc(deckDetail);
        double value = score.score + score.liveScore * 1e-7;

        ret.maxTargetValue = std::max(ret.maxTargetValue, value);
        ret.maxMultiLiveScoreUp = std::max(ret.maxMultiLiveScoreUp, deckDetail.multiLiveScoreUp);
        
        // 最低实效限制
        if (deckDetail.multiLiveScoreUp < config.multiScoreUpLowerBound)
            continue;
        
        if (value > maxValue) {
            maxValue = value;
            ret.bestDeck = RecommendDeck(deckDetail, config.target, score);
        }
    }
    return ret;
}


std::vector<RecommendDeck> BaseDeckRecommend::recommendHighScoreDeck(
    const std::vector<UserCard> &userCards,
    ScoreFunction scoreFunc,
    const DeckRecommendConfig &config,
    int liveType,
    const EventConfig &eventConfig)
{
    this->dataProvider.init();

    // 暂不支持同时指定固定卡牌和固定角色
    if (config.fixedCards.size() && config.fixedCharacters.size())
        throw std::runtime_error("Cannot set both fixed cards and fixed characters");
    // 挑战live不允许指定固定角色
    if (Enums::LiveType::isChallenge(liveType) && config.fixedCharacters.size())
        throw std::runtime_error("Cannot set fixed characters in challenge live");

    auto musicMeta = this->liveCalculator.getMusicMeta(config.musicId, config.musicDiff);

    auto areaItemLevels = areaItemService.getAreaItemLevels();
    auto& cardEpisodes = this->dataProvider.masterData->cardEpisodes;

    std::optional<double> scoreUpLimit = std::nullopt;
    // 终章技能加分上限为140
    if (eventConfig.eventId == finalChapterEventId && !Enums::LiveType::isChallenge(liveType))
        scoreUpLimit = 140.0;

    auto cards = cardCalculator.batchGetCardDetail(
        userCards, config.cardConfig, config.singleCardConfig, 
        eventConfig, areaItemLevels, scoreUpLimit
    );

    // 归类支援卡组
    std::map<int, std::vector<SupportDeckCard>> supportCards{};
    if (eventConfig.eventId == finalChapterEventId) {
        // 终章对每个角色都算一个支援卡组排序
        for (int i = 1; i <= 26; i++) {
            std::vector<SupportDeckCard> sc{};
            for (const auto& card : userCards) 
                sc.push_back(this->cardCalculator.getSupportDeckCard(card, eventConfig.eventId, i));
            std::sort(sc.begin(), sc.end(), [](const SupportDeckCard& a, const SupportDeckCard& b) { return a.bonus > b.bonus; });
            supportCards[i] = sc;
        }
    } else if(eventConfig.eventType == Enums::EventType::world_bloom) {
        // 普通wl只算一个支援卡组排序
        std::vector<SupportDeckCard> sc{};
        for (const auto& card : userCards) 
            sc.push_back(this->cardCalculator.getSupportDeckCard(card, eventConfig.eventId, eventConfig.specialCharacterId));
        std::sort(sc.begin(), sc.end(), [](const SupportDeckCard& a, const SupportDeckCard& b) { return a.bonus > b.bonus; });
        supportCards[0] = sc;
    }

    // 过滤箱活的卡，不上其它组合的
    if (eventConfig.eventUnit && config.filterOtherUnit) {
        std::vector<CardDetail> newCards{};
        for (const auto& card : cards) {
            if ((card.units.size() == 1 && card.units[0] == Enums::Unit::piapro) || 
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
            uc.specialTrainingStatus = Enums::SpecialTrainingStatus::not_doing;
            
            auto& c = findOrThrow(this->dataProvider.masterData->cards, [&](const Card& c) {
                return c.id == card_id;
            }, [&]() { return "Card not found for fixed cardId=" + std::to_string(card_id); });
            bool hasSpecialTraining = c.cardRarityType == Enums::CardRarityType::rarity_3
                                    || c.cardRarityType == Enums::CardRarityType::rarity_4;
            uc.defaultImage = hasSpecialTraining ? Enums::DefaultImage::special_training : Enums::DefaultImage::original;

            for (auto& ep : cardEpisodes) 
                if (ep.cardId == card_id) {
                    UserCardEpisodes uce{};
                    uce.cardEpisodeId = ep.id;
                    uce.scenarioStatus = 0;
                    uc.episodes.push_back(uce);
                }
            auto card = cardCalculator.batchGetCardDetail(
                {uc}, config.cardConfig, config.singleCardConfig, 
                eventConfig, areaItemLevels, scoreUpLimit
            );
            if (card.size() > 0) {
                fixedCards.push_back(card[0]);
                cards.push_back(card[0]);
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
        if (Enums::LiveType::isChallenge(liveType)) {
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
        if (eventConfig.eventType != Enums::EventType::world_bloom) {
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
        auto cardsSortedByStrength = cardDetails;

        // 卡牌大致按强度排序，保证dfs先遍历强度高的卡组
        if (config.target == RecommendTarget::Skill) {
            std::sort(cardsSortedByStrength.begin(), cardsSortedByStrength.end(), [](const CardDetail& a, const CardDetail& b) { 
                return std::make_tuple(a.skill.max, a.skill.min, a.cardId) > std::make_tuple(b.skill.max, b.skill.min, b.cardId);
            });
        } else {
            std::sort(cardsSortedByStrength.begin(), cardsSortedByStrength.end(), [](const CardDetail& a, const CardDetail& b) { 
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
                    liveType, config, rng, cardsSortedByStrength, supportCards, sf,
                    calcInfo,
                    config.limit, Enums::LiveType::isChallenge(liveType), config.member, honorBonus,
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
                liveType, config, rng, cardsSortedByStrength, supportCards, sf,
                calcInfo,
                config.limit, Enums::LiveType::isChallenge(liveType), config.member, honorBonus,
                eventConfig.eventType, eventConfig.eventId, fixedCards
            );
        }
        else if (config.algorithm == RecommendAlgorithm::DFS) {
            // 使用DFS
            calcInfo.deckCards.clear();
            calcInfo.deckCharacters = 0;

            // 插入固定卡牌
            for (const auto& card : fixedCards) {
                calcInfo.deckCards.push_back(&card);
                calcInfo.deckCharacters.flip(card.characterId);
            }

            findBestCardsDFS(
                liveType, config, cardsSortedByStrength, supportCards, sf,
                calcInfo,
                config.limit, Enums::LiveType::isChallenge(liveType), config.member, honorBonus, 
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

    return ans;
}
