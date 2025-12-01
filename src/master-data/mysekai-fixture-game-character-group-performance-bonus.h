#ifndef MYSEKAI_FIXTURE_GAME_CHARACTER_GROUP_PERFORMANCE_BONUS_H
#define MYSEKAI_FIXTURE_GAME_CHARACTER_GROUP_PERFORMANCE_BONUS_H

#include "common/collection-utils.h"

struct MysekaiFixtureGameCharacterGroupPerformanceBonus {
    int id;
    int mysekaiFixtureGameCharacterGroupId;
    double bonusRate; 

    static inline std::vector<MysekaiFixtureGameCharacterGroupPerformanceBonus> fromJsonList(const json& jsonData) {
        std::vector<MysekaiFixtureGameCharacterGroupPerformanceBonus> bonuses;
        for (const auto& item : jsonData) {
            MysekaiFixtureGameCharacterGroupPerformanceBonus bonus;
            bonus.id = item.value("id", 0);
            bonus.mysekaiFixtureGameCharacterGroupId = item.value("mysekaiFixtureGameCharacterGroupId", 0);
            bonus.bonusRate = item.value("bonusRate", 0.0);
            bonuses.push_back(bonus);
        }
        return bonuses;
    }
};

#endif