#include "deck-recommend/base-deck-recommend.h"


static int getCharaBonusKey(int chara, int bonus) {
    return bonus * 100 + chara;
}
static int getBonus(int key) {
    return key / 100;
}
static int getChara(int key) {
    return key % 100;
}
static std::pair<int, int> getBonusChara(int key) {
    return {getBonus(key), getChara(key)};
}


// 分层过滤加成
using BonusFilter = std::function<bool(int key)>;
static const std::vector<BonusFilter> bonusFilters = {
    // 各个组合各自组卡
    [](int key) {
        int chara = getChara(key);
        return (chara - 1) / 4 == 0 || chara > 20;
    },
    [](int key) {
        int chara = getChara(key);
        return (chara - 1) / 4 == 1 || chara > 20;
    },
    [](int key) {
        int chara = getChara(key);
        return (chara - 1) / 4 == 2 || chara > 20;
    },
    [](int key) {
        int chara = getChara(key);
        return (chara - 1) / 4 == 3 || chara > 20;
    },
    [](int key) {
        int chara = getChara(key);
        return (chara - 1) / 4 == 4 || chara > 20;
    },
    // 最后一级：全部
    [](int key) { 
        return true; 
    },
};
static std::map<int, bool> applyFilter(const BonusFilter& filter, std::map<int, bool>& hasBonusCharaCards) {
    std::map<int, bool> ret{};
    for (const auto& [key, hasCard] : hasBonusCharaCards) 
        if (filter(key)) 
            ret[key] = hasCard;
    return ret;
}


bool dfsBonus(
    const DeckRecommendConfig &config, 
    RecommendCalcInfo &dfsInfo, 
    std::set<int> &targets,
    int currentBonus,
    std::vector<int>& current,
    std::map<int, std::vector<std::vector<int>>>& result,
    std::map<int, bool>& hasBonusCharaCards,
    std::set<int>& charaVis
)
{
    if ((int)current.size() == config.member) {
        if (targets.count(currentBonus)) {
            result[currentBonus].push_back(current);
            if (result[currentBonus].size() == config.limit) 
                targets.erase(currentBonus); 
        }
        return targets.size() > 0;
    }

    // 超过时间，退出
    if (dfsInfo.isTimeout()) 
        return false;

    // 加成超过目标，剪枝
    if (currentBonus > *targets.rbegin())
        return true;

    // 获取遍历起点，从上一个key的下一个开始遍历，保证key是递增的
    auto start_it = hasBonusCharaCards.begin();
    if (!current.empty()) {
        start_it = hasBonusCharaCards.lower_bound(current.back());
        ++start_it; 
    }

    // 获取剩下的卡中能取的member-current.size()个最低和最高加成，用于剪枝
    int lowestBonus = 0, highestBonus = 0;
    auto it = start_it;
    for (int rest = config.member - (int)current.size(); rest > 0 && it != hasBonusCharaCards.end(); ++it) {
        auto [bonus, chara] = getBonusChara(it->first);
        if (charaVis.find(chara) != charaVis.end()) continue; // 跳过重复角色
        if (!it->second) continue; // 跳过没有卡牌
        lowestBonus += bonus, --rest;
    }
    it = hasBonusCharaCards.end(), --it;
    for (int rest = config.member - (int)current.size(); rest > 0; --it) {
        auto [bonus, chara] = getBonusChara(it->first);
        if (charaVis.find(chara) != charaVis.end()) continue; // 跳过重复角色
        if (!it->second) continue; // 跳过没有卡牌
        highestBonus += bonus, --rest;
        if (it == start_it) break;  // 需要包含start_it（还没取）
    }
    if(currentBonus + lowestBonus > *targets.rbegin() || currentBonus + highestBonus < *targets.begin()) 
        return true;

    // 搜索剩下卡牌
    for (auto it = start_it; it != hasBonusCharaCards.end(); ++it) {
        auto [bonus, chara] = getBonusChara(it->first);
        if (charaVis.find(chara) != charaVis.end()) continue;   // 跳过重复角色
        if (!it->second) continue; // 跳过没有卡牌

        it->second = false;
        charaVis.insert(chara);
        current.push_back(it->first);

        bool cont = dfsBonus(config, dfsInfo, targets, 
            currentBonus + bonus, current, result, hasBonusCharaCards, charaVis);
        if (!cont) return false; 

        current.pop_back();
        charaVis.erase(chara);
        it->second = true; 
    }   
    return true;
}


