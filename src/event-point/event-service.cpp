#include "event-point/event-service.h"
#include <set>

int EventService::getEventType(int eventId)
{
    auto& events = this->dataProvider.masterData->events;
    auto& event = findOrThrow(events, [eventId](const Event& it) { 
        return it.id == eventId; 
    }, [&]() { return "Event not found for eventId=" + std::to_string(eventId); });
    return event.eventType;
}

EventConfig EventService::getEventConfig(int eventId, int specialCharacterId)
{
   return {
        eventId,
        this->getEventType(eventId),
        this->getEventBonusUnit(eventId),
        specialCharacterId
    };
}

int EventService::getEventBonusUnit(int eventId)
{
    auto& eventDeckBonuses = this->dataProvider.masterData->eventDeckBonuses;
    auto& gameCharacterUnits = this->dataProvider.masterData->gameCharacterUnits;
    std::unordered_set<int> s{};
    for (const auto& it : eventDeckBonuses) {
        if (it.eventId == eventId && it.gameCharacterUnitId != 0) {
            auto unit = findOrThrow(gameCharacterUnits, [it](const GameCharacterUnit& a) { 
                return a.id == it.gameCharacterUnitId; 
            }, [&]() { return "Game character unit not found for gameCharacterUnitId=" + std::to_string(it.gameCharacterUnitId); });
            s.insert(unit.unit);
        }
    }
    if (s.size() != 1) return 0;
    return *s.begin();
}

