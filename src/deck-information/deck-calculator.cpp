#include "deck-information/deck-calculator.h"
#include "common/timer.h"
#include "deck-calculator.h"

static int world_bloom_enum = mapEnum(EnumMap::eventType, "world_bloom");
static int piapro_unit_enum = mapEnum(EnumMap::unit, "piapro");

static int other_member_score_up_reference_rate_enum    = mapEnum(EnumMap::skillEffectType, "other_member_score_up_reference_rate");
static int score_up_unit_count_enum                     = mapEnum(EnumMap::skillEffectType, "score_up_unit_count");

std::optional<double> DeckCalculator::getDeckBonus(const std::vector<const CardDetail *> &deckCards, std::optional<int> eventType) 
{
    // 如果没有预处理好活动加成，则返回空
    for (const auto &card : deckCards) 
        if (!card->eventBonus.has_value()) 
            return std::nullopt;
    double bonus = 0;
    for (const auto &card : deckCards) 
        bonus += card->eventBonus.value();
    if (eventType != world_bloom_enum) 
        return bonus;
    
    // 如果是世界开花活动，还需要计算卡组的异色加成
    auto& worldBloomDifferentAttributeBonuses = this->dataProvider.masterData->worldBloomDifferentAttributeBonuses;
    bool attr_vis[10] = {};
    for (const auto &card : deckCards) 
        attr_vis[card->attr] = true;
    int attr_count = 0;
    for (int i = 0; i < 10; ++i) 
        attr_count += attr_vis[i];
    auto it = findOrThrow(worldBloomDifferentAttributeBonuses, [&](const auto &it) { 
        return it.attributeCount == attr_count; 
    });
    return bonus + it.bonusRate;
}

SupportDeckBonus DeckCalculator::getSupportDeckBonus(const std::vector<const CardDetail*> &deckCards, const std::vector<CardDetail> &allCards, int supportDeckCount)
{
    double bonus = 0;
    int count = 0;
    std::vector<CardDetail> cards{};
    for (const auto &card : allCards) {
        // 如果没有预处理好支援卡组加成，则跳过
        if (!card.supportDeckBonus.has_value()) 
            continue;
        // 支援卡组的卡不能和主队伍重复，需要排除掉
        if (std::find_if(deckCards.begin(), deckCards.end(), [&](const auto &it) { 
            return it->cardId == card.cardId; 
        }) != deckCards.end()) 
            continue;
        bonus += card.supportDeckBonus.value();
        count++;
        // cards.push_back(card); for debug
        // 修改为能够自定义支援卡组的数量
        if (count >= supportDeckCount) return { bonus, cards };
    }
    // 就算组不出完整的支援卡组也得返回
    return { bonus, cards };
}

int DeckCalculator::getHonorBonusPower()
{
    auto& honors = this->dataProvider.masterData->honors;
    auto& userHonors = this->dataProvider.userData->userHonors;
    int bonus = 0;
    for (const auto &userHonor : userHonors) {
        auto it = findOrThrow(honors, [&](const auto &it) { 
            return it.id == userHonor.honorId; 
        });
        auto levelIt = findOrThrow(it.levels, [&](const auto &it) { 
            return it.level == userHonor.level; 
        });
        bonus += levelIt.bonus;
    }
    return bonus;
}

long long c[5] = {};

