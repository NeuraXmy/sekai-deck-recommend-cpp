#ifndef CARD_CALCULATOR_H
#define CARD_CALCULATOR_H

#include "data-provider/data-provider.h"
#include "card-information/card-service.h"
#include "card-information/card-detail-map.h"
#include "card-information/card-power-calculator.h"
#include "card-information/card-skill-calculator.h"
#include "card-information/card-service.h"
#include "area-item-information/area-item-service.h"
#include "mysekai-information/mysekai-service.h"
#include "event-point/card-event-calculator.h"
#include "event-point/card-bloom-event-calculator.h"
#include "event-point/event-service.h"

/**
 * 计算过程中使用的卡牌详情信息
 */
struct CardDetail {
    int cardId;
    int level;
    int skillLevel;
    int masterRank;
    int cardRarityType;
    int characterId;
    std::vector<int> units;
    int attr;
    CardDetailMap<DeckCardPowerDetail> power;
    CardDetailMap<DeckCardSkillDetail> skill;
    std::optional<double> eventBonus;
    std::optional<double> supportDeckBonus;
    bool hasCanvasBonus;
    bool episode1Read;
    bool episode2Read;
    bool afterTraining;
    int defaultImage;
};

class CardCalculator {

    DataProvider dataProvider;
    CardPowerCalculator powerCalculator;
    CardSkillCalculator skillCalculator;
    CardEventCalculator eventCalculator;
    CardBloomEventCalculator bloomEventCalculator;
    AreaItemService areaItemService;
    CardService cardService;
    MySekaiService mysekaiService;

public:

    CardCalculator(DataProvider dataProvider) : 
        dataProvider(dataProvider),
        powerCalculator(dataProvider),
        skillCalculator(dataProvider),
        eventCalculator(dataProvider),
        bloomEventCalculator(dataProvider),
        areaItemService(dataProvider),
        cardService(dataProvider),
        mysekaiService(dataProvider) {}

    /**
     * 获取卡牌详细数据
     * @param userCard 用户卡牌
     * @param userAreaItemLevels 用户拥有的区域道具等级
     * @param config 卡牌设置
     * @param eventConfig 活动设置
     * @param hasCanvasBonus 是否拥有自定义世界中的画布
     * @param userGateBonuses 用户拥有的大门加成
     */
    std::optional<CardDetail> getCardDetail(
        const UserCard& userCard,
        const std::vector<AreaItemLevel>& userAreaItemLevels,
        const std::unordered_map<int, CardConfig>& config,
        const std::optional<EventConfig>& eventConfig = std::nullopt,
        bool hasCanvasBonus = false,
        const std::vector<MysekaiGateBonus>& userGateBonuses = std::vector<MysekaiGateBonus>()
    );

    /**
     * 批量获取卡牌详细数据
     * @param userCards 多张卡牌
     * @param config 卡牌设置
     * @param eventConfig 活动设置
     * @param areaItemLevels （可选）纳入计算的区域道具等级
     */
    std::vector<CardDetail> batchGetCardDetail(
        const std::vector<UserCard>& userCards,
        const std::unordered_map<int, CardConfig>& config,
        const std::optional<EventConfig>& eventConfig = std::nullopt,
        const std::vector<AreaItemLevel>& areaItemLevels = std::vector<AreaItemLevel>()
    );

    /**
     * 卡牌是否肯定劣于另一张卡牌
     * @param cardDetail0 卡牌
     * @param cardDetail1 另一张卡牌
     */
    bool isCertainlyLessThan(
        const CardDetail& cardDetail0,
        const CardDetail& cardDetail1
    );
};

#endif  // CARD_CALCULATOR_H