#ifndef HONOR_H
#define HONOR_H

#include "common/collection-utils.h"

struct HonorLevel {
    int honorId;
    int level;
    int bonus;

    static inline std::vector<HonorLevel> fromJsonList(const json& jsonData) {
        std::vector<HonorLevel> honorLevels;
        for (const auto& item : jsonData) {
            HonorLevel honorLevel;
            honorLevel.honorId = item.value("honorId", 0);
            honorLevel.level = item.value("level", 0);
            honorLevel.bonus = item.value("bonus", 0);
            honorLevels.push_back(honorLevel);
        }
        return honorLevels;
    }
};

struct Honor {
    int id;
    int seq;
    int groupId;
    int honorRarity;
    std::vector<HonorLevel> levels;
    std::string assetbundleName;

    static inline std::vector<Honor> fromJsonList(const json& jsonData) {
        std::vector<Honor> honors;
        for (const auto& item : jsonData) {
            Honor honor;
            honor.id = item.value("id", 0);
            honor.seq = item.value("seq", 0);
            honor.groupId = item.value("groupId", 0);
            honor.honorRarity = mapEnum(EnumMap::honorRarity, item.value("honorRarity", ""));
            honor.levels = HonorLevel::fromJsonList(item.value("levels", json::array()));
            honor.assetbundleName = item.value("assetbundleName", "");
            honors.push_back(honor);
        }
        return honors;
    }
};


#endif // HONOR_H