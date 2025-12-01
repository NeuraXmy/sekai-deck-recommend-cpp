#ifndef AREA_ITEM_H
#define AREA_ITEM_H

#include "common/collection-utils.h"

struct AreaItem {
    int id = 0;
    int areaId = 0;
    
    static inline std::vector<AreaItem> fromJsonList(const json& jsonData) {
        std::vector<AreaItem> areaItems;
        for (const auto& item : jsonData) {
            AreaItem areaItem;
            areaItem.id = item.value("id", 0);
            areaItem.areaId = item.value("areaId", 0);
            areaItems.push_back(areaItem);
        }
        return areaItems;
    }
};

#endif