#ifndef USER_MYSEKAI_FIXTURE_GAME_CHARACTER_PERFORMANCE_BONUS_H
#define USER_MYSEKAI_FIXTURE_GAME_CHARACTER_PERFORMANCE_BONUS_H

#include "common/collection-utils.h"

struct UserMysekaiFixtureGameCharacterPerformanceBonus {
    int gameCharacterId = 0;
    double totalBonusRate = 0.0;

    static inline std::vector<UserMysekaiFixtureGameCharacterPerformanceBonus> fromJsonList(const json& jsonData) {
        std::vector<UserMysekaiFixtureGameCharacterPerformanceBonus> performanceBonuses;
        for (const auto& item : jsonData) {
            UserMysekaiFixtureGameCharacterPerformanceBonus performanceBonus;
            performanceBonus.gameCharacterId = item.value("gameCharacterId", 0);
            performanceBonus.totalBonusRate = item.value("totalBonusRate", 0.0);
            performanceBonuses.push_back(performanceBonus);
        }
        return performanceBonuses;
    }
};

#endif // USER_MYSEKAI_FIXTURE_GAME_CHARACTER_PERFORMANCE_BONUS_H