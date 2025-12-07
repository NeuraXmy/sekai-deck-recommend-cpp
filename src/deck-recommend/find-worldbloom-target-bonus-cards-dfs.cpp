#include "deck-recommend/base-deck-recommend.h"


static int getCharaAttrBonusKey(int chara, int attr, int bonus) {
    return bonus * 1000 + chara * 10 + attr;
}
static int getBonus(int key) {
    return key / 1000;
}
static int getChara(int key) {
    return (key / 10) % 100;
}
static int getAttr(int key) {
    return key % 10;
}
static std::tuple<int, int, int> getCharaAttrBonus(int key) {
    return { getChara(key), getAttr(key), getBonus(key) };
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


bool dfsWorldBloomBonus(
    const DeckRecommendConfig &config, 
    RecommendCalcInfo &dfsInfo, 
    std::set<int> &targets,
    int currentBonus,
    std::vector<int>& current,
    std::map<int, std::vector<std::vector<int>>>& result,
    std::map<int, bool>& hasBonusCharaCards,
    std::set<int>& charaVis,
    std::array<int, 10>& attrVis,
    std::map<int, int>& diffAttrBonus,
    const int maxAttrBonus
)
{
    int diffAttrCount = 0;
    for (int i = 0; i < 10; ++i) 
        diffAttrCount += bool(attrVis[i]);
    int currentDiffAttrBonus = diffAttrBonus[diffAttrCount];

    if ((int)current.size() == config.member) {
        int realCurrentBonus = currentBonus + currentDiffAttrBonus;
        if (targets.count(realCurrentBonus)) {
            result[realCurrentBonus].push_back(current);
            if (result[realCurrentBonus].size() == config.limit) 
                targets.erase(realCurrentBonus); 
        }
        return targets.size() > 0;
    }

    // 超过时间，退出
    if (dfsInfo.isTimeout()) 
        return false;

    // 加成超过目标，剪枝
    if (currentBonus + currentDiffAttrBonus > *targets.rbegin())
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
        auto [chara, attr, bonus] = getCharaAttrBonus(it->first);
        if (charaVis.find(chara) != charaVis.end()) continue; // 跳过重复角色
        if (!it->second) continue; // 跳过没有卡牌
        lowestBonus += bonus, --rest;
    }
    it = hasBonusCharaCards.end(), --it;
    for (int rest = config.member - (int)current.size(); rest > 0; --it) {
        auto [chara, attr, bonus] = getCharaAttrBonus(it->first);
        if (charaVis.find(chara) != charaVis.end()) continue; // 跳过重复角色
        if (!it->second) continue; // 跳过没有卡牌
        highestBonus += bonus, --rest;
        if (it == start_it) break;  // 需要包含start_it（还没取）
    }
    // 最低加成假设为当前异色数（因为加入新卡异色数只会变多），最高加成假设为全异色
    if(currentBonus + currentDiffAttrBonus + lowestBonus  > *targets.rbegin() 
    || currentBonus + maxAttrBonus         + highestBonus < *targets.begin()) 
        return true;

    // 搜索剩下卡牌
    for (auto it = start_it; it != hasBonusCharaCards.end(); ++it) {
        auto [chara, attr, bonus] = getCharaAttrBonus(it->first);
        if (charaVis.find(chara) != charaVis.end()) continue;   // 跳过重复角色
        if (!it->second) continue; // 跳过没有卡牌

        it->second = false;
        attrVis[attr]++;
        charaVis.insert(chara);
        current.push_back(it->first);

        bool cont = dfsWorldBloomBonus(
            config, dfsInfo, targets, 
            currentBonus + bonus, current, result, hasBonusCharaCards, charaVis,
            attrVis, diffAttrBonus, maxAttrBonus
        );
        if (!cont) return false; 

        current.pop_back();
        charaVis.erase(chara);
        attrVis[attr]--;
        it->second = true; 
    }   
    return true;
}


void BaseDeckRecommend::findWorldBloomTargetBonusCardsDFS(
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
    if (eventId.value_or(0) == finalChapterEventId)
        throw std::invalid_argument("final chapter event is not supported for bonus target");

    std::map<int, std::vector<SupportDeckCard>> emptySupportCards{};

    std::vector<int> bonusList = config.bonusList;
    for (auto& x : bonusList) x *= 2;
    std::sort(bonusList.begin(), bonusList.end());
    if (bonusList.empty()) 
        throw std::runtime_error("Bonus list is empty");

    if (eventType.value_or(0) != Enums::EventType::world_bloom) {
        // 该函数只用于WL活动
        throw std::runtime_error("this func is only used for world bloom event");
    }

    // 按照加成*2和角色类型和卡牌颜色归类
    std::map<int, std::vector<const CardDetail *>> bonusCharaCards;
    std::map<int, bool> hasBonusCharaCards;
    for (const auto &card : cardDetails) {
        if (card.maxEventBonus.has_value() && card.maxEventBonus.value() > 0) {
            if (std::abs(std::round(card.maxEventBonus.value() * 2) - card.maxEventBonus.value() * 2) > 1e-6)
                continue;
            int bonus = std::round(card.maxEventBonus.value() * 2);
            int chara = card.characterId;
            int attr = card.attr;
            int key = getCharaAttrBonusKey(chara, attr, bonus);
            bonusCharaCards[key].push_back(&card);
            hasBonusCharaCards[key] = true;
        }
    }
    for(auto& [key, cards] : bonusCharaCards) {
        std::sort(cards.begin(), cards.end(), [](const CardDetail *a, const CardDetail *b) {
            return std::tuple(a->power.max, a->cardId) < std::tuple(b->power.max, b->cardId);
        });
    }

    // wl异色加成
    std::map<int, int> diffAttrBonus = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}};
    int maxAttrBonus = 0;
    auto& worldBloomDifferentAttributeBonuses = this->dataProvider.masterData->worldBloomDifferentAttributeBonuses;
    for (const auto &bonus : worldBloomDifferentAttributeBonuses) {
        diffAttrBonus[bonus.attributeCount] = std::round(bonus.bonusRate * 2);
        maxAttrBonus = std::max(maxAttrBonus, diffAttrBonus[bonus.attributeCount]);
    }

    // 剩余的组卡目标
    std::set<int> targets(bonusList.begin(), bonusList.end());

    // 按照不同层级过滤进行分层搜索
    for(auto& filter : bonusFilters) {
        auto filteredHasBonusCharaCards = applyFilter(filter, hasBonusCharaCards);

        std::vector<int> current;
        std::map<int, std::vector<std::vector<int>>> result; 
        std::set<int> charaVis; 
        std::array<int, 10> attrVis = {}; 
        dfsWorldBloomBonus(
            config, dfsInfo, targets, 
            0, current, result, filteredHasBonusCharaCards, charaVis, 
            attrVis, diffAttrBonus, maxAttrBonus
        );

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
                    std::cerr << "Warning: World Bloom bonus mismatch, expected " 
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
