#ifndef BASE_DECK_RECOMMEND_H
#define BASE_DECK_RECOMMEND_H

#include "deck-recommend/deck-result-update.h"
#include "deck-information/deck-calculator.h"
#include "card-information/card-calculator.h"
#include "live-score/live-calculator.h"
#include "area-item-information/area-item-service.h"
#include <random>

using Rng = std::mt19937_64;

enum class RecommendAlgorithm {
    DFS,
    SA,
    GA
};

struct DeckRecommendConfig {
    // 歌曲ID
    int musicId;
    // 歌曲难度
    int musicDiff;
    // 需要推荐的卡组数量（按分数高到低）
    int limit = 1; 
    // 限制人数（2-5、默认5）
    int member = 5; 
    // 超时时间
    int timeout_ms = std::numeric_limits<int>::max();
    // 卡牌设置
    std::unordered_map<int, CardConfig> cardConfig = {};

    // 箱活是否过滤掉其他组合成员
    bool filterOtherUnit = false; 

    // 推荐算法
    RecommendAlgorithm algorithm = RecommendAlgorithm::SA; 

    // 推荐优化目标
    RecommendTarget target = RecommendTarget::Score;

    // 指定加成列表（目标为Bonus时）
    std::vector<int> bonusList = {};

    // 指定一定要包含的卡牌
    std::vector<int> fixedCards = {}; 

    // 强制满画布加成
    bool forceCanvasBonus = false;

    // bfes花前技能选择策略
    SkillReferenceChooseStrategy skillReferenceChooseStrategy = SkillReferenceChooseStrategy::Average;

    // 模拟退火参数
    int saRunCount = 20; // 运行次数
    int saSeed = -1; // 随机数种子 -1 代表使用当前时间
    int saMaxIter = 1000000; // 最大迭代次数
    int saMaxIterNoImprove = 10000; // 最大无改进迭代次数
    int saMaxTimeMs = 200; // 最大运行时间
    double saStartTemperature = 1e8; // 初始温度
    double saCoolingRate = 0.999; // 冷却速率
    bool saDebug = false; // 是否输出调试信息

    // 遗传算法参数
    int gaSeed = -1;
    bool gaDebug = false;
    int gaMaxIter = 1000000; // 最大迭代次数
    int gaMaxIterNoImprove = 5; // 最大无改进迭代次数
    int gaPopSize = 10000; // 种群大小
    int gaParentSize = 1000; // 父代数量
    int gaEliteSize = 0; // 精英数量
    double gaCrossoverRate = 1.0; // 交叉率
    double gaBaseMutationRate = 0.1; // 基础变异率
    double gaNoImproveIterToMutationRate = 0.02; // 无改进迭代次数转换为变异率的比例
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

    // 计算第一位+后几位顺序无关的哈希值
    long long calcDeckHash(const std::vector<const CardDetail*>& deck);

    // 获取卡组的最佳排列并计算分数
    RecommendDeck getBestPermutation(
        DeckCalculator& deckCalculator,
        const std::vector<const CardDetail*> &deckCards,
        const std::vector<CardDetail> &allCards,
        const std::function<int(const DeckDetail &)> &scoreFunc,
        int honorBonus,
        std::optional<int> eventType,
        std::optional<int> eventId,
        int liveType,
        const DeckRecommendConfig& config
    ) const;

    /**
     * 使用递归寻找最佳卡组
     * 栈深度不超过member+1层
     * 复杂度O(n^member)，带大量剪枝
     * （按分数高到低排序）
     * @param cardDetails 参与计算的卡牌
     * @param allCards 全部卡牌（按支援卡组加成排序）
     * @param scoreFunc 获得分数的公式
     * @param dfsInfo DFS信息
     * @param limit 需要推荐的卡组数量（按分数高到低）
     * @param isChallengeLive 是否挑战Live（人员可重复）
     * @param member 人数限制（2-5、默认5）
     * @param honorBonus 称号加成
     * @param eventType （可选）活动类型
     */
    void findBestCardsDFS(
        int liveType,
        const DeckRecommendConfig& config,
        const std::vector<CardDetail>& cardDetails,
        const std::vector<CardDetail>& allCards,
        const std::function<int(const DeckDetail&)>& scoreFunc,
        RecommendCalcInfo& dfsInfo,
        int limit = 1,
        bool isChallengeLive = false,
        int member = 5,
        int honorBonus = 0,
        std::optional<int> eventType = std::nullopt,
        std::optional<int> eventId = std::nullopt,
        const std::vector<CardDetail>& fixedCards = {}
    );

