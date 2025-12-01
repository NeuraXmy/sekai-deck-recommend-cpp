#include "deck-information/deck-service.h"

UserCard DeckService::getUserCard(int cardId)
{
    auto& userCards = this->dataProvider.userData->userCards;
    return findOrThrow(userCards, [cardId](const UserCard& it) { 
        return it.cardId == cardId; 
    }, [&]() { return "User card not found for cardId=" + std::to_string(cardId); });
}

UserDeck DeckService::getDeck(int deckId)
{
    auto& userDecks = this->dataProvider.userData->userDecks;
    return findOrThrow(userDecks, [deckId](const UserDeck& it) { 
        return it.deckId == deckId; 
    }, [&]() { return "User deck not found for deckId=" + std::to_string(deckId); });
}

std::vector<UserCard> DeckService::getDeckCards(const UserDeck &userDeck)
{
    std::vector<int> cardIds = {userDeck.member1, userDeck.member2, userDeck.member3, userDeck.member4, userDeck.member5};
    std::vector<UserCard> userCards;
    for (const auto& cardId : cardIds) {
        auto userCard = this->getUserCard(cardId);
        userCards.push_back(userCard);
    }
    return userCards;
}

UserDeck DeckService::toUserDeck(const std::vector<DeckCardDetail> &userCards, long long userId, int deckId)
{
    if (userCards.size() != 5) throw std::runtime_error("deck card should be 5");
    UserDeck userDeck{};
    // userDeck.name = name;
    userDeck.userId = userId;
    userDeck.deckId = deckId;
    userDeck.leader = userCards[0].cardId;
    userDeck.subLeader = userCards[1].cardId;
    userDeck.member1 = userCards[0].cardId;
    userDeck.member2 = userCards[1].cardId;
    userDeck.member3 = userCards[2].cardId;
    userDeck.member4 = userCards[3].cardId;
    userDeck.member5 = userCards[4].cardId;
    return userDeck;
}

UserChallengeLiveSoloDeck DeckService::getChallengeLiveSoloDeck(int characterId)
{
    auto& userChallengeLiveSoloDecks = this->dataProvider.userData->userChallengeLiveSoloDecks;
    return findOrThrow(userChallengeLiveSoloDecks, [characterId](const UserChallengeLiveSoloDeck& it) { 
        return it.characterId == characterId; 
    }, [&]() { return "User challenge live solo deck not found for characterId=" + std::to_string(characterId); });
}

std::vector<UserCard> DeckService::getChallengeLiveSoloDeckCards(const UserChallengeLiveSoloDeck &deck)
{
    std::vector<int> cardIds = {deck.leader, deck.support1, deck.support2, deck.support3, deck.support4};
    std::vector<UserCard> userCards{};
    for (const auto& cardId : cardIds) {
        if (cardId == 0) continue;
        auto userCard = this->getUserCard(cardId);
        userCards.push_back(userCard);
    }
    return userCards;
}

UserChallengeLiveSoloDeck DeckService::toUserChallengeLiveSoloDeck(const std::vector<DeckCardDetail> &userCards, int characterId)
{
    if (userCards.size() < 1) throw std::runtime_error("deck card should >= 1");
    if (userCards.size() > 5) throw std::runtime_error("deck card should <= 5");
    UserChallengeLiveSoloDeck userChallengeLiveSoloDeck{};
    userChallengeLiveSoloDeck.characterId = characterId;
    userChallengeLiveSoloDeck.leader = userCards[0].cardId;
    userChallengeLiveSoloDeck.support1 = userCards.size() < 2 ? 0 : userCards[1].cardId;
    userChallengeLiveSoloDeck.support2 = userCards.size() < 3 ? 0 : userCards[2].cardId;
    userChallengeLiveSoloDeck.support3 = userCards.size() < 4 ? 0 : userCards[3].cardId;
    userChallengeLiveSoloDeck.support4 = userCards.size() < 5 ? 0 : userCards[4].cardId;
    return userChallengeLiveSoloDeck;
}

UserWorldBloomSupportDeck DeckService::toUserWorldBloomSupportDeck(const std::vector<CardDetail> &userCards, int gameCharacterId)
{
    UserWorldBloomSupportDeck userWorldBloomSupportDeck{};
    userWorldBloomSupportDeck.gameCharacterId = gameCharacterId;
    userWorldBloomSupportDeck.member1 = userCards.size() < 1 ? 0 : userCards[0].cardId;
    userWorldBloomSupportDeck.member2 = userCards.size() < 2 ? 0 : userCards[1].cardId;
    userWorldBloomSupportDeck.member3 = userCards.size() < 3 ? 0 : userCards[2].cardId;
    userWorldBloomSupportDeck.member4 = userCards.size() < 4 ? 0 : userCards[3].cardId;
    userWorldBloomSupportDeck.member5 = userCards.size() < 5 ? 0 : userCards[4].cardId;
    userWorldBloomSupportDeck.member6 = userCards.size() < 6 ? 0 : userCards[5].cardId;
    userWorldBloomSupportDeck.member7 = userCards.size() < 7 ? 0 : userCards[6].cardId;
    userWorldBloomSupportDeck.member8 = userCards.size() < 8 ? 0 : userCards[7].cardId;
    userWorldBloomSupportDeck.member9 = userCards.size() < 9 ? 0 : userCards[8].cardId;
    userWorldBloomSupportDeck.member10 = userCards.size() < 10 ? 0 : userCards[9].cardId;
    userWorldBloomSupportDeck.member11 = userCards.size() < 11 ? 0 : userCards[10].cardId;
    userWorldBloomSupportDeck.member12 = userCards.size() < 12 ? 0 : userCards[11].cardId;
    return userWorldBloomSupportDeck;
}
