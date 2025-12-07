#include "deck-recommend/event-deck-recommend.h"

std::vector<RecommendDeck> EventDeckRecommend::recommendEventDeck(int eventId, int liveType, const DeckRecommendConfig &config, int specialCharacterId)
{
    auto eventConfig = eventService.getEventConfig(eventId, specialCharacterId);
    if (!eventConfig.eventType) {
        throw std::runtime_error("Event type not found for " + std::to_string(eventId));
    }

    // 5v5 外部参数传入统一liveType为multi 内部计算时改为cheerful
    if (eventConfig.eventType == Enums::EventType::cheerful && liveType == Enums::LiveType::multi_live) {
        liveType = Enums::LiveType::cheerful_live;
    }

    auto userCards = dataProvider.userData->userCards;
    return baseRecommend.recommendHighScoreDeck(userCards,
        this->eventCalculator.getEventPointFunction(
            liveType, 
            eventConfig.eventType,
            config.liveSkillOrder,
            config.specificSkillOrder,
            config.multiTeammateScoreUp,
            config.multiTeammatePower
        ), 
        config, 
        liveType, 
        eventConfig
    );
}