    /**
     * 使用模拟退火寻找最佳卡组
     * （按分数高到低排序）
     * @param config 配置
     * @param cardDetails 参与计算的卡牌
     * @param allCards 全部卡牌（按支援卡组加成排序）
     * @param scoreFunc 获得分数的公式
     * @param dfsInfo DFS信息
     * @param limit 需要推荐的卡组数量（按分数高到低）
     * @param isChallengeLive 是否挑战Live（人员可重复）
     * @param member 人数限制（2-5、默认5）
     * @param honorBonus 称号加成
     * @param eventType （可选）活动类型
     */
    void findBestCardsSA(
        int liveType,
        const DeckRecommendConfig& config,
        Rng& rng,
        const std::vector<CardDetail>& cardDetails,
        const std::vector<CardDetail>& allCards,
        const std::function<int(const DeckDetail&)>& scoreFunc,
        RecommendCalcInfo& dfsInfo,
        int limit = 1,
        bool isChallengeLive = false,
        int member = 5,
        int honorBonus = 0,
        std::optional<int> eventType = std::nullopt,
        std::optional<int> eventId = std::nullopt,
        const std::vector<CardDetail>& fixedCards = {}
    );

    /**
     * 使用遗传算法寻找最佳卡组
     * （按分数高到低排序）
     * @param config 配置
     * @param cardDetails 参与计算的卡牌
     * @param allCards 全部卡牌（按支援卡组加成排序）
     * @param scoreFunc 获得分数的公式
     * @param dfsInfo DFS信息
     * @param limit 需要推荐的卡组数量（按分数高到低）
     * @param isChallengeLive 是否挑战Live（人员可重复）
     * @param member 人数限制（2-5、默认5）
     * @param honorBonus 称号加成
     * @param eventType （可选）活动类型
     */
    void findBestCardsGA(
        int liveType,
        const DeckRecommendConfig& config,
        Rng& rng,
        const std::vector<CardDetail>& cardDetails,
        const std::vector<CardDetail>& allCards,
        const std::function<int(const DeckDetail&)>& scoreFunc,
        RecommendCalcInfo& dfsInfo,
        int limit = 1,
        bool isChallengeLive = false,
        int member = 5,
        int honorBonus = 0,
        std::optional<int> eventType = std::nullopt,
        std::optional<int> eventId = std::nullopt,
        const std::vector<CardDetail>& fixedCards = {}
    );

    /**
     * 使用递归寻找指定非WL活动加成卡组
     * @param cardDetails 参与计算的卡牌
     * @param scoreFunc 获得分数的公式
     * @param dfsInfo DFS信息
     * @param limit 需要推荐的卡组数量
     * @param member 人数限制（2-5、默认5）
     * @param eventType （可选）活动类型
     */
    void findTargetBonusCardsDFS(
        int liveType,
        const DeckRecommendConfig& config,
        const std::vector<CardDetail>& cardDetails,
        const std::function<int(const DeckDetail&)>& scoreFunc,
        RecommendCalcInfo& dfsInfo,
        int limit = 1,
        int member = 5,
        std::optional<int> eventType = std::nullopt,
        std::optional<int> eventId = std::nullopt
    );

    /**
     * 使用递归寻找指定WL活动加成卡组
     * @param cardDetails 参与计算的卡牌
     * @param scoreFunc 获得分数的公式
     * @param dfsInfo DFS信息
     * @param limit 需要推荐的卡组数量
     * @param member 人数限制（2-5、默认5）
     * @param eventType （可选）活动类型
     */
    void findWorldBloomTargetBonusCardsDFS(
        int liveType,
        const DeckRecommendConfig& config,
        const std::vector<CardDetail>& cardDetails,
        const std::function<int(const DeckDetail&)>& scoreFunc,
        RecommendCalcInfo& dfsInfo,
        int limit = 1,
        int member = 5,
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