#ifndef EVENT_RARITY_BONUS_RATE_H
#define EVENT_RARITY_BONUS_RATE_H

#include "common/collection-utils.h"

struct EventRarityBonusRate {
    int id = 0;
    int cardRarityType = 0;
    int masterRank = 0;
    double bonusRate = 0.0;

    static inline std::vector<EventRarityBonusRate> fromJsonList(const json& jsonData) {
        std::vector<EventRarityBonusRate> eventRarityBonusRates;
        for (const auto& item : jsonData) {
            EventRarityBonusRate eventRarityBonusRate;
            eventRarityBonusRate.id = item.value("id", 0);
            eventRarityBonusRate.cardRarityType = mapEnum(EnumMap::cardRarityType, item.value("cardRarityType", ""));
            eventRarityBonusRate.masterRank = item.value("masterRank", 0);
            eventRarityBonusRate.bonusRate = item.value("bonusRate", 0.0);
            eventRarityBonusRates.push_back(eventRarityBonusRate);
        }
        return eventRarityBonusRates;
    }
};

#endif // EVENT_RARITY_BONUS_RATE_H