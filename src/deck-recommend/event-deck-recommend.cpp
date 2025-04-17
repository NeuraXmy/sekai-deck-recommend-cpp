#include "deck-recommend/event-deck-recommend.h"

static int event_type_cheerful = mapEnum(EnumMap::eventType, "cheerful_carnival");
static int live_type_cheerful = mapEnum(EnumMap::liveType, "cheerful");
static int live_type_multi = mapEnum(EnumMap::liveType, "multi");

std::vector<RecommendDeck> EventDeckRecommend::recommendEventDeck(int eventId, int liveType, const DeckRecommendConfig &config, int specialCharacterId)
{
    auto eventConfig = eventService.getEventConfig(eventId, specialCharacterId);
    if (!eventConfig.eventType) {
        throw std::runtime_error("Event type not found for " + std::to_string(eventId));
    }

    if (eventConfig.eventType == event_type_cheerful && liveType == live_type_multi) {
        liveType = live_type_cheerful;
    }

    auto userCards = dataProvider.userData->userCards;
    return baseRecommend.recommendHighScoreDeck(userCards,
        this->eventCalculator.getEventPointFunction(liveType, eventConfig.eventType), 
        config, 
        liveType, 
        eventConfig
    );
}