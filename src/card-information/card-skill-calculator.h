#ifndef CARD_SKILL_CALCULATOR_H
#define CARD_SKILL_CALCULATOR_H

#include "data-provider/data-provider.h"
#include "card-information/card-detail-map.h"


struct SkillDetail {
    double scoreUp;
    double lifeRecovery;
    bool hasScoreUpEnhance;
    int scoreUpEnhanceUnit;
    double scoreUpEnhanceValue;
    // 用于特殊判断的meta数据（例如bfes花前技能等需要打补丁计算的情况）
    std::map<std::string, std::any> meta; 
};

struct DeckCardSkillDetail {
    double scoreUp;
    double lifeRecovery;
};

class CardSkillCalculator {

    DataProvider dataProvider;

public:

    CardSkillCalculator(DataProvider dataProvider) : dataProvider(dataProvider) {}

    /**
     * 获得不同情况下的卡牌技能
     * @param userCard 用户卡牌
     * @param card 卡牌
     */
    CardDetailMap<DeckCardSkillDetail> getCardSkill(
        const UserCard& userCard,
        const Card& card
    );

    /**
     * 获取卡牌技能
     * @param userCard 用户卡牌
     * @param card 卡牌
     */
    SkillDetail getSkillDetail(
        const UserCard& userCard,
        const Card& card
    );
    
    /**
     * 获得技能（会根据当前选择的觉醒状态）
     * @param userCard 用户卡牌
     * @param card 卡牌
     */
    Skill getSkill(
        const UserCard& userCard,
        const Card& card
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