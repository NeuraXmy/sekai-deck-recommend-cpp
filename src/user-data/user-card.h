#ifndef USER_CARD_H
#define USER_CARD_H

#include "common/collection-utils.h"

struct UserCardEpisodes {
    int cardEpisodeId = 0;
    int scenarioStatus = 0;

    static inline std::vector<UserCardEpisodes> fromJsonList(const json& jsonData) {
        std::vector<UserCardEpisodes> episodes;
        for (const auto& item : jsonData) {
            UserCardEpisodes episode;
            episode.cardEpisodeId = item.value("cardEpisodeId", 0);
            episode.scenarioStatus = mapEnum(EnumMap::scenarioStatus, item.value("scenarioStatus", ""));
            episodes.push_back(episode);
        }
        return episodes;
    }
};

struct UserCard {
    int userId;
    int cardId;
    int level = 0;
    int exp = 0;
    int totalExp = 0;
    int skillLevel = 0;
    int skillExp = 0;
    int totalSkillExp = 0;
    int masterRank = 0;
    int specialTrainingStatus = 0;
    int defaultImage = 0;
    std::vector<UserCardEpisodes> episodes;

    static inline std::vector<UserCard> fromJsonList(const json& jsonData) {
        std::vector<UserCard> userCards;
        for (const auto& item : jsonData) {
            UserCard userCard;
            userCard.userId = item.value("userId", 0);
            userCard.cardId = item.value("cardId", 0);
            userCard.level = item.value("level", 0);
            userCard.exp = item.value("exp", 0);
            userCard.totalExp = item.value("totalExp", 0);
            userCard.skillLevel = item.value("skillLevel", 0);
            userCard.skillExp = item.value("skillExp", 0);
            userCard.totalSkillExp = item.value("totalSkillExp", 0);
            userCard.masterRank = item.value("masterRank", 0);
            userCard.specialTrainingStatus = mapEnum(EnumMap::specialTrainingStatus, item.value("specialTrainingStatus", ""));
            userCard.episodes = UserCardEpisodes::fromJsonList(item.value("episodes", json::array()));
            userCard.defaultImage = mapEnum(EnumMap::defaultImage, item.value("defaultImage", ""));
            userCards.push_back(userCard);
        }
        return userCards;
    }
};

#endif  // USER_CARD_H