void BaseDeckRecommend::findTargetBonusCardsDFS(
    int liveType, 
    const DeckRecommendConfig &config, 
    const std::vector<CardDetail> &cardDetails, 
    const std::function<Score(const DeckDetail &)> &scoreFunc, 
    RecommendCalcInfo &dfsInfo, 
    int limit, 
    int member, 
    std::optional<int> eventType, 
    std::optional<int> eventId
)
{
    std::map<int, std::vector<SupportDeckCard>> emptySupportCards{};

    std::vector<int> bonusList = config.bonusList;
    for (auto& x : bonusList) x *= 2;
    std::sort(bonusList.begin(), bonusList.end());
    if (bonusList.empty()) 
        throw std::runtime_error("Bonus list is empty");

    if (eventType.value_or(0) == Enums::EventType::world_bloom) {
        // 该函数只用于非WL活动
        throw std::runtime_error("this func is not used for world bloom event");
    }

    // 按照加成*2和角色类型归类
    std::map<int, std::vector<const CardDetail *>> bonusCharaCards;
    std::map<int, bool> hasBonusCharaCards;
    for (const auto &card : cardDetails) {
        if (card.maxEventBonus.has_value() && card.maxEventBonus.value() > 0) {
            if (std::abs(std::round(card.maxEventBonus.value() * 2) - card.maxEventBonus.value() * 2) > 1e-6)
                continue;
            int bonus = std::round(card.maxEventBonus.value() * 2);
            int chara = card.characterId;
            int key = getCharaBonusKey(chara, bonus);
            bonusCharaCards[key].push_back(&card);
            hasBonusCharaCards[key] = true;
        }
    }
    for(auto& [key, cards] : bonusCharaCards) {
        std::sort(cards.begin(), cards.end(), [](const CardDetail *a, const CardDetail *b) {
            return std::tuple(a->power.max, a->cardId) < std::tuple(b->power.max, b->cardId);
        });
    }

    // 剩余的组卡目标
    std::set<int> targets(bonusList.begin(), bonusList.end());

    // 按照不同层级过滤进行分层搜索
    for(auto& filter : bonusFilters) {
        auto filteredHasBonusCharaCards = applyFilter(filter, hasBonusCharaCards);

        std::vector<int> current;
        std::map<int, std::vector<std::vector<int>>> result; 
        std::set<int> charaVis; 
        dfsBonus(config, dfsInfo, targets, 0, current, result, filteredHasBonusCharaCards, charaVis);

        // 取卡
        for (auto& [bonus, bonusResult] : result) {
            for (auto &resultKeys : bonusResult) {
                std::vector<const CardDetail *> deckCards{};
                for (auto key : resultKeys) 
                    deckCards.push_back(bonusCharaCards[key].front()); 
                // 计算卡组详情
                auto deckRes = getBestPermutation(
                    deckCalculator, deckCards, emptySupportCards, scoreFunc,
                    0, eventType, eventId, liveType, config
                ).bestDeck.value();
                // 需要验证加成正确
                if(std::abs(deckRes.eventBonus.value_or(0) * 2 - bonus) < 1e-6)
                    dfsInfo.update(deckRes, 1e9);
                else
                    std::cerr << "Warning: Event bonus mismatch, expected " 
                            << bonus / 2.0 << ", got " 
                            << deckRes.eventBonus.value_or(0) << std::endl;
            }
        }

        if (targets.empty()) {
            // 如果已经找到所有目标，退出
            break;
        }
    }
}
