#ifndef SKILL_H
#define SKILL_H

#include "common/collection-utils.h"

struct SkillEffectDetail {
    int id = 0;
    int level = 0;
    double activateEffectDuration = 0;
    int activateEffectValueType = 0;
    double activateEffectValue = 0;
    double activateEffectValue2 = 0;

    static inline std::vector<SkillEffectDetail> fromJsonList(const json& jsonData) {
        std::vector<SkillEffectDetail> skillEffectDetails;
        for (const auto& item : jsonData) {
            SkillEffectDetail skillEffectDetail;
            skillEffectDetail.id = item.value("id", 0);
            skillEffectDetail.level = item.value("level", 0);
            skillEffectDetail.activateEffectDuration = item.value("activateEffectDuration", 0.0);
            skillEffectDetail.activateEffectValueType = mapEnum(EnumMap::activateEffectValueType, item.value("activateEffectValueType", ""));
            skillEffectDetail.activateEffectValue = item.value("activateEffectValue", 0.0);
            skillEffectDetail.activateEffectValue2 = item.value("activateEffectValue2", 0.0);
            skillEffectDetails.push_back(skillEffectDetail);
        }
        return skillEffectDetails;
    }
};

struct SkillEnhanceCondition {
    int id = 0;
    int seq = 0;
    int unit = 0;

    static inline SkillEnhanceCondition fromJson(const json& jsonData) {
        SkillEnhanceCondition skillEnhanceCondition;
        skillEnhanceCondition.id = jsonData.value("id", 0);
        skillEnhanceCondition.seq = jsonData.value("seq", 0);
        skillEnhanceCondition.unit = mapEnum(EnumMap::unit, jsonData.value("unit", ""));
        return skillEnhanceCondition;
    }
};

struct SkillEnhance {
    int id = 0;
    int skillEnhanceType = 0;
    int activateEffectValueType = 0;
    double activateEffectValue = 0;
    SkillEnhanceCondition skillEnhanceCondition;

    static inline SkillEnhance fromJson(const json& jsonData) {
        SkillEnhance skillEnhance;
        skillEnhance.id = jsonData.value("id", 0);
        skillEnhance.skillEnhanceType = mapEnum(EnumMap::skillEnhanceType, jsonData.value("skillEnhanceType", ""));
        skillEnhance.activateEffectValueType = mapEnum(EnumMap::activateEffectValueType, jsonData.value("activateEffectValueType", ""));
        skillEnhance.activateEffectValue = jsonData.value("activateEffectValue", 0.0);
        skillEnhance.skillEnhanceCondition = SkillEnhanceCondition::fromJson(jsonData.value("skillEnhanceCondition", json()));
        return skillEnhance;
    }
};

struct SkillEffect {
    int id = 0;
    int skillEffectType = 0;
    int activateNotesJudgmentType = 0;
    int activateCharacterRank = 0;
    int activateUnitCount = 0;
    int conditionType = 0;
    std::vector<SkillEffectDetail> skillEffectDetails;
    std::optional<SkillEnhance> skillEnhance = std::nullopt;

    static inline std::vector<SkillEffect> fromJsonList(const json& jsonData) {
        std::vector<SkillEffect> skillEffects;
        for (const auto& item : jsonData) {
            SkillEffect skillEffect;
            skillEffect.id = item.value("id", 0);
            skillEffect.skillEffectType = mapEnum(EnumMap::skillEffectType, item.value("skillEffectType", ""));
            skillEffect.activateNotesJudgmentType = mapEnum(EnumMap::activateNotesJudgmentType, item.value("activateNotesJudgmentType", ""));
            skillEffect.activateCharacterRank = item.value("activateCharacterRank", 0);
            skillEffect.activateUnitCount = item.value("activateUnitCount", 0);
            skillEffect.conditionType = mapEnum(EnumMap::conditionType, item.value("conditionType", ""));
            skillEffect.skillEffectDetails = SkillEffectDetail::fromJsonList(item.value("skillEffectDetails", json::array()));
            if (item.contains("skillEnhance")) {
                skillEffect.skillEnhance = SkillEnhance::fromJson(item.at("skillEnhance"));
            }
            skillEffects.push_back(skillEffect);
        }
        return skillEffects;
    }
};

struct Skill {
    int id = 0;
    int skillFilterId = 0;
    std::vector<SkillEffect> skillEffects;

    static inline std::vector<Skill> fromJsonList(const json& jsonData) {
        std::vector<Skill> skills;
        for (const auto& item : jsonData) {
            Skill skill;
            skill.id = item.value("id", 0);
            skill.skillFilterId = item.value("skillFilterId", 0);
            skill.skillEffects = SkillEffect::fromJsonList(item.value("skillEffects", json::array()));
            skills.push_back(skill);
        }
        return skills;
    }
};

#endif