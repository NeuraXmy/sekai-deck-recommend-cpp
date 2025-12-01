#ifndef USER_DECK_H
#define USER_DECK_H

#include "common/collection-utils.h"

struct UserDeck {
    long long userId = 0;
    int deckId = 0;
    int leader = 0;
    int subLeader = 0;
    int member1 = 0;
    int member2 = 0;
    int member3 = 0;
    int member4 = 0;
    int member5 = 0;

    static inline std::vector<UserDeck> fromJsonList(const json& jsonData) {
        std::vector<UserDeck> userDecks;
        for (const auto& item : jsonData) {
            UserDeck userDeck;
            userDeck.userId = item.value("userId", 0ll);
            userDeck.deckId = item.value("deckId", 0);
            userDeck.leader = item.value("leader", 0);
            userDeck.subLeader = item.value("subLeader", 0);
            userDeck.member1 = item.value("member1", 0);
            userDeck.member2 = item.value("member2", 0);
            userDeck.member3 = item.value("member3", 0);
            userDeck.member4 = item.value("member4", 0);
            userDeck.member5 = item.value("member5", 0);
            userDecks.push_back(userDeck);
        }
        return userDecks;
    }
};

#endif