#ifndef CARD_SERVICE_H
#define CARD_SERVICE_H

#include "data-provider/data-provider.h"

struct CardConfig {
    bool disable = false;       // 禁用此稀有度卡牌
    bool rankMax = false;       // 强制满级
    bool episodeRead = false;   // 前后篇剧情是否已读
    bool masterMax = false;     // 强制满破
    bool skillMax = false;      // 强制满技能
    bool canvas = false;        // 强制使用画布加成
};

class CardService {

    DataProvider dataProvider;

public:

    CardService(const DataProvider& dataProvider) : dataProvider(dataProvider) {}

    /**
     * 获得卡牌组合信息（包括原始组合与应援组合）
     * @param card 卡牌
     */
    std::vector<int> getCardUnits(const Card& card);

    /**
     * 应用卡牌设置
     * @param userCard 用户卡牌
     * @param card 卡牌
     * @param cardConfig 卡牌配置
     */
    UserCard applyCardConfig(const UserCard& userCard, const Card& card, const CardConfig& cardConfig);

};

#endif // CARD_SERVICE_H