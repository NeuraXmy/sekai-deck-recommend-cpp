#ifndef BLOOM_SUPPORT_DECK_RECOMMEND_H
#define BLOOM_SUPPORT_DECK_RECOMMEND_H

#include "card-information/card-calculator.h"
#include "deck-information/deck-calculator.h"

class BloomSupportDeckRecommend {

    DataProvider dataProvider;
    CardCalculator cardCalculator;
    DeckCalculator deckCalculator;

public:

    BloomSupportDeckRecommend(DataProvider dataProvider)
        : dataProvider(dataProvider),
          cardCalculator(dataProvider),
          deckCalculator(dataProvider) {}

    /**
     * 推荐支援卡组
     * @param mainDeck 主要卡组
     * @param specialCharacterId 支援角色
     */
    std::vector<CardDetail> recommendBloomSupportDeck(
        const std::vector<CardDetail>& mainDeck,
        int specialCharacterId,
        int supportDeckCount
    );

};

#endif

