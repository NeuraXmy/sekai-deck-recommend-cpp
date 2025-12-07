#include "deck-recommend/challenge-live-deck-recommend.h"

std::vector<RecommendDeck> ChallengeLiveDeckRecommend::recommendChallengeLiveDeck(int characterId, const DeckRecommendConfig &config)
{
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
        liveCalculator.getLiveScoreFunction(Enums::LiveType::challenge_live),
        config,
        Enums::LiveType::challenge_live
    );
}