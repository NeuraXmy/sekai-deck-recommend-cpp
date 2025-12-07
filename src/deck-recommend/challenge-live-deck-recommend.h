#ifndef CHALLENGE_LIVE_DECK_RECOMMEND_H
#define CHALLENGE_LIVE_DECK_RECOMMEND_H

#include "base-deck-recommend.h"

class ChallengeLiveDeckRecommend {

    DataProvider dataProvider;
    BaseDeckRecommend baseRecommend;
    LiveCalculator liveCalculator;

public:

    ChallengeLiveDeckRecommend(DataProvider dataProvider) : 
        dataProvider(dataProvider), 
        baseRecommend(dataProvider),
        liveCalculator(dataProvider) {}

    /**
     * 推荐挑战Live用的卡牌
     * 根据Live分数高低推荐
     * @param characterId 角色ID
     * @param config 推荐设置
     */
    std::vector<RecommendDeck> recommendChallengeLiveDeck(
        int liveType,
        int characterId, 
        const DeckRecommendConfig& config
    );

};

#endif