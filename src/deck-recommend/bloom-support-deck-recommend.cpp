#include "deck-recommend/bloom-support-deck-recommend.h"

std::vector<CardDetail> BloomSupportDeckRecommend::recommendBloomSupportDeck(
    const std::vector<CardDetail> &mainDeck, 
    int specialCharacterId,
    int supportDeckCount
)
{
    auto userCards = dataProvider.userData->userCards;
    auto allCards = cardCalculator.batchGetCardDetail(userCards, {}, EventConfig{ .specialCharacterId = specialCharacterId });

    std::vector<const CardDetail*> pMainDeck{};
    for (const auto& card : mainDeck) 
        pMainDeck.push_back(&card);
    return deckCalculator.getSupportDeckBonus(pMainDeck, allCards, supportDeckCount).cards;
}