DeckDetail DeckCalculator::getDeckDetailByCards(
    const std::vector<const CardDetail*> &cardDetails, 
    const std::vector<CardDetail> &allCards, 
    int honorBonus, 
    std::optional<int> eventType,
    std::optional<int> eventId,
    SkillReferenceChooseStrategy skillReferenceChooseStrategy
)
{
    // 预处理队伍和属性，存储每个队伍或属性出现的次数
    int attr_map[10] = {};
    int unit_map[10] = {};
    for (auto p : cardDetails) {
        auto& cardDetail = *p;
        attr_map[cardDetail.attr]++;
        for (const auto &key : cardDetail.units) {
            unit_map[key]++;
        }
    }

    // 计算当前卡组的综合力，要加上称号的固定加成
    std::vector<DeckCardPowerDetail> cardPower{};
    for (auto p : cardDetails) {
        auto& cardDetail = *p;
        DeckCardPowerDetail powerDetail = {};
        for (const auto &unit : cardDetail.units) {
            auto current = cardDetail.power.get(unit, unit_map[unit], attr_map[cardDetail.attr]);
            // 有多个组合时，取最高加成组合
            powerDetail = current.total > powerDetail.total ? current : powerDetail;
        }
        cardPower.push_back(powerDetail);
    }

    int base = 0;
    for (const auto &p : cardPower) base += p.base;
    int areaItemBonus = 0;
    for (const auto &p : cardPower)  areaItemBonus += p.areaItemBonus;
    int characterBonus = 0;
    for (const auto &p : cardPower) characterBonus += p.characterBonus;
    int fixtureBonus = 0;
    for (const auto &p : cardPower) fixtureBonus += p.fixtureBonus;
    int gateBonus = 0;
    for (const auto &p : cardPower) gateBonus += p.gateBonus;
    int total = 0;
    for (const auto &p : cardPower) total += p.total;
    total += honorBonus;
    DeckPowerDetail power = { 
        base, 
        areaItemBonus, 
        characterBonus, 
        honorBonus, 
        fixtureBonus, 
        gateBonus, 
        total 
    };

    // 计算当前卡组的技能效果，并归纳卡牌在队伍中的详情信息
    std::vector<DeckCardDetail> cards{};
    for (int i = 0; i < int(cardDetails.size()); ++i) {
        auto& cardDetail = *cardDetails[i];
        DeckCardSkillDetail skill = {};
        for (const auto &unit : cardDetail.units) {
            auto current = cardDetail.skill.get(unit, unit_map[unit], 1);
            skill = current.scoreUp > skill.scoreUp ? current : skill;
        }

        // 打补丁计算某些技能
        auto& skillMeta = cardDetail.skill.meta;
        if (skillMeta.count("type")) {
            int type = std::any_cast<int>(skillMeta["type"]);
            if (type == other_member_score_up_reference_rate_enum) {
                // oc bfes花前（随机取一个成员的最大加成乘比例）
                double rate = std::any_cast<double>(skillMeta["rate"]);
                double maxValue = std::any_cast<double>(skillMeta["max"]);
                std::vector<double> memberSkillMaxs = {};
                for (int j = 0; j < int(cardDetails.size()); ++j) if (i != j) {
                    double m = cardDetails[j]->skill.max;
                    m = std::min(m * rate / 100., maxValue);
                    memberSkillMaxs.push_back(m);
                }
                double chosenSkillMax = 0;
                // 不同选择策略
                if (skillReferenceChooseStrategy == SkillReferenceChooseStrategy::Max) 
                    chosenSkillMax = *std::max_element(memberSkillMaxs.begin(), memberSkillMaxs.end());
                else if (skillReferenceChooseStrategy == SkillReferenceChooseStrategy::Min)
                    chosenSkillMax = *std::min_element(memberSkillMaxs.begin(), memberSkillMaxs.end());
                else if (skillReferenceChooseStrategy == SkillReferenceChooseStrategy::Average)
                    chosenSkillMax = std::accumulate(memberSkillMaxs.begin(), memberSkillMaxs.end(), 0.0) / memberSkillMaxs.size();
                skill.scoreUp += chosenSkillMax;
            } 
            else if (type == score_up_unit_count_enum) {
                // vs bfes花前技能（按照不同团数量）
                int unitCount = 0;
                for(int i = 0; i < 10; i++)
                    if (unit_map[i] > 0 && i != piapro_unit_enum)
                        unitCount++;
                auto& valuesMap = std::any_cast<std::map<int, double>&>(skillMeta["value"]);
                double value = 0;
                if (unitCount < valuesMap.begin()->first) 
                    value = 0;
                else if (unitCount >= valuesMap.rbegin()->first) 
                    value = valuesMap.rbegin()->second;
                else 
                    value = valuesMap[unitCount];
                skill.scoreUp += value;
            }
        }

        cards.push_back(DeckCardDetail{ 
            cardDetail.cardId, 
            cardDetail.level, 
            cardDetail.skillLevel, 
            cardDetail.masterRank, 
            cardPower.at(i),
            cardDetail.eventBonus, 
            skill,
            cardDetail.episode1Read,
            cardDetail.episode2Read,
            cardDetail.afterTraining,
            cardDetail.defaultImage,
            cardDetail.hasCanvasBonus,
        });
    }

    // 计算卡组活动加成（与顺序无关，不用考虑deckCardOrder）
    auto eventBonus = getDeckBonus(cardDetails, eventType);

    // （与顺序无关，不用考虑deckCardOrder）
    auto supportDeckBonus = this->getSupportDeckBonus(cardDetails, allCards, this->getWorldBloomSupportDeckCount(eventId.value_or(0)));

    return DeckDetail{ 
        power, 
        eventBonus, 
        supportDeckBonus.bonus,
        std::nullopt, // supportDeckBonus.cards, // for debug
        cards 
    };
}


int DeckCalculator::getWorldBloomSupportDeckCount(int eventId) const
{
    int turn = this->dataProvider.masterData->getWorldBloomEventTurn(eventId);
    // wl1 12 wl2 20
    return turn == 1 ? 12 : 20;
}
