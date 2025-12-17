#include "deck-recommend/base-deck-recommend.h"
#include <any>
#include <algorithm>


struct Individual {
    int cardNum = 0;
    std::array<const CardDetail*, 5> deck{};
    uint64_t deckHash;
    double fitness;

    bool operator<(const Individual& other) const {
        return fitness < other.fitness;
    }
    bool operator>(const Individual& other) const {
        return fitness > other.fitness;
    }

    uint64_t calcDeckHash() {
        auto a = deck;
        std::sort(a.begin() + 1, a.begin() + cardNum, [](const CardDetail* a, const CardDetail* b) {
            return a->cardId < b->cardId;
        });
        constexpr uint64_t base = 10007;
        uint64_t hash = 0;
        for (int i = 0; i < cardNum; ++i) 
            hash = hash * base + a[i]->cardId;
        return hash;
    }

    void addCard(const CardDetail* card) {
        deck[cardNum++] = card;
    }

    std::string toString() const {
        std::string s = "Individual(";
        for (int i = 0; i < cardNum; ++i) {
            s += std::to_string(deck[i]->cardId);
            if (i != cardNum - 1) 
                s += ",";
        }
        s += ")";
        return s;
    }
};


// 计算随机选择权重（综合力/技能加成越大越容易被选中）
std::vector<double> calcRandomSelectWeights(
    const std::vector<CardDetail>& cards, 
    RecommendTarget target,
    const std::vector<CardDetail>& excluded
) {
    std::vector<double> weights{};
    for (const auto& card : cards) {
        bool skip = false;
        for (const auto& ex : excluded) {
            if (card.cardId == ex.cardId) {
                skip = true;
                break;
            }
        }
        if (skip) {
            weights.push_back(0.0);
            continue;
        }

        if (target == RecommendTarget::Skill) {
            // 以技能加成的平方为权重以扩大差距
            weights.push_back((double)card.skill.max * card.skill.max);
        } else {
            // 以综合力的平方为权重以扩大差距
            weights.push_back((double)card.power.max * card.power.max);
        }
    }
    // 归一化 & 计算前缀和
    double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
    if (sum > 0) 
        for (auto& weight : weights) 
            weight /= sum;
    for (int i = 1; i < (int)weights.size(); ++i)
        weights[i] += weights[i - 1];
    return weights;
}


// 根据权重随机选择一个index
int randomSelectIndexByWeight(Rng& rng, const std::vector<double>& weights) {
    if (weights.empty()) 
        throw std::invalid_argument("no cards to select");
    double rand = std::uniform_real_distribution<double>(0.0, 1.0)(rng);
    auto it = std::lower_bound(weights.begin(), weights.end(), rand);
    int index = std::distance(weights.begin(), it);
    return index;
}

// 根据权重随机选择n个不重复的index
std::vector<int> randomSelectIndexByWeight(Rng& rng, const std::vector<double>& weights, int n) {
    if (n > (int)weights.size()) 
        throw std::invalid_argument("no enough cards to select");
    std::vector<int> indices{};
    indices.reserve(n);
    while ((int)indices.size() < n) {
        int idx = randomSelectIndexByWeight(rng, weights);
        if (std::find(indices.begin(), indices.end(), idx) == indices.end()) {
            indices.push_back(idx);
        }
    }
    return indices;
}


