#include "deck-recommend/base-deck-recommend.h"

int world_bloom_type_enum = mapEnum(EnumMap::eventType, "world_bloom");


int getBonusCharaKey(int chara, int bonus) {
    return bonus * 100 + chara;
}
int getBonus(int key) {
    return key / 100;
}
int getChara(int key) {
    return key % 100;
}
std::pair<int, int> getBonusChara(int key) {
    return {getBonus(key), getChara(key)};
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
    for (int rest = config.member - (int)current.size(); rest > 0 && it != start_it; --it) {
        auto [bonus, chara] = getBonusChara(it->first);
        if (charaVis.find(chara) != charaVis.end()) continue; // 跳过重复角色
        if (!it->second) continue; // 跳过没有卡牌
        highestBonus += bonus, --rest;
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
    const std::function<int(const DeckDetail &)> &scoreFunc, 
    RecommendCalcInfo &dfsInfo, 
    int limit, 
    int member, 
    std::optional<int> eventType, 
    std::optional<int> eventId
)
{
    std::vector<int> bonusList = config.bonusList;
    for (auto& x : bonusList) x *= 2;
    std::sort(bonusList.begin(), bonusList.end());
    if (bonusList.empty()) 
        throw std::runtime_error("Bonus list is empty");

    if (eventType.value_or(0) == world_bloom_type_enum) {
        // 暂时不支持wl
        throw std::runtime_error("bonus target for World Bloom is not supported currently");
    }

    // 按照加成*2和角色类型归类
    std::map<int, std::vector<const CardDetail *>> bonusCharaCards;
    std::map<int, bool> hasBonusCharaCards;
    for (const auto &card : cardDetails) {
        if (card.eventBonus.has_value() && card.eventBonus.value() > 0) {
            int bonus = std::round(card.eventBonus.value() * 2);
            int chara = card.characterId;
            int key = getBonusCharaKey(chara, bonus);
            bonusCharaCards[key].push_back(&card);
            hasBonusCharaCards[key] = true;
        }
    }
    for(auto& [key, cards] : bonusCharaCards) {
        std::sort(cards.begin(), cards.end(), [](const CardDetail *a, const CardDetail *b) {
            return std::tuple(a->power.max, a->cardId) < std::tuple(b->power.max, b->cardId);
        });
    }

    // 搜索
    std::vector<int> current;
    std::map<int, std::vector<std::vector<int>>> result; 
    std::set<int> charaVis; 
    std::set<int> targets(bonusList.begin(), bonusList.end());
    dfsBonus(config, dfsInfo, targets, 0, current, result, hasBonusCharaCards, charaVis);

    // 取卡
    for (auto& [bonus, bonusResult] : result) {
        for (auto &resultKeys : bonusResult) {
            std::vector<const CardDetail *> deckCards{};
            for (auto key : resultKeys) 
                deckCards.push_back(bonusCharaCards[key].front()); 
            // 计算卡组详情
            auto deckRes = getBestPermutation(
                deckCalculator, deckCards, {}, scoreFunc,
                0, eventType, eventId, config.target, liveType
            );
            // 需要验证加成正确
            if(std::floor(deckRes.eventBonus.value_or(0) * 2) == bonus) 
                dfsInfo.update(deckRes, 1e9);
        }
    }
}
