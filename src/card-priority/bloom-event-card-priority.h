#ifndef BLOOM_EVENT_CARD_PRIORITY_H
#define BLOOM_EVENT_CARD_PRIORITY_H

#include "card-priority/card-priority-filter.h"
#include "common/collection-utils.h"

inline std::vector<CardPriority> bloomCardPriorities = {
    CardPriority{
        .eventBonus = 25 + 25, // 同队 5破四星
        .cardRarityType = Enums::CardRarityType::rarity_4,
        .masterRank = 5,
        .priority = 0
    },
    CardPriority{
        .eventBonus = 25 + 10, // 同队 0破四星
        .cardRarityType = Enums::CardRarityType::rarity_4,
        .masterRank = 0,
        .priority = 10
    },
    CardPriority{
        .eventBonus = 25 + 15, // 同队 5破生日
        .cardRarityType = Enums::CardRarityType::rarity_birthday,
        .masterRank = 5,
        .priority = 10
    },
    CardPriority{
        .eventBonus = 25 + 5, // 同队 0破生日
        .cardRarityType = Enums::CardRarityType::rarity_birthday,
        .masterRank = 0,
        .priority = 20
    },
    CardPriority{
        .eventBonus = 25 + 5, // 同队 5破三星
        .cardRarityType = Enums::CardRarityType::rarity_3,
        .masterRank = 0,
        .priority = 20
    },
    CardPriority{
        .eventBonus = 25, // 异队 5破四星
        .cardRarityType = Enums::CardRarityType::rarity_4,
        .masterRank = 5,
        .priority = 21
    },
    CardPriority{
        .eventBonus = 10, // 异队 0破四星
        .cardRarityType = Enums::CardRarityType::rarity_4,
        .masterRank = 0,
        .priority = 22
    },
    CardPriority{
        .eventBonus = 25,
        .cardRarityType = Enums::CardRarityType::rarity_3,
        .masterRank = 0,
        .priority = 30
    },
    CardPriority{
        .eventBonus = 25,
        .cardRarityType = Enums::CardRarityType::rarity_2,
        .masterRank = 0,
        .priority = 40
    },
    CardPriority{
        .eventBonus = 25,
        .cardRarityType = Enums::CardRarityType::rarity_1,
        .masterRank = 0,
        .priority = 50
    },
    CardPriority{
        .eventBonus = 5,
        .cardRarityType = Enums::CardRarityType::rarity_birthday,
        .masterRank = 0,
        .priority = 70
    },
    CardPriority{
        .eventBonus = 0,
        .cardRarityType = Enums::CardRarityType::rarity_3,
        .masterRank = 0,
        .priority = 80
    },
    CardPriority{
        .eventBonus = 0,
        .cardRarityType = Enums::CardRarityType::rarity_2,
        .masterRank = 0,
        .priority = 90
    },
    CardPriority{
        .eventBonus = 0,
        .cardRarityType = Enums::CardRarityType::rarity_1,
        .masterRank = 0,
        .priority = 100
    }
};

#endif