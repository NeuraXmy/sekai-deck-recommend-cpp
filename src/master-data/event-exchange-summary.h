#ifndef EVENT_EXCHANGE_SUMMARY_H
#define EVENT_EXCHANGE_SUMMARY_H

#include "common/collection-utils.h"

struct EventExchange {
    int id = 0;
    int resourceBoxId = 0;
    int exchangeLimit = 0;

    inline static std::vector<EventExchange> fromJsonList(const json& jsonData) {
        std::vector<EventExchange> eventExchanges;
        for (const auto& item : jsonData) {
            EventExchange eventExchange;
            eventExchange.id = item.value("id", 0);
            eventExchange.resourceBoxId = item.value("resourceBoxId", 0);
            eventExchange.exchangeLimit = item.value("exchangeLimit", 0);
            eventExchanges.push_back(eventExchange);
        }
        return eventExchanges;
    }
};

struct EventExchangeSummary {
    int id = 0;
    int eventId = 0;
    TS startAt = 0;
    TS endAt = 0;
    std::vector<EventExchange> eventExchanges;
    
    inline static std::vector<EventExchangeSummary> fromJsonList(const json& jsonData) {
        std::vector<EventExchangeSummary> eventExchangeSummaries;
        for (const auto& item : jsonData) {
            EventExchangeSummary eventExchangeSummary;
            eventExchangeSummary.id = item.value("id", 0);
            eventExchangeSummary.eventId = item.value("eventId", 0);
            eventExchangeSummary.startAt = item.value("startAt", 0);
            eventExchangeSummary.endAt = item.value("endAt", 0);
            eventExchangeSummary.eventExchanges = EventExchange::fromJsonList(item["eventExchanges"]);
            eventExchangeSummaries.push_back(eventExchangeSummary);
        }
        return eventExchangeSummaries;
    }
};

#endif // EVENT_EXCHANGE_SUMMARY_H