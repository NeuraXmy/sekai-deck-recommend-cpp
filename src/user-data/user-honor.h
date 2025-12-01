#ifndef USER_HONOR_H
#define USER_HONOR_H

#include "common/collection-utils.h"

struct UserHonor {
    int honorId = 0;
    int level = 0;

    static inline std::vector<UserHonor> fromJsonList(const json& jsonData) {
        std::vector<UserHonor> userHonors;
        for (const auto& item : jsonData) {
            UserHonor userHonor;
            userHonor.honorId = item.value("honorId", 0);
            userHonor.level = item.value("level", 0);
            userHonors.push_back(userHonor);
        }
        return userHonors;
    }
};

#endif // USER_HONOR_H