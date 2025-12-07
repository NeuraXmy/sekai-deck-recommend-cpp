#include "deck-recommend/mysekai-deck-recommend.h"

std::vector<RecommendDeck> MysekaiDeckRecommend::recommendMysekaiDeck(
    int eventId, 
    const DeckRecommendConfig &config, 
    int specialCharacterId
)
{
    auto eventConfig = eventService.getEventConfig(eventId, specialCharacterId);
    if (!eventConfig.eventType) {
        throw std::runtime_error("Event type not found for " + std::to_string(eventId));
    }

    auto cfg = config;
    cfg.target = RecommendTarget::Mysekai;
    cfg.keepAfterTrainingState = true;

    auto userCards = dataProvider.userData->userCards;
    return baseRecommend.recommendHighScoreDeck(userCards,
        this->mysekaiEventCalculator.getMysekaiEventPointFunction(), 
        cfg, 
        Enums::LiveType::multi_live,
        eventConfig
    );
}