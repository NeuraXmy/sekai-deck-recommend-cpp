#ifndef CHARACTER_RANKS_H
#define CHARACTER_RANKS_H

#include "common/collection-utils.h"

struct CharacterRank {
    int id;
    int characterId;
    int characterRank;
    double power1BonusRate;
    double power2BonusRate;
    double power3BonusRate;

    static inline std::vector<CharacterRank> fromJsonList(const json& jsonData) {
        std::vector<CharacterRank> characterRanks;
        for (const auto& item : jsonData) {
            CharacterRank characterRank;
            characterRank.id = item.value("id", 0);
            characterRank.characterId = item.value("characterId", 0);
            characterRank.characterRank = item.value("characterRank", 0);
            characterRank.power1BonusRate = item.value("power1BonusRate", 0.0);
            characterRank.power2BonusRate = item.value("power2BonusRate", 0.0);
            characterRank.power3BonusRate = item.value("power3BonusRate", 0.0);
            characterRanks.push_back(characterRank);
        }
        return characterRanks;
    }
};


#endif // CHARACTER_RANKS_H