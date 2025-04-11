#ifndef EVENT_DECK_RECOMMEND_H
#define EVENT_DECK_RECOMMEND_H

#include "deck-recommend/base-deck-recommend.h"
#include "event-point/event-service.h"
#include "event-point/event-calculator.h"

class EventDeckRecommend {
    DataProvider dataProvider;
    BaseDeckRecommend baseRecommend;
    EventService eventService;
    EventCalculator eventCalculator;

public:

    EventDeckRecommend(DataProvider dataProvider)
        : dataProvider(dataProvider),
          baseRecommend(dataProvider),
          eventService(dataProvider), 
          eventCalculator(dataProvider) {}

    /**
     * 推荐活动用的卡牌
     * 根据活动PT高低推荐
     * @param eventId 活动ID
     * @param liveType Live类型
     * @param config 推荐设置
     * @param specialCharacterId 指定的角色（用于世界开花活动支援卡组）
     */
    std::vector<RecommendDeck> recommendEventDeck(
        int eventId,
        int liveType,
        const DeckRecommendConfig& config,
        int specialCharacterId = 0
    );

};


#endif // EVENT_DECK_RECOMMEND_H