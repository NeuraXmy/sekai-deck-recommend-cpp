#include "card-information/card-skill-calculator.h"


static int score_up_enum = mapEnum(EnumMap::skillEffectType, "score_up");
static int score_up_condition_life_enum = mapEnum(EnumMap::skillEffectType, "score_up_condition_life");
static int score_up_keep_enum = mapEnum(EnumMap::skillEffectType, "score_up_keep");
static int life_recovery_enum = mapEnum(EnumMap::skillEffectType, "life_recovery");
static int score_up_character_rank_enum = mapEnum(EnumMap::skillEffectType, "score_up_character_rank");
static int special_training_enum = mapEnum(EnumMap::defaultImage, "special_training");


CardDetailMap<DeckCardSkillDetail> CardSkillCalculator::getCardSkill(const UserCard &userCard, const Card &card)
{
    CardDetailMap<DeckCardSkillDetail> skillMap{};
    SkillDetail detail = getSkillDetail(userCard, card);
    if (detail.hasScoreUpEnhance) {
        // 组合相关，处理不同人数的情况
        for (int i = 1; i <= 5; ++i) {
            // 如果全部同队还有一次额外加成
            double scoreUp = detail.scoreUp + (i == 5 ? 5 : (i - 1)) * detail.scoreUpEnhanceValue;
            skillMap.set(detail.scoreUpEnhanceUnit, i, 1, scoreUp, {scoreUp, detail.lifeRecovery});
        }
    }
    // 固定加成，即便是组分也有个保底加成
    skillMap.set(any_unit_enum, 1, 1, detail.scoreUp, {detail.scoreUp, detail.lifeRecovery});
    return skillMap;
}

SkillDetail CardSkillCalculator::getSkillDetail(const UserCard &userCard, const Card &card)
{
    SkillDetail ret = {0, 0, false, 0, 0};
    auto skill = getSkill(userCard, card);
    auto characterRank = getCharacterRank(card.characterId);
    double characterRankBonus = 0;
    
    for (auto& skillEffect : skill.skillEffects) {
        auto skillEffectDetail = findOrThrow(skillEffect.skillEffectDetails, [&](auto& it) {
            return it.level == userCard.skillLevel;
        });
        if (skillEffect.skillEffectType == score_up_enum ||
            skillEffect.skillEffectType == score_up_condition_life_enum ||
            skillEffect.skillEffectType == score_up_keep_enum) {
            // 计算一般分卡
            double current = skillEffectDetail.activateEffectValue;
            // 组分特殊计算
            // 目前只有组分会用skillEnhance，新FES不用
            if (skillEffect.skillEnhance.has_value()) {
                ret.hasScoreUpEnhance = true;
                ret.scoreUpEnhanceUnit = skillEffect.skillEnhance->skillEnhanceCondition.unit;
                ret.scoreUpEnhanceValue = skillEffect.skillEnhance->activateEffectValue;
            }
            // 通过取max的方式，可以直接拿到判分、血分最高加成
            ret.scoreUp = std::max(ret.scoreUp, current);
        } else if (skillEffect.skillEffectType == life_recovery_enum) {
            // 计算奶卡
            ret.lifeRecovery += skillEffectDetail.activateEffectValue;
        } else if (skillEffect.skillEffectType == score_up_character_rank_enum) {
            // 计算新FES卡，角色等级额外加成
            if (skillEffect.activateCharacterRank != 0 && skillEffect.activateCharacterRank <= characterRank) {
                characterRankBonus = std::max(characterRankBonus, skillEffectDetail.activateEffectValue);
            }
        }
        // TODO 新FES卡觉醒前只能说根本没法算，有待支持
    }
    // 新FES卡角色等级额外加成
    ret.scoreUp += characterRankBonus;
    return ret;
}

Skill CardSkillCalculator::getSkill(const UserCard &userCard, const Card &card)
{
    int skillId = card.skillId;
    // 有觉醒后特殊技能且当前选择的是觉醒后
    if (card.specialTrainingSkillId != 0 && userCard.defaultImage == special_training_enum) {
        skillId = card.specialTrainingSkillId;
    }
    auto skills = dataProvider.masterData->skills;
    return findOrThrow(skills, [&](auto& it) {
        return it.id == skillId;
    });
}

int CardSkillCalculator::getCharacterRank(int characterId)
{
    auto userCharacters = dataProvider.userData->userCharacters;
    auto userCharacter = findOrThrow(userCharacters, [&](auto& it) {
        return it.characterId == characterId;
    });
    return userCharacter.characterRank;
}
