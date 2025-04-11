#include "deck-recommend/event-deck-recommend.h"

std::vector<RecommendDeck> EventDeckRecommend::recommendEventDeck(int eventId, int liveType, const DeckRecommendConfig &config, int specialCharacterId)
{
    auto eventConfig = eventService.getEventConfig(eventId, specialCharacterId);
    if (!eventConfig.eventType) {
        throw std::runtime_error("Event type not found for " + std::to_string(eventId));
    }

    auto userCards = dataProvider.userData->userCards;
    return baseRecommend.recommendHighScoreDeck(userCards,
        this->eventCalculator.getEventPointFunction(liveType, eventConfig.eventType), 
        config, 
        liveType, 
        eventConfig
    );
}