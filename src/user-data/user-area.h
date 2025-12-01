#ifndef USER_AREA_H
#define USER_AREA_H

#include "common/collection-utils.h"

struct UserAreaActionSets {
    int id = 0;
    int status = 0;

    static inline std::vector<UserAreaActionSets> fromJsonList(const json& jsonData) {
        std::vector<UserAreaActionSets> actionSets;
        for (const auto& item : jsonData) {
            UserAreaActionSets actionSet;
            actionSet.id = item.value("id", 0);
            actionSet.status = mapEnum(EnumMap::actionSetStatus, item.value("status", ""));
            actionSets.push_back(actionSet);
        }
        return actionSets;
    }
};

struct UserAreaItems {
    int areaItemId = 0;
    int level = 0;

    static inline std::vector<UserAreaItems> fromJsonList(const json& jsonData) {
        std::vector<UserAreaItems> areaItems;
        for (const auto& item : jsonData) {
            UserAreaItems areaItem;
            areaItem.areaItemId = item.value("areaItemId", 0);
            areaItem.level = item.value("level", 0);
            areaItems.push_back(areaItem);
        }
        return areaItems;
    }
};

struct UserAreaStatus {
    int areaId = 0;
    int status = 0;

    static inline UserAreaStatus fromJson(const json& jsonData) {
        UserAreaStatus userAreaStatus;
        userAreaStatus.areaId = jsonData.value("areaId", 0);
        userAreaStatus.status = mapEnum(EnumMap::areaStatus, jsonData.value("status", ""));
        return userAreaStatus;
    }
};

struct UserArea {
    int areaId = 0;
    std::vector<UserAreaActionSets> actionSets;
    std::vector<UserAreaItems> areaItems;
    UserAreaStatus userAreaStatus;

    static inline std::vector<UserArea> fromJsonList(const json& jsonData) {
        std::vector<UserArea> userAreas;
        for (const auto& item : jsonData) {
            UserArea userArea;
            userArea.areaId = item.value("areaId", 0);
            userArea.actionSets = UserAreaActionSets::fromJsonList(item.value("actionSets", json::array()));
            userArea.areaItems = UserAreaItems::fromJsonList(item.value("areaItems", json::array()));
            userArea.userAreaStatus = UserAreaStatus::fromJson(item.value("userAreaStatus", json()));
            userAreas.push_back(userArea);
        }
        return userAreas;
    }
};

#endif // USER_AREA_H