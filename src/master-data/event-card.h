#ifndef EVENT_CARD_H
#define EVENT_CARD_H

#include "common/collection-utils.h"

struct EventCard {
    int id = 0;
    int cardId = 0;
    int eventId = 0;
    double bonusRate = 0.0;

    static inline std::vector<EventCard> fromJsonList(const json& jsonData) {
        std::vector<EventCard> eventCards;
        for (const auto& item : jsonData) {
            EventCard eventCard;
            eventCard.id = item.value("id", 0);
            eventCard.cardId = item.value("cardId", 0);
            eventCard.eventId = item.value("eventId", 0);
            eventCard.bonusRate = item.value("bonusRate", 0.0);
            eventCards.push_back(eventCard);
        }
        return eventCards;
    }
};


#endif // EVENT_CARD_H