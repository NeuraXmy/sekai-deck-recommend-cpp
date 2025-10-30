#ifndef MYSEKAI_EVENT_CALCULATOR_H
#define MYSEKAI_EVENT_CALCULATOR_H

#include "live-score/live-calculator.h"
#include "data-provider/data-provider.h"
#include "deck-information/deck-service.h"
#include <optional>


class MysekaiEventCalculator {

    DataProvider dataProvider;

public:

    MysekaiEventCalculator(const DataProvider& dataProvider) : 
        dataProvider(dataProvider) {}

    /**
     * 获得卡组的烤森活动点数
     * @param deckDetail 卡组
     */
    Score getDeckMysekaiEventPoint(const DeckDetail& deckDetail);

    /**
     * 获取计算烤森活动点数的函数
     * @param liveType Live类型
     * @param eventType 活动类型
     */
    ScoreFunction getMysekaiEventPointFunction();

};


#endif // MYSEKAI_EVENT_CALCULATOR_H