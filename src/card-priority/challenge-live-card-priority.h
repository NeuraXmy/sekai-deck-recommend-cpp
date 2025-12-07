#ifndef CHALLENGE_LIVE_CARD_PRIORITY_H
#define CHALLENGE_LIVE_CARD_PRIORITY_H

#include "card-priority/card-priority-filter.h"
#include "common/collection-utils.h"

inline std::vector<CardPriority> challengeLiveCardPriorities = {
    CardPriority{
        .eventBonus = 0,
        .cardRarityType = Enums::CardRarityType::rarity_4,
        .masterRank = 0,
        .priority = 0
    },
    CardPriority{
        .eventBonus = 0,
        .cardRarityType = Enums::CardRarityType::rarity_birthday,
        .masterRank = 0,
        .priority = 10
    },
    CardPriority{
        .eventBonus = 0,
        .cardRarityType = Enums::CardRarityType::rarity_3,
        .masterRank = 0,
        .priority = 20
    },
    CardPriority{
        .eventBonus = 0,
        .cardRarityType = Enums::CardRarityType::rarity_2,
        .masterRank = 0,
        .priority = 30
    },
    CardPriority{
        .eventBonus = 0,
        .cardRarityType = Enums::CardRarityType::rarity_1,
        .masterRank = 0,
        .priority = 40
    }
};


#endif