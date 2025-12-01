#ifndef CARD_EPISODE_H
#define CARD_EPISODE_H

#include "common/collection-utils.h"

struct CardEpisode {
    int id = 0;
    int seq = 0;
    int cardId = 0;
    int power1BonusFixed = 0;
    int power2BonusFixed = 0;
    int power3BonusFixed = 0;
    int cardEpisodePartType = 0;

    static inline std::vector<CardEpisode> fromJsonList(const json& jsonData) {
        std::vector<CardEpisode> cardEpisodes;
        for (const auto& item : jsonData) {
            CardEpisode cardEpisode;
            cardEpisode.id = item.value("id", 0);
            cardEpisode.seq = item.value("seq", 0);
            cardEpisode.cardId = item.value("cardId", 0);
            cardEpisode.power1BonusFixed = item.value("power1BonusFixed", 0);
            cardEpisode.power2BonusFixed = item.value("power2BonusFixed", 0);
            cardEpisode.power3BonusFixed = item.value("power3BonusFixed", 0);
            cardEpisode.cardEpisodePartType = mapEnum(EnumMap::cardEpisodePartType, item.value("cardEpisodePartType", ""));
            cardEpisodes.push_back(cardEpisode);
        }
        return cardEpisodes;
    }
};


#endif