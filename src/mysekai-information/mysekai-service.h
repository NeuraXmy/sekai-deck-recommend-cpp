#ifndef MYSEKAI_SERVICE_H
#define MYSEKAI_SERVICE_H

#include "data-provider/data-provider.h"
#include <set>


struct MysekaiGateBonus {
    int unit;
    double powerBonusRate;
};


class MySekaiService {

    DataProvider dataProvider;

public:

    MySekaiService(DataProvider dataProvider) : dataProvider(dataProvider) {}

    /**
     * 获得带有自定义世界画布加成的卡牌
     * 看上去一个卡牌只有加成和不加成两种状态，直接返回卡牌ID列表
     * 计算逻辑：根据稀有度确定固定加成，不享受区域道具、角色等级加成，享受家具、大门加成
     */
    std::unordered_set<int> getMysekaiCanvasBonusCards();

    /**
     * 获得自定义世界的家具加成
     * 很贴心地已经由服务器算好了，直接返回就行
     * 计算逻辑：totalBonusRate的单位看上去是0.1%
     */
    std::vector<UserMysekaiFixtureGameCharacterPerformanceBonus> getMysekaiFixtureBonuses();

    /**
     * 获得自定义世界的大门加成
     * 计算逻辑：原创角色看组合；如果V有支援组合，看支援组合；如果V没有支援组合，取加成最大值
     */
    std::vector<MysekaiGateBonus> getMysekaiGateBonuses();

};


#endif  // MYSEKAI_SERVICE_H