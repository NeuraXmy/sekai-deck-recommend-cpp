#ifndef WORLD_BLOOM_SUPPORT_DECK_UNIT_EVENT_LIMITED_BONUS_H
#define WORLD_BLOOM_SUPPORT_DECK_UNIT_EVENT_LIMITED_BONUS_H

#include "common/collection-utils.h"

struct WorldBloomSupportDeckUnitEventLimitedBonus {
    int id = 0;
    int eventId = 0;
    int gameCharacterId = 0;
    int cardId = 0;
    double bonusRate = 0.0;

    static inline std::vector<WorldBloomSupportDeckUnitEventLimitedBonus> fromJsonList(const json& jsonData) {
        std::vector<WorldBloomSupportDeckUnitEventLimitedBonus> worldBloomSupportDeckUnitEventLimitedBonuses;
        for (const auto& item : jsonData) {
            WorldBloomSupportDeckUnitEventLimitedBonus worldBloomSupportDeckUnitEventLimitedBonus;
            worldBloomSupportDeckUnitEventLimitedBonus.id = item.value("id", 0);
            worldBloomSupportDeckUnitEventLimitedBonus.eventId = item.value("eventId", 0);
            worldBloomSupportDeckUnitEventLimitedBonus.gameCharacterId = item.value("gameCharacterId", 0);
            worldBloomSupportDeckUnitEventLimitedBonus.cardId = item.value("cardId", 0);
            worldBloomSupportDeckUnitEventLimitedBonus.bonusRate = item.value("bonusRate", 0.0);
            worldBloomSupportDeckUnitEventLimitedBonuses.push_back(worldBloomSupportDeckUnitEventLimitedBonus);
        }
        return worldBloomSupportDeckUnitEventLimitedBonuses;
    }
};

#endif // WORLD_BLOOM_SUPPORT_DECK_UNIT_EVENT_LIMITED_BONUS_H