#ifndef USER_WORLD_BLOOM_SUPPORT_DECK_H
#define USER_WORLD_BLOOM_SUPPORT_DECK_H

#include "common/collection-utils.h"

struct UserWorldBloomSupportDeck {
    int gameCharacterId = 0;
    int member1 = 0;
    int member2 = 0;
    int member3 = 0;
    int member4 = 0;
    int member5 = 0;
    int member6 = 0;
    int member7 = 0;
    int member8 = 0;
    int member9 = 0;
    int member10 = 0;
    int member11 = 0;
    int member12 = 0;

    static inline std::vector<UserWorldBloomSupportDeck> fromJsonList(const json& jsonData) {
        std::vector<UserWorldBloomSupportDeck> userWorldBloomSupportDecks;
        for (const auto& item : jsonData) {
            UserWorldBloomSupportDeck userWorldBloomSupportDeck;
            userWorldBloomSupportDeck.gameCharacterId = item.value("gameCharacterId", 0);
            userWorldBloomSupportDeck.member1 = item.value("member1", 0);
            userWorldBloomSupportDeck.member2 = item.value("member2", 0);
            userWorldBloomSupportDeck.member3 = item.value("member3", 0);
            userWorldBloomSupportDeck.member4 = item.value("member4", 0);
            userWorldBloomSupportDeck.member5 = item.value("member5", 0);
            userWorldBloomSupportDeck.member6 = item.value("member6", 0);
            userWorldBloomSupportDeck.member7 = item.value("member7", 0);
            userWorldBloomSupportDeck.member8 = item.value("member8", 0);
            userWorldBloomSupportDeck.member9 = item.value("member9", 0);
            userWorldBloomSupportDeck.member10 = item.value("member10", 0);
            userWorldBloomSupportDeck.member11 = item.value("member11", 0);
            userWorldBloomSupportDeck.member12 = item.value("member12", 0);
            userWorldBloomSupportDecks.push_back(userWorldBloomSupportDeck);
        }
        return userWorldBloomSupportDecks;
    }
};

#endif // USER_WORLD_BLOOM_SUPPORT_DECK_H