#ifndef MYSEKAI_GATE_LEVEL_H
#define MYSEKAI_GATE_LEVEL_H

#include "common/collection-utils.h"

struct MysekaiGateLevel {
    int id = 0;
    int mysekaiGateId = 0;
    int level = 0;
    int mysekaiGateMaterialGroupId = 0;
    int mysekaiGateCharacterVisitCountRateId = 0;
    double powerBonusRate = 0.0;

    static inline std::vector<MysekaiGateLevel> fromJsonList(const json& jsonData) {
        std::vector<MysekaiGateLevel> mysekaiGateLevels;
        for (const auto& item : jsonData) {
            MysekaiGateLevel mysekaiGateLevel;
            mysekaiGateLevel.id = item.value("id", 0);
            mysekaiGateLevel.mysekaiGateId = item.value("mysekaiGateId", 0);
            mysekaiGateLevel.level = item.value("level", 0);
            mysekaiGateLevel.mysekaiGateMaterialGroupId = item.value("mysekaiGateMaterialGroupId", 0);
            mysekaiGateLevel.mysekaiGateCharacterVisitCountRateId = item.value("mysekaiGateCharacterVisitCountRateId", 0);
            mysekaiGateLevel.powerBonusRate = item.value("powerBonusRate", 0.0);
            mysekaiGateLevels.push_back(mysekaiGateLevel);
        }
        return mysekaiGateLevels;
    }
};


#endif