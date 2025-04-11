#ifndef CARD_EVENT_CALCULATOR_H
#define CARD_EVENT_CALCULATOR_H

#include "data-provider/data-provider.h"

class CardEventCalculator {

    DataProvider dataProvider;  

public:

    CardEventCalculator(const DataProvider& dataProvider) : dataProvider(dataProvider) {}

    /**
     * 获取卡牌的角色、属性加成
     * @param eventId 活动ID
     * @param card 卡牌
     */
    double getEventDeckBonus(int eventId, const Card& card);

    /**
     * 获取单一卡牌的活动加成（含角色、属性、当期、突破加成）
     * 返回值用到的时候还得/100
     * @param userCard 用户卡牌
     * @param eventId 活动ID
     */
    double getCardEventBonus(const UserCard& userCard, int eventId);

};


#endif // CARD_EVENT_CALCULATOR_H