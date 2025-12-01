#ifndef USER_GAMEDATA_H
#define USER_GAMEDATA_H

#include "common/collection-utils.h"

struct UserGameData {
    int userId = 0;
    int deck = 0;
    int customProfileId = 0;
    int rank = 0;
    int exp = 0;
    int totalExp = 0;
    int coin = 0;
    int virtualCoin = 0;

    static inline UserGameData fromJson(const json& jsonData) {
        UserGameData userGameData;
        userGameData.userId = jsonData.value("userId", 0);
        userGameData.deck = jsonData.value("deck", 0);
        userGameData.customProfileId = jsonData.value("customProfileId", 0);
        userGameData.rank = jsonData.value("rank", 0);
        userGameData.exp = jsonData.value("exp", 0);
        userGameData.totalExp = jsonData.value("totalExp", 0);
        userGameData.coin = jsonData.value("coin", 0);
        userGameData.virtualCoin = jsonData.value("virtualCoin", 0);
        return userGameData;
    }
};

#endif // USER_GAMEDATA_H