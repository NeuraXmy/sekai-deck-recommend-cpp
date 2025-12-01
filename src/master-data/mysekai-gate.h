#ifndef MYSEKAI_GATE_H
#define MYSEKAI_GATE_H

#include <common/collection-utils.h>

struct MysekaiGate {
    int id;
    int unit;
    std::string assetbundleName;

    static inline std::vector<MysekaiGate> fromJsonList(const json& jsonData) {
        std::vector<MysekaiGate> mysekaiGates;
        for (const auto& item : jsonData) {
        MysekaiGate mysekaiGate;
        mysekaiGate.id = item.value("id", 0);
        mysekaiGate.unit = mapEnum(EnumMap::unit, item.value("unit", ""));
        mysekaiGate.assetbundleName = item.value("assetbundleName", "");
        mysekaiGates.push_back(mysekaiGate);
        }
        return mysekaiGates;
    }
};

#endif