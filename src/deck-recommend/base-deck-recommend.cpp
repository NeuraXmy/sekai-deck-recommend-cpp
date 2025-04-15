#include "deck-recommend/base-deck-recommend.h"
#include "card-priority/card-priority-filter.h"
#include "common/timer.h"
#include "base-deck-recommend.h"
#include <chrono>
#include <random>

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


/*
获取当前卡组的最佳排列
*/
RecommendDeck getBestPermutation(
    DeckCalculator& deckCalculator,
    const std::vector<const CardDetail*> &deckCards,
    const std::vector<CardDetail> &allCards,
    const std::function<int(const DeckDetail &)> &scoreFunc,
    int honorBonus,
    std::optional<int> eventType,
    std::optional<int> eventId
) {
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
    int bestScoreUp = cards[0].skill.scoreUp;
    int bestScoreIndex = 0;
    for (size_t i = 0; i < cards.size(); ++i) {
        if (cards[i].skill.scoreUp > bestScoreUp) {
            bestScoreUp = cards[i].skill.scoreUp;
            bestScoreIndex = i;
        }
    }
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


void BaseDeckRecommend::findBestCards(
    const std::vector<CardDetail> &cardDetails, 
    const std::vector<CardDetail> &allCards, 
    const std::function<int(const DeckDetail &)> &scoreFunc, 
    RecommendCalcInfo& dfsInfo,
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
        dfsInfo.update(
            getBestPermutation(
                this->deckCalculator, deckCards, allCards, scoreFunc, 
                honorBonus, eventType, eventId
            ), limit
        );
        return;
    }

    // 超时
    if (dfsInfo.isTimeout()) 
        return;

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
        // C位一定是技能最好的卡牌，跳过技能比C位还好的
        if (deckCards.size() >= 1 && deckCards[0]->skill.isCertainlyLessThan(card.skill)) continue;
        // 为了优化性能，必须和C位同色或同组
        if (deckCards.size() >= 1 && card.attr != deckCards[0]->attr && !containsAny(deckCards[0]->units, card.units)) {
            continue;
        }
        // 要求生成的卡组后面4个位置按强弱排序、同强度按卡牌ID排序
        // 如果上一张卡肯定小，那就不符合顺序；在旗鼓相当的前提下（因为两两组合有四种情况，再排除掉这张卡肯定小的情况，就是旗鼓相当），要ID大
        if (deckCards.size() >= 2 && this->cardCalculator.isCertainlyLessThan(*deckCards[deckCards.size() - 1], card)) continue;
        if (deckCards.size() >= 2 && !this->cardCalculator.isCertainlyLessThan(card, *deckCards[deckCards.size() - 1]) &&
            card.cardId < deckCards[deckCards.size() - 1]->cardId) {
            continue;
        }
        // 如果肯定比上一次选定的卡牌要弱，那么舍去，让这张卡去后面再选
        if (preCard.has_value() && this->cardCalculator.isCertainlyLessThan(card, preCard.value())) continue;
        preCard = card;

        // 递归，寻找所有情况
        deckCards.push_back(&card);
        deckCharacters.insert(card.characterId);
        findBestCards(
            cardDetails, allCards, scoreFunc, dfsInfo,
            limit, isChallengeLive, member, honorBonus, eventType, eventId
        );
        deckCards.pop_back();
        deckCharacters.erase(card.characterId);
    }
}


