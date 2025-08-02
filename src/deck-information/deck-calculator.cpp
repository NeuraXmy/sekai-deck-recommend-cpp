#include "deck-information/deck-calculator.h"
#include "common/timer.h"
#include "deck-calculator.h"

static int world_bloom_enum = mapEnum(EnumMap::eventType, "world_bloom");
static int piapro_unit_enum = mapEnum(EnumMap::unit, "piapro");

static int other_member_score_up_reference_rate_enum    = mapEnum(EnumMap::skillEffectType, "other_member_score_up_reference_rate");
static int score_up_unit_count_enum                     = mapEnum(EnumMap::skillEffectType, "score_up_unit_count");

static int default_image_original_enum = mapEnum(EnumMap::defaultImage, "original");
static int default_image_special_training_enum = mapEnum(EnumMap::defaultImage, "special_training");


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


DeckDetail DeckCalculator::getDeckDetailByCards(
    const std::vector<const CardDetail*> &cardDetails, 
    const std::vector<CardDetail> &allCards, 
    int honorBonus, 
    std::optional<int> eventType,
    std::optional<int> eventId,
    SkillReferenceChooseStrategy skillReferenceChooseStrategy,
    bool keepAfterTrainingState
)
{
    int card_num = int(cardDetails.size());
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
    int unit_num = 0;
    for (int i = 0; i < 10; ++i) 
        unit_num += bool(unit_map[i]);

    // 计算当前卡组的综合力，要加上称号的固定加成
    std::vector<DeckCardPowerDetail> cardPower{};
    cardPower.reserve(card_num);
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

    DeckPowerDetail power{};
    for (const auto &p : cardPower) power.base += p.base;
    for (const auto &p : cardPower) power.areaItemBonus += p.areaItemBonus;
    for (const auto &p : cardPower) power.characterBonus += p.characterBonus;
    for (const auto &p : cardPower) power.fixtureBonus += p.fixtureBonus;
    for (const auto &p : cardPower) power.gateBonus += p.gateBonus;
    for (const auto &p : cardPower) power.total += p.total;
    power.total += honorBonus;

    // 计算当前卡组每个卡牌的花前/花后固定技能效果（进Live之前）
    std::vector<std::array<DeckCardSkillDetail, 2>> prepareSkills{};
    prepareSkills.reserve(card_num);
    int skill1LargerNum = 0;
    for (int i = 0; i < card_num; ++i) {
        auto& cardDetail = *cardDetails[i];
        // 获取普通技能效果（所有普通技能&bf花后）
        DeckCardSkillDetail s2 = {};
        // 组分技能效果（对vs有多个组合取最大）或 固定技能效果
        for (const auto &unit : cardDetail.units) {
            auto current = cardDetail.skill.get(unit, unit_map[unit], 1);
            if (current.scoreUp > s2.scoreUp) s2 = current;
        }

        // 获取bf花前技能效果
        DeckCardSkillDetail s1 = {};
        // 吸分技能效果(max)
        auto current = cardDetail.skill.get(ref_unit_enum, 1, 1);
        current.scoreUp += current.scoreUpReferenceMax;   
        if (current.skillId != s2.skillId && current.scoreUp > s1.scoreUp) s1 = current;
        // 异组技能效果
        current = cardDetail.skill.get(diff_unit_enum, unit_num - 1, 1);
        if (current.skillId != s2.skillId && current.scoreUp > s1.scoreUp) s1 = current;
        // 没有双技能则复制普通技能
        if (s1.skillId == 0) s1 = s2;

        // 统计 花后 < 花前max 的情况数量，后面会用到
        skill1LargerNum += int(s2.scoreUp < s1.scoreUp);
        
        prepareSkills.push_back({ s1, s2 });
    }

    // 计算当前卡组的实际技能效果（包括选择花前/花后技能），并归纳卡牌在队伍中的详情信息
    std::vector<DeckCardDetail> cards{};
    cards.reserve(card_num);
    for (int i = 0; i < card_num; ++i) {
        auto& cardDetail = *cardDetails[i];
        auto& s1 = prepareSkills[i][0]; // 花前技能
        auto& s2 = prepareSkills[i][1]; // 花后技能
        DeckCardSkillDetail skill = {}; // 实际技能

        // 计算花前技能的实际值
        auto s = s1; // 实际花前技能
        if (s.hasScoreUpReference) {
            s.scoreUp -= s.scoreUpReferenceMax; // 从max回到还没吸的基础值
            // 收集其他成员的技能最大值
            std::vector<double> memberSkillMaxs = {};
            for (int j = 0; j < card_num; ++j) if (i != j) {
                double m = 0;
                if (keepAfterTrainingState) {
                    // 吸取对应花前花后状态的技能最大值
                    m = cardDetails[j]->defaultImage == default_image_special_training_enum ? 
                        prepareSkills[j][1].scoreUp : prepareSkills[j][0].scoreUp;
                } else {
                    /**
                     * 这里能够直接吸取其他成员花前花后最大的一个，而不用考虑其他卡的实际状态
                     * 因为其他卡如果是下文的 情况1 则取的确实是最大的花后
                     * 情况2.b 其他卡取的确实是最大的花前
                     * 情况2.a 不会进入该计算，因为全场只有一张卡为情况2，与i和j都是情况2矛盾
                     */
                    m = std::max(prepareSkills[j][0].scoreUp, prepareSkills[j][1].scoreUp);
                }
                m = std::min(std::floor(m * s.scoreUpReferenceRate / 100.), s.scoreUpReferenceMax);
                memberSkillMaxs.push_back(m);
            }
            // 不同选择策略
            double chosenSkillMax = 0;
            if (skillReferenceChooseStrategy == SkillReferenceChooseStrategy::Max) 
                chosenSkillMax = *std::max_element(memberSkillMaxs.begin(), memberSkillMaxs.end());
            else if (skillReferenceChooseStrategy == SkillReferenceChooseStrategy::Min)
                chosenSkillMax = *std::min_element(memberSkillMaxs.begin(), memberSkillMaxs.end());
            else if (skillReferenceChooseStrategy == SkillReferenceChooseStrategy::Average)
                chosenSkillMax = std::accumulate(memberSkillMaxs.begin(), memberSkillMaxs.end(), 0.0) / memberSkillMaxs.size();
            s.scoreUp += chosenSkillMax; 
        } 

        if (keepAfterTrainingState) {
            // 保持用户选择的花前花后状态技能
            skill = cardDetail.defaultImage == default_image_special_training_enum ? s2 : s;

        } else {
            // 最优技能选择
            if(s2.scoreUp >= s1.scoreUp) {
                /**
                 * 1. 花后 >= 花前max 的情况（由于花后只有固定值技能，因此这里花后实际=花后max）
                 * 可以不管其他双技能卡牌的状态直接选花后技能 (因为选更高的总能让其他卡吸取的分更多)
                 * 此外，如果没有双技能则s1s2是一样的技能，也会直接走这边
                 */
                skill = s2;
            } else {
                /**
                 * 2. 花后 < 花前max 的情况
                 *  a. 如果只有一张卡，直接计算实际值选取最大的一个
                 *  b. 如果有多张卡，全取花前max即可最优
                 *     原因: 对于非吸分花前技能(ocbf花前)，花前max就是实际值，max大实际就大
                 *     对于吸分花前技能(vsbf花前)，多张卡全取花前，则吸取的数值可以让各自全都达到max
                 */
                if(skill1LargerNum > 1) {
                    // 如果有多张卡，全取花前max即可最优   
                    skill = s;
                } else {
                    // 考虑花前可能有随机性，相等优先选择花后
                    skill = s2.scoreUp >= s.scoreUp ? s2 : s;   
                }
            }
        }

        // 如果确实是双技能，根据技能调整卡面状态
        int defaultImage = cardDetail.defaultImage;
        if (s1.skillId != s2.skillId) {
            defaultImage = skill.isAfterTraining ? default_image_special_training_enum : default_image_original_enum;
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
            defaultImage,
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
