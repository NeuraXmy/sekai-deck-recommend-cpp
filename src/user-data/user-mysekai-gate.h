#ifndef USER_MYSEKAI_GATE_H
#define USER_MYSEKAI_GATE_H

#include "common/collection-utils.h"

struct UserMysekaiGate {
    int mysekaiGateId;
    int mysekaiGateSkinId;
    int mysekaiGateLevel;
    int visitCount;
    bool isSettingAtHomeSite;

    static inline std::vector<UserMysekaiGate> fromJsonList(const json& jsonData) {
        std::vector<UserMysekaiGate> userMysekaiGates;
        for (const auto& item : jsonData) {
            UserMysekaiGate userMysekaiGate;
            userMysekaiGate.mysekaiGateId = item.value("mysekaiGateId", 0);
            userMysekaiGate.mysekaiGateSkinId = item.value("mysekaiGateSkinId", 0);
            userMysekaiGate.mysekaiGateLevel = item.value("mysekaiGateLevel", 0);
            userMysekaiGate.visitCount = item.value("visitCount", 0);
            userMysekaiGate.isSettingAtHomeSite = item.value("isSettingAtHomeSite", false);
            userMysekaiGates.push_back(userMysekaiGate);
        }
        return userMysekaiGates;
    }
};

#endif