void BaseDeckRecommend::findBestCardsSA(
    const DeckRecommendConfig& cfg,
    Rng& rng,
    const std::vector<CardDetail> &cardDetails,     // 所有参与组队的卡牌
    const std::vector<CardDetail> &allCards,        // 全部卡牌（用于计算支援卡组加成）
    const std::function<int(const DeckDetail &)> &scoreFunc,    
    RecommendCalcInfo& saInfo,
    int limit, 
    bool isChallengeLive, 
    int member, 
    int honorBonus, 
    std::optional<int> eventType, 
    std::optional<int> eventId
)
{
    // 计算第一位+后几位顺序无关的哈希值
    auto calcDeckHash = [](const std::vector<const CardDetail*>& deck) {
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

    // 根据卡的角色map参与组队的卡牌
    constexpr int MAX_CID = 27;
    std::vector<CardDetail> charaCardDetails[MAX_CID] = {};
    for (const auto& card : cardDetails) 
        charaCardDetails[card.characterId].push_back(card);
    
    double temperature = cfg.saStartTemperature;
    auto start_time = std::chrono::high_resolution_clock::now();
    int iter_num = 0;
    int no_improve_iter_num = 0;
    int current_score = 0;
    int last_score = 0;
    std::vector<int> replacableCardIndices{};
    std::set<int> deckCharacters{};
    std::set<int> deckCardIds{};
    
    // 根据综合力生成一个初始卡组
    std::vector<const CardDetail*> deck{};
    {
        if (!isChallengeLive) {
            // 遍历所有角色 找到每个角色综合力最高的一个卡牌
            for (int i = 0; i < MAX_CID; ++i) {
                auto& cards = charaCardDetails[i];
                if (cards.empty()) continue;
                auto& max_card = *std::max_element(cards.begin(), cards.end(), [](const CardDetail& a, const CardDetail& b) {
                    return a.power.min != b.power.min ? a.power.min < b.power.min : a.cardId > b.cardId;
                });
                deck.push_back(&max_card);
            }
        } else {
            for (auto& card : cardDetails) 
                deck.push_back(&card);
        }
        // 再排序后resize为member数量
        std::sort(deck.begin(), deck.end(), [](const CardDetail* a, const CardDetail* b) {
            return a->power.min != b->power.min ? a->power.min > b->power.min : a->cardId < b->cardId;
        });
        if (int(deck.size()) > member) 
            deck.resize(member);
        // 计算当前综合力
        auto deckDetail = deckCalculator.getDeckDetailByCards(deck, allCards, honorBonus, eventType, eventId);
        auto recDeck = getBestPermutation(
            this->deckCalculator, deck, allCards, scoreFunc, 
            honorBonus, eventType, eventId
        );
        // 记录当前卡组
        for (const auto& card : deck) {
            deckCharacters.insert(card->characterId);
            deckCardIds.insert(card->cardId);
        }
        saInfo.update(recDeck, limit);
        saInfo.deckScoreMap[calcDeckHash(deck)] = recDeck.score;
    }

    // 退火
    while (true) {
        // 随机选一个位置替换
        int pos = std::uniform_int_distribution<int>(0, int(deck.size()) - 1)(rng);

        // 收集该位置能够替换的卡的索引 chara_index * 10000 + card_index
        replacableCardIndices.clear();
        for (int i = 0; i < MAX_CID; ++i) {
            // 不是挑战live的情况，排除和其他几个卡角色相同的
            if (!isChallengeLive && i != deck[pos]->characterId && deckCharacters.count(i))
                continue;
            for(int j = 0; j < int(charaCardDetails[i].size()); j++) {
                // 如果是挑战live，需要排除和其他卡重复（不是挑战live的情况不用，因为其他卡会被角色相同判断排除）
                // 但是不排除需要替换的那张卡，避免出现没有能够替换的问题
                if (isChallengeLive && charaCardDetails[i][j].cardId != deck[pos]->cardId 
                    && deckCardIds.count(charaCardDetails[i][j].cardId))
                    continue;
                replacableCardIndices.push_back(i * 10000 + j);
            }
        }
        
        // 随机一个进行替换
        int index = std::uniform_int_distribution<int>(0, int(replacableCardIndices.size()) - 1)(rng);
        int chara_index = replacableCardIndices[index] / 10000;
        int card_index = replacableCardIndices[index] % 10000;
        auto old_card = deck[pos];
        auto new_card = &charaCardDetails[chara_index][card_index];
        
        // 替换，计算新的综合力，并计算接受概率
        deck[pos] = new_card;
        long long hash = calcDeckHash(deck);
        bool visited = saInfo.deckScoreMap.count(hash);
        int new_score = 0;
        RecommendDeck recDeck{};
        if (visited) {
            // 如果已经计算过这个组合，直接取值
            new_score = saInfo.deckScoreMap[hash];
        }
        else {
            recDeck = getBestPermutation(
                this->deckCalculator, deck, allCards, scoreFunc, 
                honorBonus, eventType, eventId
            );
            new_score = recDeck.score;
            saInfo.deckScoreMap[hash] = new_score;
        }

        double delta = new_score - current_score;
        double accept_prob = 0.0;
        if (delta > 0) {
            accept_prob = 1.0;
        } else {
            accept_prob = std::exp(delta / temperature);
        }

        // 以一定概率接受新的卡牌
        if (std::uniform_real_distribution<double>(0.0, 1.0)(rng) < accept_prob) {
            // 替换
            deckCharacters.erase(old_card->characterId);
            deckCardIds.erase(old_card->cardId);
            deckCharacters.insert(new_card->characterId);
            deckCardIds.insert(new_card->cardId);
            deck[pos] = new_card;
            last_score = current_score;
            current_score = new_score;
            // 记录当前卡组答案
            if (!visited)
                saInfo.update(RecommendDeck{recDeck, current_score}, limit);
        } else {
            // 恢复
            deck[pos] = old_card;
            last_score = current_score;
        }

        if (cfg.saDebug) {
            std::cerr << "sa iter: " << iter_num << ", score: " << new_score 
                    << ", last_score: " << last_score << ", temp: " << temperature 
                    << ", prob: " << accept_prob << " no_impro_iter: " << no_improve_iter_num 
                    << std::endl;
        }

        // 超出次数限制
        if (++iter_num >= cfg.saMaxIter) 
            break;
        // 超出未优化次数限制
        if (current_score <= last_score) {
            if(++no_improve_iter_num >= cfg.saMaxIterNoImprove) 
                break;
        } else {
            no_improve_iter_num = 0;
        }
        // 超出时间限制
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (elapsed_time > cfg.saMaxTimeMs) 
            break;  
        // 超出总时间限制
        if (saInfo.isTimeout()) 
            break;
        temperature *= cfg.saCoolingRate;
    }
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

    std::vector<RecommendDeck> ans{};
    std::vector<CardDetail> cardDetails{};
    std::vector<CardDetail> preCardDetails{};
    auto sf = [&scoreFunc, &musicMeta](const DeckDetail& deckDetail) { return scoreFunc(musicMeta, deckDetail); };

    RecommendCalcInfo calcInfo{};
    calcInfo.start_ts = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    calcInfo.timeout = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(config.timeout_ms)).count();

    while (true) {
        if (!config.useSa) {
            // 为了优化性能，会根据活动加成和卡牌稀有度优先级筛选卡牌
            cardDetails = filterCardPriority(liveType, eventConfig.eventType, cards, preCardDetails, config.member);
        } else {
            // 如果使用模拟退火不需要过滤
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

        if (config.useSa) {
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
        } else {
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
