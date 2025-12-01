#ifndef EVENT_H
#define EVENT_H

#include "common/collection-utils.h"

struct Event {
    int id = 0;
    int eventType = 0;
    TS startAt = 0;
    TS aggregateAt = 0;
    TS rankingAnnounceAt = 0;
    TS distributionStartAt = 0;
    TS closedAt = 0;
    TS distributionEndAt = 0;

    inline static std::vector<Event> fromJsonList(const json& jsonData) {
        std::vector<Event> events;
        for (const auto& item : jsonData) {
            Event event;
            event.id = item.value("id", 0);
            event.eventType = mapEnum(EnumMap::eventType, item.value("eventType", ""));
            event.startAt = item.value("startAt", TS());
            event.aggregateAt = item.value("aggregateAt", TS());
            event.rankingAnnounceAt = item.value("rankingAnnounceAt", TS());
            event.distributionStartAt = item.value("distributionStartAt", TS());
            event.closedAt = item.value("closedAt", TS());
            event.distributionEndAt = item.value("distributionEndAt", TS());
            events.push_back(event);
        }
        return events;
    }
};

#endif // EVENT_H