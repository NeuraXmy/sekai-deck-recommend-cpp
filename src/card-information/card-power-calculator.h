#ifndef CARD_POWER_CALCULATOR_H
#define CARD_POWER_CALCULATOR_H

#include "data-provider/data-provider.h"
#include "card-information/card-detail-map.h"
#include "card-information/card-service.h"
#include "mysekai-information/mysekai-service.h"

#include <array>

using BasePower = std::array<int, 3>;

struct DeckCardPowerDetail {
    int base;
    int areaItemBonus;
    int characterBonus;
    int fixtureBonus;
    int gateBonus;
    int total;

    std::ostream& operator<<(std::ostream& os) const  {
        os << "DeckCardPowerDetail(base=" << base 
           << ", areaItemBonus=" << areaItemBonus 
           << ", characterBonus=" << characterBonus 
           << ", fixtureBonus=" << fixtureBonus 
           << ", gateBonus=" << gateBonus 
           << ", total=" << total << ")";
        return os;
    }
};
  
class CardPowerCalculator {

    DataProvider dataProvider;

public:

    CardPowerCalculator(DataProvider dataProvider) : dataProvider(dataProvider) {}

    /**
     * 计算在不同情况（组合属性人数不同）下的综合力
     * @param userCard 用户卡牌
     * @param card 卡牌
     * @param cardUnits 卡牌所属组合（因为计算逻辑在CardCalculator里面，传参来避免循环构造）
     * @param userAreaItemLevels 用户拥有的区域道具等级
     * @param hasCanvasBonus 是否拥有自定义世界中的画布
     * @param userGateBonuses 用户拥有的自定义世界大门加成
     */
    CardDetailMap<DeckCardPowerDetail> getCardPower(
        const UserCard& userCard,
        const Card& card,
        const std::vector<int>& cardUnits,
        const std::vector<AreaItemLevel>& userAreaItemLevels,
        bool hasCanvasBonus,
        const std::vector<MysekaiGateBonus>& userGateBonuses,
        std::optional<int> fixtureBonusLimit = std::nullopt
    );
    
    /**
     * 获得卡牌在特定情况下的综合力
     * 将基础综合、角色加成、区域道具加成三部分合一
     * @param card 卡牌
     * @param basePower 基础综合
     * @param characterBonus 角色加成
     * @param fixtureBonus 家具加成
     * @param gateBonus 大门加成
     * @param userAreaItemLevels 用户拥有的区域道具等级
     * @param unit 组合
     * @param sameUnit 是否同组
     * @param sameAttr 是否同色
     */
    DeckCardPowerDetail getPower(
        const Card& card,
        const BasePower& basePower,
        int characterBonus,
        int fixtureBonus,
        int gateBonus,
        const std::vector<AreaItemLevel>& userAreaItemLevels,
        int unit,
        bool sameUnit,
        bool sameAttr
    );

    /**
     * 获取卡牌基础综合力（含卡牌等级、觉醒、突破等级、前后篇、画布加成），这部分综合力直接显示在卡牌右上角，分为3个子属性
     * @param userCard 用户卡牌（要看卡牌等级、觉醒状态、突破等级、前后篇解锁状态）
     * @param card 卡牌
     * @param hasMysekaiCanvas 是否拥有自定义世界中的画布（影响画布加成）
     */
    BasePower getBasePower(
        const UserCard& userCard,
        const Card& card,
        bool hasMysekaiCanvas
    );

    /**
     * 获得区域道具加成的综合力
     * @param userAreaItemLevels 用户所持的区域道具等级
     * @param basePower 卡牌基础综合力
     * @param characterId 角色ID
     * @param unit 用于加成的组合
     * @param sameUnit 是否同组合
     * @param attr 用于加成的属性
     * @param sameAttr 是否同属性
     */
    int getAreaItemBonusPower(
        const std::vector<AreaItemLevel>& userAreaItemLevels,
        const BasePower& basePower,
        int characterId,
        int unit,
        bool sameUnit,
        int attr,
        bool sameAttr
    );

    /**
     * 获取卡牌角色加成综合力
     * @param basePower 卡牌基础综合力
     * @param characterId 角色ID
     */
    int getCharacterBonusPower(
        const BasePower& basePower,
        int characterId
    );

    /**
     * 计算自定义世界中的家具加成（玩偶）
     * @param basePower 卡牌基础综合力
     * @param characterId 角色ID
     */
    int getFixtureBonusPower(
        const BasePower& basePower,
        int characterId,
        std::optional<int> limit = std::nullopt
    );

    /**
     * 自定义世界的大门加成
     * 如果是无应援的V家角色，按最大加成算
     * @param basePower 基础综合
     * @param userGateBonuses 当前生效的门加成
     * @param cardUnits 当前卡有的组合
     */
    int getGateBonusPower(
        const BasePower& basePower,
        const std::vector<MysekaiGateBonus>& userGateBonuses,
        const std::vector<int>& cardUnits
    );

    /**
     * 求和综合力
     * @param power 三维
     */
    int sumPower(const BasePower& power);

};

#endif  // CARD_POWER_CALCULATOR_H