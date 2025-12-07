#include "deck-recommend/challenge-live-deck-recommend.h"

std::vector<RecommendDeck> ChallengeLiveDeckRecommend::recommendChallengeLiveDeck(int liveType, int characterId, const DeckRecommendConfig &config)
{
    if (!Enums::LiveType::isChallenge(liveType))
        throw std::runtime_error("Invalid live type for challenge live deck recommend: " + std::to_string(liveType));

    auto& userCards = this->dataProvider.userData->userCards;
    auto& cards = this->dataProvider.masterData->cards;
    std::vector<UserCard> characterCards{};
    for (const auto& userCard : userCards) {
        auto card = std::find_if(cards.begin(), cards.end(), [&userCard](const Card& card) {
            return card.id == userCard.cardId;
        });
        if (card != cards.end() && card->characterId == characterId) {
            characterCards.push_back(userCard);
        }
    }

    return this->baseRecommend.recommendHighScoreDeck(
        characterCards,
        liveCalculator.getLiveScoreFunction(
            liveType,
            config.liveSkillOrder,
            config.specificSkillOrder
        ),
        config,
        liveType
    );
}