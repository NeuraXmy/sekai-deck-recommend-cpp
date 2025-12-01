#ifndef EVENT_DECK_BONUS_H
#define EVENT_DECK_BONUS_H

#include "common/collection-utils.h"

struct EventDeckBonus {
    int id = 0;
    int eventId = 0;
    int gameCharacterUnitId = 0;
    int cardAttr = 0;
    double bonusRate = 0.0;

    static inline std::vector<EventDeckBonus> fromJsonList(const json& jsonData) {
        std::vector<EventDeckBonus> eventDeckBonuses;
        for (const auto& item : jsonData) {
            EventDeckBonus eventDeckBonus;
            eventDeckBonus.id = item.value("id", 0);
            eventDeckBonus.eventId = item.value("eventId", 0);
            eventDeckBonus.gameCharacterUnitId = item.value("gameCharacterUnitId", 0);
            eventDeckBonus.cardAttr = mapEnum(EnumMap::attr, item.value("cardAttr", ""));
            eventDeckBonus.bonusRate = item.value("bonusRate", 0.0);
            eventDeckBonuses.push_back(eventDeckBonus);
        }
        return eventDeckBonuses;
    }
};


#endif // EVENT_DECK_BONUS_H