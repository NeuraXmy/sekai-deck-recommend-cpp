#ifndef AREA_H
#define AREA_H

#include "common/collection-utils.h"

struct Area {
    int id = 0;
    int areaType = 0;
    int viewType = 0;

    static inline std::vector<Area> fromJsonList(const json& jsonData) {
        std::vector<Area> areas;
        for (const auto& item : jsonData) {
            Area area;
            area.id = item.value("id", 0);
            area.areaType = mapEnum(EnumMap::areaType, item.value("areaType", ""));
            area.viewType = mapEnum(EnumMap::viewType, item.value("viewType", ""));
            areas.push_back(area);
        }
        return areas;
    }
};


#endif