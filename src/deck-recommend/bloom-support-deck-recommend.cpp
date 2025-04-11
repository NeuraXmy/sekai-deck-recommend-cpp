#include "deck-recommend/bloom-support-deck-recommend.h"

std::vector<CardDetail> BloomSupportDeckRecommend::recommendBloomSupportDeck(
    const std::vector<CardDetail> &mainDeck, 
    int specialCharacterId,
    int supportDeckCount
)
{
    auto userCards = dataProvider.userData->userCards;
    auto allCards = cardCalculator.batchGetCardDetail(userCards, {}, EventConfig{ .specialCharacterId = specialCharacterId });
    return deckCalculator.getSupportDeckBonus(mainDeck, allCards, supportDeckCount).cards;
}