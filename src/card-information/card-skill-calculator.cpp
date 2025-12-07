#include "card-information/card-skill-calculator.h"


CardDetailMap<DeckCardSkillDetail> CardSkillCalculator::getCardSkill(
    const UserCard &userCard, 
    const Card &card, 
    std::optional<double> scoreUpLimit
)
{
    double limit = scoreUpLimit.value_or(std::numeric_limits<double>::max());

    CardDetailMap<DeckCardSkillDetail> skillMap{};
    std::vector<SkillDetail> details = { getSkillDetail(userCard, card, false) };
    if (card.specialTrainingSkillId != 0) 
        details.push_back(getSkillDetail(userCard, card, true));

    for (auto& detail : details) {
        DeckCardSkillDetail deckDetail = {
            .skillId = detail.skillId,
            .isAfterTraining = detail.isAfterTraining,
            .scoreUp = std::min(detail.scoreUp, limit),
            .lifeRecovery = detail.lifeRecovery,
        };

        // 固定加成，即便是组分也有个保底加成
        skillMap.set(Enums::Unit::any, 1, 1, deckDetail.scoreUp, deckDetail);

        // 组分
        if (detail.sameUnitScoreUp) {
            // 组合相关，处理不同人数的情况
            for (int i = 1; i <= 5; ++i) {
                // 如果全部同队还有一次额外加成
                auto dd = deckDetail;
                dd.scoreUp += (i == 5 ? 5 : (i - 1)) * detail.sameUnitScoreUp;
                dd.scoreUp = std::min(dd.scoreUp, limit);
                skillMap.set(detail.sameUnitScoreUpUnit, i, 1, dd.scoreUp, dd);
            }
        }

        // 吸收加分
        if (detail.hasScoreUpReference) {
            auto dd = deckDetail;
            dd.hasScoreUpReference = true;
            dd.scoreUpReferenceRate = detail.scoreUpReferenceRate;
            dd.scoreUpReferenceMax = detail.scoreUpReferenceMax;
            dd.scoreUpReferenceMax = std::min(dd.scoreUpReferenceMax, limit - dd.scoreUp);
            // 这边cmpValue只设置最大值就行，因为最小值被前面的固定加成设置
            skillMap.set(Enums::Unit::ref, 1, 1, dd.scoreUp + detail.scoreUpReferenceMax, dd);
        }

        // 异团数量加分
        if (detail.hasDifferentUnitCountScoreUp) {
            // 处理不同异团人数的情况
            for (int i = 0; i <= 2; ++i) {
                auto dd = deckDetail;
                if (i > 0) {
                    dd.scoreUp += detail.differentUnitCountScoreUpMap[i];
                    dd.scoreUp = std::min(dd.scoreUp, limit);
                }
                skillMap.set(Enums::Unit::diff, i, 1, dd.scoreUp, dd);
            }
        }
    }
    return skillMap;
}

SkillDetail CardSkillCalculator::getSkillDetail(const UserCard &userCard, const Card &card, bool afterTraining)
{
    SkillDetail ret = {};
    auto skill = getSkill(userCard, card, afterTraining);
    ret.skillId = skill.id;
    ret.isAfterTraining = afterTraining;

    auto characterRank = getCharacterRank(card.characterId);
    double characterRankBonus = 0;
    
    for (auto& skillEffect : skill.skillEffects) {
        auto skillEffectDetail = findOrThrow(skillEffect.skillEffectDetails, [&](auto& it) {
            return it.level == userCard.skillLevel;
        }, [&]() { return "Skill effect detail not found for skillEffectId=" + std::to_string(skillEffect.id) + " level=" + std::to_string(userCard.skillLevel); });
        if (skillEffect.skillEffectType == Enums::SkillEffectType::score_up ||
            skillEffect.skillEffectType == Enums::SkillEffectType::score_up_condition_life ||
            skillEffect.skillEffectType == Enums::SkillEffectType::score_up_keep) {
            // 一般分卡
            double current = skillEffectDetail.activateEffectValue;
            // 组分
            if (skillEffect.skillEnhance.has_value()) {
                ret.sameUnitScoreUp = true;
                ret.sameUnitScoreUpUnit = skillEffect.skillEnhance->skillEnhanceCondition.unit;
                ret.sameUnitScoreUp = skillEffect.skillEnhance->activateEffectValue;
            }
            // 通过取max的方式，可以直接拿到判分、血分最高加成
            ret.scoreUp = std::max(ret.scoreUp, current);
        } else if (skillEffect.skillEffectType == Enums::SkillEffectType::life_recovery) {
            // 奶卡
            ret.lifeRecovery += skillEffectDetail.activateEffectValue;
        } else if (skillEffect.skillEffectType == Enums::SkillEffectType::score_up_character_rank) {
            // bf角色等级额外加成
            if (skillEffect.activateCharacterRank != 0 && skillEffect.activateCharacterRank <= characterRank) {
                characterRankBonus = std::max(characterRankBonus, skillEffectDetail.activateEffectValue);
            }
        } else if (skillEffect.skillEffectType == Enums::SkillEffectType::other_member_score_up_reference_rate) {
            // oc bfes花前 吸收加分
            ret.hasScoreUpReference = true;
            ret.scoreUpReferenceRate = skillEffectDetail.activateEffectValue;
            ret.scoreUpReferenceMax = skillEffectDetail.activateEffectValue2;
        } else if (skillEffect.skillEffectType == Enums::SkillEffectType::score_up_unit_count) {
            // vs bfes花前 异团数量加分
            ret.hasDifferentUnitCountScoreUp = true;
            ret.differentUnitCountScoreUpMap[skillEffect.activateUnitCount] = skillEffectDetail.activateEffectValue;
        } else {
            // std::cerr << "[sekai-deck-recommend-cpp] warning: Unhandled skill effect type: " 
            //           << mappedEnumToString(EnumMap::skillEffectType, skillEffect.skillEffectType) 
            //           << " for card: " << card.id << std::endl;
        }
    }

    // bf角色等级额外加成
    ret.scoreUp += characterRankBonus;
    return ret;
}

Skill CardSkillCalculator::getSkill(const UserCard &userCard, const Card &card, bool afterTraining)
{
    int skillId = card.skillId;
    // 有觉醒后特殊技能且当前选择的是觉醒后
    if (card.specialTrainingSkillId != 0 && afterTraining) {
        skillId = card.specialTrainingSkillId;
    }
    auto skills = dataProvider.masterData->skills;
    return findOrThrow(skills, [&](auto& it) {
        return it.id == skillId;
    }, [&]() { return "Skill not found for skillId=" + std::to_string(skillId); });
}

int CardSkillCalculator::getCharacterRank(int characterId)
{
    auto userCharacters = dataProvider.userData->userCharacters;
    auto userCharacter = findOrThrow(userCharacters, [&](auto& it) {
        return it.characterId == characterId;
    }, [&]() { return "User character not found for characterId=" + std::to_string(characterId); });
    return userCharacter.characterRank;
}
