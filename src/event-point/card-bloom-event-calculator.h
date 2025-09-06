#ifndef CARD_BLOOM_EVENT_CALCULATOR_H
#define CARD_BLOOM_EVENT_CALCULATOR_H

#include "data-provider/data-provider.h"
#include "card-information/card-service.h"
#include <optional>


class CardBloomEventCalculator {

    DataProvider dataProvider;
    CardService cardService;

public:

    CardBloomEventCalculator(const DataProvider& dataProvider) : 
        dataProvider(dataProvider),
        cardService(dataProvider) {}

    /**
     * 获取单张卡牌的支援加成
     * 需要注意的是，支援卡组只能上对应团队的卡，其它卡上不了
     * 未指定支援角色时返回值为nullopt
     * @param userCard 用户卡牌
     * @param eventId 活动ID
     * @param specialCharacterId 指定的加成角色
     */
    std::optional<double> getCardSupportDeckBonus(const UserCard& userCard, int eventId, int specialCharacterId);

};

#endif // CARD_BLOOM_EVENT_CALCULATOR_H