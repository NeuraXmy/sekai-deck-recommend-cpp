#ifndef CARD_SKILL_CALCULATOR_H
#define CARD_SKILL_CALCULATOR_H

#include "data-provider/data-provider.h"
#include "card-information/card-detail-map.h"


struct SkillDetail {
    int skillId = 0;
    bool isAfterTraining = false; 

    // 单卡固定的信息
    double scoreUp = 0;
    double lifeRecovery = 0;

    // 需要根据组合计算的信息
    bool hasSameUnitScoreUp = false;
    int sameUnitScoreUpUnit = 0;
    double sameUnitScoreUp = 0;

    bool hasScoreUpReference = false;
    double scoreUpReferenceRate = 0;
    double scoreUpReferenceMax = 0;

    bool hasDifferentUnitCountScoreUp = false;
    std::map<int, double> differentUnitCountScoreUpMap = {};
};

struct DeckCardSkillDetail {
    int skillId = 0;
    bool isAfterTraining = false;

    // 固定组合固定的信息
    double scoreUp = 0;
    double lifeRecovery = 0;

    // 需要根据组合详情计算的信息
    bool hasScoreUpReference = false;
    double scoreUpReferenceRate = 0;
    double scoreUpReferenceMax = 0;
    double scoreUpToReference = 0;
};

class CardSkillCalculator {

    DataProvider dataProvider;

public:

    CardSkillCalculator(DataProvider dataProvider) : dataProvider(dataProvider) {}

    /**
     * 获得不同情况下的卡牌技能
     * @param userCard 用户卡牌
     * @param card 卡牌
     * @param scoreUpLimit 终章应用的加分上限
     */
    CardDetailMap<DeckCardSkillDetail> getCardSkill(
        const UserCard& userCard,
        const Card& card,
        std::optional<double> scoreUpLimit = std::nullopt
    );

    /**
     * 获取卡牌技能
     * @param userCard 用户卡牌
     * @param card 卡牌
     */
    SkillDetail getSkillDetail(
        const UserCard& userCard,
        const Card& card,
        bool afterTraining
    );
    
    /**
     * 获得技能（会根据当前选择的觉醒状态）
     * @param userCard 用户卡牌
     * @param card 卡牌
     */
    Skill getSkill(
        const UserCard& userCard,
        const Card& card,
        bool afterTraining
    );

    /**
     * 获得角色等级
     * @param characterId 角色ID
     */
    int getCharacterRank(
        int characterId
    );
};


#endif // CARD_SKILL_CALCULATOR_H