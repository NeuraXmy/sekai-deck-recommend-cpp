#ifndef USER_CHALLENGE_LIVE_SOLO_DECK_H
#define USER_CHALLENGE_LIVE_SOLO_DECK_H

#include "common/collection-utils.h"

struct UserChallengeLiveSoloDeck {
    int characterId = 0;
    int leader = 0;
    int support1 = 0;
    int support2 = 0;
    int support3 = 0;
    int support4 = 0;

    static inline std::vector<UserChallengeLiveSoloDeck> fromJsonList(const json& jsonData) {
        std::vector<UserChallengeLiveSoloDeck> userChallengeLiveSoloDecks;
        for (const auto& item : jsonData) {
            UserChallengeLiveSoloDeck userChallengeLiveSoloDeck;
            userChallengeLiveSoloDeck.characterId = item.value("characterId", 0);
            userChallengeLiveSoloDeck.leader = item.value("leader", 0);
            userChallengeLiveSoloDeck.support1 = item.value("support1", 0);
            userChallengeLiveSoloDeck.support2 = item.value("support2", 0);
            userChallengeLiveSoloDeck.support3 = item.value("support3", 0);
            userChallengeLiveSoloDeck.support4 = item.value("support4", 0);
            userChallengeLiveSoloDecks.push_back(userChallengeLiveSoloDeck);
        }
        return userChallengeLiveSoloDecks;
    }
};

#endif // USER_CHALLENGE_LIVE_SOLO_DECK_H