#ifndef BASE_DECK_RECOMMEND_H
#define BASE_DECK_RECOMMEND_H

#include "deck-recommend/deck-result-update.h"
#include "deck-information/deck-calculator.h"
#include "card-information/card-calculator.h"
#include "live-score/live-calculator.h"
#include "area-item-information/area-item-service.h"


struct DeckRecommendConfig {
    // 歌曲ID
    int musicId;
    // 歌曲难度
    int musicDiff;
    // 需要推荐的卡组数量（按分数高到低）
    int limit = 1; 
    // 限制人数（2-5、默认5）
    int member = 5; 
    // 卡牌设置
    std::unordered_map<int, CardConfig> cardConfig = {};
};
  

class BaseDeckRecommend {
    
    DataProvider dataProvider;
    DeckCalculator deckCalculator;
    CardCalculator cardCalculator;
    LiveCalculator liveCalculator;
    AreaItemService areaItemService;

public:

    BaseDeckRecommend(DataProvider dataProvider)
        : dataProvider(dataProvider),
          deckCalculator(dataProvider),
          cardCalculator(dataProvider),
          liveCalculator(dataProvider),
          areaItemService(dataProvider) {}

    /**
     * 使用递归寻找最佳卡组
     * 栈深度不超过member+1层
     * 复杂度O(n^member)，带大量剪枝
     * （按分数高到低排序）
     * @param cardDetails 参与计算的卡牌
     * @param allCards 全部卡牌（按支援卡组加成排序）
     * @param scoreFunc 获得分数的公式
     * @param limit 需要推荐的卡组数量（按分数高到低）
     * @param isChallengeLive 是否挑战Live（人员可重复）
     * @param member 人数限制（2-5、默认5）
     * @param honorBonus 称号加成
     * @param eventType （可选）活动类型
     * @param deckCards 计算过程中的当前卡组
     * @param deckCharacters 当前卡组的人员
     */
    void findBestCards(
        const std::vector<CardDetail>& cardDetails,
        const std::vector<CardDetail>& allCards,
        const std::function<int(const DeckDetail&)>& scoreFunc,
        RecommendDeckDfsInfo& dfsInfo,
        int limit = 1,
        bool isChallengeLive = false,
        int member = 5,
        int honorBonus = 0,
        std::optional<int> eventType = std::nullopt,
        std::optional<int> eventId = std::nullopt
    );

    /**
     * 推荐高分卡组
     * @param userCards 参与推荐的卡牌
     * @param scoreFunc 分数计算公式
     * @param musicMeta 歌曲信息
     * @param limit 需要推荐的卡组数量（按分数高到低）
     * @param member 限制人数（2-5、默认5）
     * @param cardConfig 卡牌设置
     * @param debugLog 测试日志处理函数
     * @param liveType Live类型
     * @param eventId 活动ID（如果要计算活动PT的话）
     * @param eventType 活动类型（如果要计算活动PT的话）
     * @param eventUnit 箱活的团队（用于把卡过滤到只剩该团队）
     * @param specialCharacterId 指定角色ID（如果要计算世界开花活动PT的话）
     * @param isChallengeLive 是否挑战Live（人员可重复）
     */
    std::vector<RecommendDeck> recommendHighScoreDeck(
        const std::vector<UserCard>& userCards,
        ScoreFunction scoreFunc,
        const DeckRecommendConfig& config,
        int liveType = 0,
        const EventConfig& eventConfig = {}
    );

};

#endif // BASE_DECK_RECOMMEND_H