#ifndef AREA_ITEM_LEVEL_H
#define AREA_ITEM_LEVEL_H

#include "common/collection-utils.h"

struct AreaItemLevel {
    int areaItemId = 0;
    int level = 0;
    int targetUnit = 0;
    int targetCardAttr = 0;
    int targetGameCharacterId = 0;
    double power1BonusRate = 0.0;
    double power1AllMatchBonusRate = 0.0;
    double power2BonusRate = 0.0;
    double power2AllMatchBonusRate = 0.0;
    double power3BonusRate = 0.0;
    double power3AllMatchBonusRate = 0.0;

    static inline std::vector<AreaItemLevel> fromJsonList(const json& jsonData) {
        std::vector<AreaItemLevel> areaItemLevels;
        for (const auto& item : jsonData) {
            AreaItemLevel areaItemLevel;
            areaItemLevel.areaItemId = item.value("areaItemId", 0);
            areaItemLevel.level = item.value("level", 0);
            areaItemLevel.targetUnit = mapEnum(EnumMap::unit, item.value("targetUnit", ""));
            areaItemLevel.targetCardAttr = mapEnum(EnumMap::attr, item.value("targetCardAttr", ""));
            areaItemLevel.targetGameCharacterId = item.value("targetGameCharacterId", 0);
            areaItemLevel.power1BonusRate = item.value("power1BonusRate", 0.0);
            areaItemLevel.power1AllMatchBonusRate = item.value("power1AllMatchBonusRate", 0.0);
            areaItemLevel.power2BonusRate = item.value("power2BonusRate", 0.0);
            areaItemLevel.power2AllMatchBonusRate = item.value("power2AllMatchBonusRate", 0.0);
            areaItemLevel.power3BonusRate = item.value("power3BonusRate", 0.0);
            areaItemLevel.power3AllMatchBonusRate = item.value("power3AllMatchBonusRate", 0.0);
            areaItemLevels.push_back(areaItemLevel);
        }
        return areaItemLevels;
    }
};

#endif // AREA_ITEM_LEVEL_H