void BaseDeckRecommend::findBestCardsGA(
    int liveType,
    const DeckRecommendConfig& cfg,
    Rng& rng,
    const std::vector<CardDetail> &cardDetails,     // 所有参与组队的卡牌
    std::map<int, std::vector<SupportDeckCard>>& supportCards,        // 全部卡牌（用于计算支援卡组加成）
    const std::function<Score(const DeckDetail &)> &scoreFunc,    
    RecommendCalcInfo& gaInfo,
    int limit, 
    bool isChallengeLive, 
    int member, 
    int honorBonus, 
    std::optional<int> eventType, 
    std::optional<int> eventId,
    const std::vector<CardDetail>& fixedCards
) {
    int fixedSize = fixedCards.size();

    // 参数检查
    if (cfg.gaParentSize < 0 || cfg.gaParentSize > cfg.gaPopSize) 
        throw std::invalid_argument("gaParentSize must be between 0 and gaPopSize");
    if (cfg.gaEliteSize < 0 || cfg.gaEliteSize > cfg.gaPopSize) 
        throw std::invalid_argument("gaEliteSize must be between 0 and gaPopSize");

    // 防止挑战Live卡的数量小于允许上场的数量导致无法组队
    if (isChallengeLive) {
        member = std::min(member, int(cardDetails.size()));
    }

    // 计算个体的分数并更新答案
    auto updateIndividualScore = [&](Individual& individual) {
        // 检查是否已经计算过这个组合
        auto deckHash = individual.calcDeckHash();
        double targetValue = 0.0;
        if (gaInfo.deckTargetValueMap.count(deckHash)) {
            targetValue = gaInfo.deckTargetValueMap[deckHash];
        } else {
            // 计算当前综合力
            std::vector<const CardDetail*> deck{};
            deck.reserve(individual.cardNum);
            for (const auto& cardPtr : individual.deck) 
                deck.push_back(cardPtr);
            auto ret = getBestPermutation(
                this->deckCalculator, deck, supportCards, scoreFunc, 
                honorBonus, eventType, eventId, liveType, cfg
            );
            if (ret.bestDeck.has_value()) {
                targetValue = ret.bestDeck.value().targetValue;
                gaInfo.update(ret.bestDeck.value(), limit);
                gaInfo.deckTargetValueMap[deckHash] = targetValue;
            } else {
                // 目前只会由于最低实效限制导致无法组出卡组，这种情况适应度主要考虑实效
                targetValue = -1e9 + ret.maxMultiLiveScoreUp;
            }
        }
        individual.fitness = targetValue; // 分数直接作为适应度
        individual.deckHash = deckHash;
    };

    // 计算用于随机选择的卡牌权重，fixedCards不参与选择
    constexpr int MAX_CID = 27;
    auto allCardWeights = calcRandomSelectWeights(cardDetails, cfg.target, fixedCards);

    // 根据卡的角色map参与组队的卡牌
    std::vector<CardDetail> charaCardDetails[MAX_CID] = {};
    std::vector<double> charaCardWeights[MAX_CID] = {};
    for (const auto& card : cardDetails) 
        charaCardDetails[card.characterId].push_back(card);
    for (int i = 0; i < MAX_CID; ++i) 
        charaCardWeights[i] = calcRandomSelectWeights(charaCardDetails[i], cfg.target, fixedCards);

    // 生成初始种群
    std::vector<Individual> population;
    while((int)population.size() < cfg.gaPopSize) {
        Individual individual{};
        // 随机生成卡组
        if (!isChallengeLive) {
            // 活动live先随机选择member-fixed个不同角色
            std::vector<int> valid_charas{};
            for (int j = 0; j < MAX_CID; ++j)  {
                // 跳过没有卡的角色
                if (charaCardDetails[j].empty()) continue;
                // 不能是和fixedCards相同的角色
                if (std::find_if(fixedCards.begin(), fixedCards.end(), [&](const CardDetail& card) {
                    return card.characterId == j;
                }) != fixedCards.end()) continue;
                // 不能是固定角色
                if (std::find(cfg.fixedCharacters.begin(), cfg.fixedCharacters.end(), j) != cfg.fixedCharacters.end()) continue;
                valid_charas.push_back(j);
            }
            // 不足member个角色直接不能组
            if ((int)valid_charas.size() < member - fixedSize - (int)cfg.fixedCharacters.size())
                return;
            std::shuffle(valid_charas.begin(), valid_charas.end(), rng);
            valid_charas.resize(member - fixedSize - cfg.fixedCharacters.size());
            // 在开头添加固定角色
            valid_charas.insert(valid_charas.begin(), cfg.fixedCharacters.begin(), cfg.fixedCharacters.end());
            // 每个角色随机1张
            for (const auto& chara : valid_charas) {
                auto idx = randomSelectIndexByWeight(rng, charaCardWeights[chara]);
                individual.addCard(&charaCardDetails[chara][idx]);
            }
        } 
        else {
            // 挑战live随机member-fixed张不重复的
            auto indices = randomSelectIndexByWeight(rng, allCardWeights, member - fixedSize);
            for (const auto& idx : indices) 
                individual.addCard(&cardDetails[idx]);
        }
        // 添加固定卡牌（整个流程固定在最后）
        for (const auto& card : fixedCards) 
            individual.addCard(&card);
        updateIndividualScore(individual);
        population.push_back(individual);
    }   

    // 如果全部固定，不需要进化
    if(member == fixedSize) 
        return;

    int iter_num = 0;
    double cur_max_fitness = 0;
    double last_max_fitness = 0;
    int no_improve_iter_num = 0;
    double cur_mutation_rate = 0.0;

    // 交叉操作
    auto crossover = [&](const Individual& a, const Individual& b) {
        if (std::uniform_real_distribution<double>(0.0, 1.0)(rng) > cfg.gaCrossoverRate)
            return std::max(a, b);
        // 随机选择要保留的a位置（不包括固定卡牌）
        std::vector<int> pos{};
        pos.reserve(a.cardNum - fixedSize);
        for (int i = 0; i < a.cardNum - fixedSize; ++i) {
            // 如果是固定角色则一定保留
            if (std::find(cfg.fixedCharacters.begin(), cfg.fixedCharacters.end(), a.deck[i]->characterId) != cfg.fixedCharacters.end()) {
                pos.push_back(i);
                continue;
            }
            if (std::uniform_real_distribution<double>(0.0, 1.0)(rng) > 0.5) 
                pos.push_back(i);
        }
        // 从b中获取可以选择的所有位置（不包括固定）
        std::vector<int> b_pos{};
        b_pos.reserve(b.cardNum - fixedSize);
        for (int i = 0; i < b.cardNum - fixedSize; ++i) {
            auto c1 = b.deck[i];
            bool ok = true;
            for (auto p : pos) {
                auto c2 = a.deck[p];
                // 检查id是否重复
                if (c1->cardId == c2->cardId) {
                    ok = false;
                    break;
                }
                // 活动live还要检查是否有重复角色
                if (!isChallengeLive && c1->characterId == c2->characterId) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                b_pos.push_back(i);
            }
        }
        // 不应该出现的情况: b中可以选择的位置少于要在a中替换的位置
        if ((int)b_pos.size() < a.cardNum - fixedSize - (int)pos.size()) 
            throw std::runtime_error("crossover error: not enough position in B to select with "
            + a.toString() + " and " + b.toString());
        std::shuffle(b_pos.begin(), b_pos.end(), rng);
        b_pos.resize(a.cardNum - fixedSize - (int)pos.size());
        // 生成新个体
        Individual child{};
        for (const auto& p : pos) 
            child.addCard(a.deck[p]);
        for (const auto& p : b_pos)
            child.addCard(b.deck[p]);
        // 添加固定卡牌
        for (const auto& card : fixedCards) 
            child.addCard(&card);
        if (child.cardNum != member) 
            throw std::runtime_error("crossover error: deck size not equal to member with "
            + a.toString() + " and " + b.toString() + ", child: " + child.toString());

        // std::cerr << "crossover: ";
        // for (auto card : a.deck) std::cerr << card->cardId << " "; std::cerr << std::endl;
        // for (auto card : b.deck) std::cerr << card->cardId << " "; std::cerr << std::endl;
        // for (auto card : child.deck) std::cerr << card->cardId << " "; std::cerr << std::endl;
        return child;
    };

    // 变异操作
    auto mutate = [&](Individual& a) {
        // 遍历非固定的每个位置
        for (int pos = 0; pos < a.cardNum - fixedSize; ++pos) {
            if (std::uniform_real_distribution<double>(0.0, 1.0)(rng) > cur_mutation_rate) 
                continue;
            // 随机选择一张卡进行替换，需要检查新卡是否重复，最多10次避免死循环
            for (int _ = 0; _ < 10; ++_) {
                bool isFixedChara = std::find(cfg.fixedCharacters.begin(), cfg.fixedCharacters.end(), a.deck[pos]->characterId) != cfg.fixedCharacters.end();
                int index = 0;
                const CardDetail* newCard = nullptr;
                if (isFixedChara) {
                    // 如果是固定角色，则只能从该角色的卡随机
                    index = randomSelectIndexByWeight(rng, charaCardWeights[a.deck[pos]->characterId]);
                    newCard = &charaCardDetails[a.deck[pos]->characterId][index];
                }
                else {
                    // 否则从所有卡随机
                    index = randomSelectIndexByWeight(rng, allCardWeights);
                    newCard = &cardDetails[index];
                }
                // 检查与队里其他卡的冲突
                bool ok = true;
                for (int i = 0; i < a.cardNum; ++i) {
                    if (i == pos) continue;
                    auto card = a.deck[i];
                    // 检查id是否重复
                    if (card->cardId == newCard->cardId) {
                        ok = false;
                        break;
                    }
                    // 活动live还要检查是否有重复角色
                    if (!isChallengeLive && card->characterId == newCard->characterId) {
                        ok = false;
                        break;
                    }
                }
                if (ok) {
                    // std::cout << "mutate: " << a.deck[pos]->cardId << " -> " << newCard->cardId << std::endl;
                    a.deck[pos] = newCard;
                    break;
                }
            }
        }
    };

    // 迭代
    std::vector<Individual> newPopulation{};
    while (true) {
        std::sort(population.begin(), population.end(), std::greater<Individual>());
        last_max_fitness = cur_max_fitness;
        cur_mutation_rate = cfg.gaBaseMutationRate + cfg.gaNoImproveIterToMutationRate * (double)no_improve_iter_num;

        // 生成新种群
        newPopulation.clear();

        // 保留精英
        int eliteSize = std::min(cfg.gaEliteSize, (int)population.size());
        for (int i = 0; i < eliteSize; ++i)
            newPopulation.push_back(population[i]);

        // 繁殖
        int parentSize = std::min(cfg.gaParentSize, (int)population.size());
        while ((int)newPopulation.size() < cfg.gaPopSize) {
            // 随机选择两个父代
            int idx1 = std::uniform_int_distribution<int>(0, parentSize - 1)(rng);
            int idx2 = std::uniform_int_distribution<int>(0, parentSize - 1)(rng);
            Individual child = crossover(population[idx1], population[idx2]);
            mutate(child);
            updateIndividualScore(child);
            newPopulation.push_back(child);
            cur_max_fitness = std::max(cur_max_fitness, child.fitness);
        }

        // 去重
        population.clear();
        std::unordered_set<uint64_t> deckHashSet{};
        for (const auto& individual : newPopulation) {
            if (deckHashSet.count(individual.deckHash) == 0) {
                population.push_back(individual);
                deckHashSet.insert(individual.deckHash);
            }
        }

        if (cfg.gaDebug) {
            std::cout << "iter: " << iter_num << ", max fitness: " << cur_max_fitness 
            << ", mutation rate: " << cur_mutation_rate << ", population size: " << population.size() << '\n';
        }
        
        // 超出次数限制
        if (++iter_num > cfg.gaMaxIter) {
            break;
        }
        // 超出未改进次数限制
        if (cur_max_fitness <= last_max_fitness) {
            if (++no_improve_iter_num > cfg.gaMaxIterNoImprove) {
                break;
            }
        } else {
            no_improve_iter_num = 0;
        }
        // 超出总时间限制
        if (gaInfo.isTimeout()) {
            break;
        }
    }
}

