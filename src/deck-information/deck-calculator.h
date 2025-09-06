#ifndef DECK_CALCULATOR_H
#define DECK_CALCULATOR_H

#include "data-provider/data-provider.h"
#include "deck-information/deck-service.h"
#include "event-point/event-service.h"
#include "card-information/card-calculator.h"

enum class SkillReferenceChooseStrategy {
    Max,
    Min,
    Average,
};

struct DeckBonusInfo {
    std::vector<double> cardBonus{};
    double diffAttrBonus = 0.;
    double totalBonus = 0.;
};

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
     * 获取worldBloom活动的支援卡牌数量
     */
    int getWorldBloomSupportDeckCount(int eventId) const;

    /**
     * 这个函数原本在 EventCalculator 中，为防止循环引用移动到这里
     * 获取卡组活动加成
     * @param deckCards 卡组
     * @param eventType （可选）活动类型
     */
    DeckBonusInfo getDeckBonus(
        const std::vector<const CardDetail*>& deckCards, 
        std::optional<int> eventType = std::nullopt,
        std::optional<int> eventId = std::nullopt
    );

    /**
     * 这个函数原本在 EventCalculator 中，为防止循环引用移动到这里
     * 获取支援卡组加成
     * @param deckCards 卡组
     * @param allCards 所有卡牌（按支援卡组加成从大到小排序）
     * @param supportDeckCount 支援卡组数量
     */
    SupportDeckBonus getSupportDeckBonus(
        const std::vector<const CardDetail*>& deckCards, 
        const std::vector<SupportDeckCard>& supportCards, 
        int supportDeckCount
    );


    /**
     * 获取称号的综合力加成（与卡牌无关、根据称号累加）
     */
    int getHonorBonusPower();

    /**
     * 计算给定的多张卡牌综合力、技能
     * @param cardDetails 处理好的卡牌详情（数组长度1-5，兼容挑战Live）
     * @param supportCards 每个对应角色的排序后的支援队伍卡牌
     * @param honorBonus 称号加成
     * @param eventType 活动类型（用于算加成）
     * @param eventId 活动ID（用于算加成）
     * @param skillReferenceChooseStrategy bfes花前技能参考选择策略
     * @param keepAfterTrainingState 双技能卡是否保留设置状态
     * @param bestSkillAsLeader 是否自动将技能最大值作为队长
     */
    std::vector<DeckDetail> getDeckDetailByCards(
        const std::vector<const CardDetail*>& cardDetails,
        std::map<int, std::vector<SupportDeckCard>>& supportCards,
        int honorBonus = 0,
        std::optional<int> eventType = std::nullopt,
        std::optional<int> eventId = std::nullopt,
        SkillReferenceChooseStrategy skillReferenceChooseStrategy = SkillReferenceChooseStrategy::Average,
        bool keepAfterTrainingState = false,
        bool bestSkillAsLeader = true
    );
};
   
#endif