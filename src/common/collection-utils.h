#ifndef COLLECTION_UTILS_H
#define COLLECTION_UTILS_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>
#include <functional>
#include <iostream>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "common/enum-maps.h"

using TS = long long;

inline std::unordered_map<std::string, int> _enum_maps[static_cast<int>(EnumMap::_ENUM_MAP_NUM)];
inline std::unordered_map<int, std::string> _enum_reverse_maps[static_cast<int>(EnumMap::_ENUM_MAP_NUM)];

inline int mapEnum(EnumMap map_id, const std::string& key) {
    auto& map = _enum_maps[static_cast<int>(map_id)];
    auto mapIt = map.find(key);
    if (mapIt != map.end()) 
        return mapIt->second;
    else {
        int value = map.size() + 1;
        map[key] = value;
        _enum_reverse_maps[static_cast<int>(map_id)][value] = key;
        return value;
    }
}

inline std::string mappedEnumToString(EnumMap map_id, int key)
{
    auto& map = _enum_reverse_maps[static_cast<int>(map_id)];
    auto mapIt = map.find(key);
    if (mapIt != map.end()) 
        return mapIt->second;
    throw std::runtime_error("Key " + std::to_string(key) + " not found in map " + std::to_string(static_cast<int>(map_id)));
}

inline std::vector<int> mapEnumList(EnumMap map_id) {
    std::vector<int> result;
    auto& map = _enum_reverse_maps[static_cast<int>(map_id)];
    for (const auto& pair : map) {
        result.push_back(pair.first);
    }
    return result;
}


class ElementNoFoundError : public std::runtime_error {
public:
    ElementNoFoundError(const std::string& message) : std::runtime_error(message) {}
};


template <typename T, typename U>
const T& findOrThrow(const std::vector<T>& vec, const U& predicate) {
    auto it = std::find_if(vec.begin(), vec.end(), predicate);
    if (it == vec.end()) {
        throw ElementNoFoundError("Element not found");
    }
    return *it;
}

template <typename T, typename U>
T& findOrThrow(std::vector<T>& vec, const U& predicate) {
    auto it = std::find_if(vec.begin(), vec.end(), predicate);
    if (it == vec.end()) {
        throw ElementNoFoundError("Element not found");
    }
    return *it;
}

template <typename T, typename U>
T& findOrThrow(std::vector<T>& vec, const U& predicate, const std::string& error_msg) {
    auto it = std::find_if(vec.begin(), vec.end(), predicate);
    if (it == vec.end()) {
        throw ElementNoFoundError(error_msg);
    }
    return *it;
}

template <typename T, typename U, typename V>
T& findOrThrow(std::vector<T>& vec, const U& predicate, const V& error_msg_func) {
    auto it = std::find_if(vec.begin(), vec.end(), predicate);
    if (it == vec.end()) {
        throw ElementNoFoundError(error_msg_func());
    }
    return *it;
}


#endif // COLLECTION_UTILS_H