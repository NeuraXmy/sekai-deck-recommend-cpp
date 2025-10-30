#ifndef MYSEKAI_DECK_RECOMMEND_H
#define MYSEKAI_DECK_RECOMMEND_H

#include "deck-recommend/base-deck-recommend.h"
#include "mysekai-information/mysekai-event-calculator.h"
#include "event-point/event-service.h"


class MysekaiDeckRecommend {
    DataProvider dataProvider;
    BaseDeckRecommend baseRecommend;
    MysekaiEventCalculator mysekaiEventCalculator;
    EventService eventService;

public:

    MysekaiDeckRecommend(DataProvider dataProvider)
        : dataProvider(dataProvider),
          baseRecommend(dataProvider),
          mysekaiEventCalculator(dataProvider),
          eventService(dataProvider) {}

    /**
     * 推荐烤森获取活动PT用的卡牌
     * 根据活动PT高低推荐
     * @param eventId 活动ID
     * @param config 推荐设置
     * @param specialCharacterId 指定的角色（用于世界开花活动支援卡组）
     */
    std::vector<RecommendDeck> recommendMysekaiDeck(
        int eventId,
        const DeckRecommendConfig& config,
        int specialCharacterId = 0
    );

};


#endif // MYSEKAI_DECK_RECOMMEND_H