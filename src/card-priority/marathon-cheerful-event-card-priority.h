#ifndef MARATHON_CHEERFUL_EVENT_CARD_PRIORITY_H
#define MARATHON_CHEERFUL_EVENT_CARD_PRIORITY_H

#include "card-priority/card-priority-filter.h"
#include "common/json-utils.h"

inline std::vector<CardPriority> marathonCheerfulCardPriorities = {
    CardPriority{
        .eventBonus = 25 + 25 + 20 + 25, // 同色同队 当期卡 5破四星
        .cardRarityType = rarity_4_enum,
        .masterRank = 5,
        .priority = 0
    },
    CardPriority{
        .eventBonus = 25 + 25 + 20 + 10, // 同色同队 当期卡 0破四星
        .cardRarityType = rarity_4_enum,
        .masterRank = 0,
        .priority = 10
    },
    CardPriority{
        .eventBonus = 25 + 25 + 25, // 同色同队 5破四星
        .cardRarityType = rarity_4_enum,
        .masterRank = 5,
        .priority = 10
    },
    CardPriority{
        .eventBonus = 25 + 15 + 25, // 同色同队（V家） 5破四星
        .cardRarityType = rarity_4_enum,
        .masterRank = 5,
        .priority = 30
    },
    CardPriority{
        .eventBonus = 25 + 25 + 10, // 同色同队 0破四星
        .cardRarityType = rarity_4_enum,
        .masterRank = 0,
        .priority = 40
    },
    CardPriority{
        .eventBonus = 25 + 25, // 同色或同队 5破四星
        .cardRarityType = rarity_4_enum,
        .masterRank = 5,
        .priority = 40
    },
    CardPriority{
        .eventBonus = 25 + 25 + 15,
        .cardRarityType = rarity_birthday_enum,
        .masterRank = 5,
        .priority = 40
    },
    CardPriority{
        .eventBonus = 25 + 15 + 10, // 同色同队（V家） 0破四星
        .cardRarityType = rarity_4_enum,
        .masterRank = 0,
        .priority = 50
    },
    CardPriority{
        .eventBonus = 25 + 25 + 5,
        .cardRarityType = rarity_birthday_enum,
        .masterRank = 0,
        .priority = 50
    },
    CardPriority{
        .eventBonus = 25 + 25 + 5,
        .cardRarityType = rarity_3_enum,
        .masterRank = 5,
        .priority = 50
    },
    CardPriority{
        .eventBonus = 25 + 10, // 同色或同队 0破四星
        .cardRarityType = rarity_4_enum,
        .masterRank = 0,
        .priority = 60
    },
    CardPriority{
        .eventBonus = 25 + 15,
        .cardRarityType = rarity_birthday_enum,
        .masterRank = 5,
        .priority = 60
    },
    CardPriority{
        .eventBonus = 25 + 25,
        .cardRarityType = rarity_3_enum,
        .masterRank = 0,
        .priority = 60
    },
    CardPriority{
        .eventBonus = 25,
        .cardRarityType = rarity_4_enum,
        .masterRank = 5,
        .priority = 60
    },
    CardPriority{
        .eventBonus = 15 + 10,
        .cardRarityType = rarity_4_enum,
        .masterRank = 0,
        .priority = 70
    },
    CardPriority{
        .eventBonus = 25 + 5,
        .cardRarityType = rarity_birthday_enum,
        .masterRank = 0,
        .priority = 70
    },
    CardPriority{
        .eventBonus = 25 + 5,
        .cardRarityType = rarity_3_enum,
        .masterRank = 5,
        .priority = 70
    },
    CardPriority{
        .eventBonus = 25 + 25,
        .cardRarityType = rarity_2_enum,
        .masterRank = 0,
        .priority = 70
    },
    CardPriority{
        .eventBonus = 25 + 25,
        .cardRarityType = rarity_1_enum,
        .masterRank = 0,
        .priority = 70
    },
    CardPriority{
        .eventBonus = 15 + 5,
        .cardRarityType = rarity_birthday_enum,
        .masterRank = 0,
        .priority = 80
    },
    CardPriority{
        .eventBonus = 25,
        .cardRarityType = rarity_3_enum,
        .masterRank = 0,
        .priority = 80
    },
    CardPriority{
        .eventBonus = 25,
        .cardRarityType = rarity_2_enum,
        .masterRank = 0,
        .priority = 80
    },
    CardPriority{
        .eventBonus = 25,
        .cardRarityType = rarity_1_enum,
        .masterRank = 0,
        .priority = 80
    },
    CardPriority{
        .eventBonus = 10,
        .cardRarityType = rarity_4_enum,
        .masterRank = 0,
        .priority = 80
    },
    CardPriority{
        .eventBonus = 5,
        .cardRarityType = rarity_birthday_enum,
        .masterRank = 0,
        .priority = 90
    },
    CardPriority{
        .eventBonus = 0,
        .cardRarityType = rarity_3_enum,
        .masterRank = 0,
        .priority = 100
    },
    CardPriority{
        .eventBonus = 0,
        .cardRarityType = rarity_2_enum,
        .masterRank = 0,
        .priority = 100
    },
    CardPriority{
        .eventBonus = 0,
        .cardRarityType = rarity_1_enum,
        .masterRank = 0,
        .priority = 100
    }
};

#endif