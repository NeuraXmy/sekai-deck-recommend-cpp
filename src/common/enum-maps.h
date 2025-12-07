#ifndef ENUM_MAPS_H
#define ENUM_MAPS_H

#include <string>
#include <unordered_map>

enum class EnumMap {
    unit,
    attr,
    areaType,
    viewType,
    cardEpisodePartType,
    cardRarityType,
    cardParameterType,
    eventType,
    supportUnitType,
    honorRarity,
    musicDifficulty,
    characterType,
    musicVocalType,
    skillEffectType,
    activateNotesJudgmentType,
    conditionType,
    activateEffectValueType,
    skillEnhanceType,
    worldBloomSupportDeckCharacterType,
    actionSetStatus,
    areaStatus,
    scenarioStatus,
    specialTrainingStatus,
    defaultImage,
    liveType,

    _ENUM_MAP_NUM,
};


inline std::unordered_map<std::string, int>& getEnumMap(EnumMap map_id) {
    static std::unordered_map<std::string, int> maps[static_cast<int>(EnumMap::_ENUM_MAP_NUM)];
    return maps[static_cast<int>(map_id)];
}

inline std::unordered_map<int, std::string>& getEnumReverseMap(EnumMap map_id) {
    static std::unordered_map<int, std::string> maps[static_cast<int>(EnumMap::_ENUM_MAP_NUM)];
    return maps[static_cast<int>(map_id)];
}

inline int mapEnum(EnumMap map_id, const std::string& key) {
    auto& map = getEnumMap(map_id);
    auto mapIt = map.find(key);
    if (mapIt != map.end()) 
        return mapIt->second;
    else {
        int value = map.size() + 1;
        map[key] = value;
        getEnumReverseMap(map_id)[value] = key;
        return value;
    }
}

inline std::string mappedEnumToString(EnumMap map_id, int key)
{
    auto& map = getEnumReverseMap(map_id);
    auto mapIt = map.find(key);
    if (mapIt != map.end()) 
        return mapIt->second;
    throw std::runtime_error("Key " + std::to_string(key) + " not found in map " + std::to_string(static_cast<int>(map_id)));
}

inline std::vector<int> mapEnumList(EnumMap map_id) {
    std::vector<int> result;
    auto& map = getEnumReverseMap(map_id);
    for (const auto& pair : map) {
        result.push_back(pair.first);
    }
    return result;
}


#endif  // ENUM_MAPS_H