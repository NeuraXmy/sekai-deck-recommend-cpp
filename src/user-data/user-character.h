#ifndef USER_CHARACTER_H
#define USER_CHARACTER_H

#include "common/collection-utils.h"

struct UserCharacter {
    int characterId = 0;
    int characterRank = 0;

    static inline std::vector<UserCharacter> fromJsonList(const json& jsonData) {
        std::vector<UserCharacter> userCharacters;
        for (const auto& item : jsonData) {
            UserCharacter userCharacter;
            userCharacter.characterId = item.value("characterId", 0);
            userCharacter.characterRank = item.value("characterRank", 0);
            userCharacters.push_back(userCharacter);
        }
        return userCharacters;
    }
};

#endif // USER_CHARACTER_H