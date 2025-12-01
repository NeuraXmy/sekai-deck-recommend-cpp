#ifndef EVENT_ITEM_H
#define EVENT_ITEM_H

#include "common/collection-utils.h"

struct EventItem {
    int id = 0;
    int eventId = 0;
    int gameCharacterId = 0;

    static inline std::vector<EventItem> fromJsonList(const json& jsonData) {
        std::vector<EventItem> eventItems;
        for (const auto& item : jsonData) {
            EventItem eventItem;
            eventItem.id = item.value("id", 0);
            eventItem.eventId = item.value("eventId", 0);
            eventItem.gameCharacterId = item.value("gameCharacterId", 0);
            eventItems.push_back(eventItem);
        }
        return eventItems;
    }
};

#endif // EVENT_ITEM_H