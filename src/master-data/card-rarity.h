#ifndef CARD_RARITY_H
#define CARD_RARITY_H

#include "common/collection-utils.h"

struct CardRarity {
    int cardRarityType = 0;
    int seq = 0;
    int maxLevel = 0;
    int trainingMaxLevel = 0;
    int maxSkillLevel = 0;

    static inline std::vector<CardRarity> fromJsonList(const json& jsonData) {
        std::vector<CardRarity> cardRarities;
        for (const auto& item : jsonData) {
            CardRarity cardRarity;
            cardRarity.cardRarityType = mapEnum(EnumMap::cardRarityType, item.value("cardRarityType", ""));
            cardRarity.seq = item.value("seq", 0);
            cardRarity.maxLevel = item.value("maxLevel", 0);
            cardRarity.trainingMaxLevel = item.value("trainingMaxLevel", 0);
            cardRarity.maxSkillLevel = item.value("maxSkillLevel", 0);
            cardRarities.push_back(cardRarity);
        }
        return cardRarities;
    }
};

#endif

