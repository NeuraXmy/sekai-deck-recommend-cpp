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
#include "common/common-enums.h"

using TS = long long;

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