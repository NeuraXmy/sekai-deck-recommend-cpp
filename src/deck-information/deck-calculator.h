#ifndef DECK_CALCULATOR_H
#define DECK_CALCULATOR_H

#include "data-provider/data-provider.h"
#include "deck-information/deck-service.h"
#include "event-point/event-service.h"
#include "card-information/card-calculator.h"


struct SupportDeckBonus {
    double bonus;
    std::vector<CardDetail> cards;
};


class DeckCalculator {
    DataProvider dataProvider;
    CardCalculator cardCalculator;

public:

    DeckCalculator(DataProvider dataProvider) 
        : dataProvider(dataProvider), 
          cardCalculator(CardCalculator(dataProvider)) {}

    /**
     * 这个函数原本在 EventCalculator 中，为防止循环引用移动到这里
     * 获取卡组活动加成
     * @param deckCards 卡组
     * @param eventType （可选）活动类型
     */
    std::optional<double> getDeckBonus(const std::vector<CardDetail>& deckCards, std::optional<int> eventType = std::nullopt);

    /**
     * 这个函数原本在 EventCalculator 中，为防止循环引用移动到这里
     * 获取支援卡组加成
     * @param deckCards 卡组
     * @param allCards 所有卡牌（按支援卡组加成从大到小排序）
     * @param supportDeckCount 支援卡组数量
     */
    SupportDeckBonus getSupportDeckBonus(const std::vector<CardDetail>& deckCards, const std::vector<CardDetail>& allCards, int supportDeckCount);


    /**
     * 获取称号的综合力加成（与卡牌无关、根据称号累加）
     */
    int getHonorBonusPower();

    /**
     * 计算给定的多张卡牌综合力、技能
     * @param cardDetails 处理好的卡牌详情（数组长度1-5，兼容挑战Live）
     * @param allCards 参与计算的所有卡，按支援队伍加成从大到小排序
     * @param honorBonus 称号加成
     * @param eventType 活动类型（用于算加成）
     */
    DeckDetail getDeckDetailByCards(
        const std::vector<CardDetail>& cardDetails,
        const std::vector<CardDetail>& allCards,
        int honorBonus = 0,
        std::optional<int> eventType = std::nullopt,
        std::optional<int> eventId = std::nullopt
    );

    /**
     * 根据用户卡组获得卡组详情
     * @param deckCards 用户卡组中的用户卡牌
     * @param allCards 用户全部卡牌
     * @param eventConfig （可选）活动设置
     * @param areaItemLevels （可选）使用的区域道具
     */
    DeckDetail getDeckDetail(
        const std::vector<UserCard>& deckCards,
        const std::vector<UserCard>& allCards,
        std::optional<EventConfig> eventConfig = std::nullopt,
        std::vector<AreaItemLevel> areaItemLevels = {}
    );
};
   
#endif