#ifndef LIVE_CALCULATOR_H
#define LIVE_CALCULATOR_H

#include "data-provider/data-provider.h"
#include "deck-information/deck-calculator.h"

enum class LiveSkillOrder {
    best,
    worst,
    average,
    specific,
};

union Score { 
    struct {
        int score;
        int liveScore;
    };
    struct {
        int mysekaiEventPoint;
        int mysekaiInternalPoint;
    };
};
using ScoreFunction = std::function<Score(const MusicMeta&, const DeckDetail&)>;

struct SortedSkillDetails {
    std::vector<DeckCardSkillDetail> details;
    bool sorted = false;
};

struct LiveDetail {
    int score;
    double time;
    int life;
    int tap;
    std::optional<DeckDetail> deck = std::nullopt;
};

struct LiveSkill {
    std::optional<int> seq = std::nullopt;
    int cardId;
};


class LiveCalculator {

    DataProvider dataProvider;
    DeckCalculator deckCalculator;

public:

    LiveCalculator(const DataProvider& dataProvider) : 
        dataProvider(dataProvider),
        deckCalculator(dataProvider) {}

    /**
     * 获取歌曲数据
     * @param musicId 歌曲ID
     * @param musicDiff 歌曲难度
     */
    MusicMeta getMusicMeta(int musicId, int musicDif);

    /**
     * 获得基础分数
     * @param musicMeta 歌曲数据
     * @param liveType Live类型
     * @private
     */
    double getBaseScore(const MusicMeta &musicMeta, int liveType);

    /**
     * 获得技能分数
     * @param musicMeta 歌曲数据
     * @param liveType Live类型
     * @private
     */
    std::vector<double> getSkillScore(const MusicMeta &musicMeta, int liveType);

    /**
     * 根据情况排序技能数据
     * @param deckDetail
     * @param liveType
     * @param skillDetails
     */
    SortedSkillDetails getSortedSkillDetails(
        const DeckDetail &deckDetail, 
        int liveType, 
        LiveSkillOrder liveSkillOrder,
        std::optional<std::vector<int>> specificSkillOrder = std::nullopt,
        const std::optional<std::vector<DeckCardSkillDetail>>& skillDetails = std::nullopt,
        std::optional<int> multiTeammateScoreUp = std::nullopt
    );

    /**
     * 根据情况排序技能实际效果
     * @param sorted 技能是否排序
     * @param cardLength 卡组卡牌数量
     * @param skillScores 原始技能效果
     */
    void sortSkillRate(
        bool sorted, 
        int cardLength, 
        std::vector<double>& skillScores
    );

    /**
     * 根据给定的卡组和歌曲数据计算Live详情
     * @param deckDetail 卡组信息
     * @param musicMeta 歌曲数据
     * @param liveType Live类型
     * @param skillDetails 技能顺序（小于5后面技能留空、如果是多人需要放入加权后的累加值），如果留空则计算当前技能多人效果或最佳技能
     * @param multiPowerSum 多人队伍综合力总和（用于计算活跃加成，留空则使用5倍当前卡组综合力）
     */
    LiveDetail getLiveDetailByDeck(
        const DeckDetail &deckDetail, 
        const MusicMeta &musicMeta, 
        int liveType, 
        LiveSkillOrder liveSkillOrder,
        std::optional<std::vector<int>> specificSkillOrder = std::nullopt,
        const std::optional<std::vector<DeckCardSkillDetail>>& skillDetails = std::nullopt,
        int multiPowerSum = 0,
        std::optional<int> multiTeammateScoreUp = std::nullopt,
        std::optional<int> multiTeammatePower = std::nullopt
    );

    /**
     * 获得当前卡组在多人Live下的技能
     * @param deckDetail 卡组信息
     */
    DeckCardSkillDetail getMultiLiveSkill(const DeckDetail &deckDetail);

    /**
     * 按给定顺序计算单人技能效果
     * @param liveSkills 技能顺序（可以小于6个）
     * @param skillDetails 卡组技能信息
     */
    std::optional<std::vector<DeckCardSkillDetail>> getSoloLiveSkill(
        const std::vector<LiveSkill> &liveSkills, 
        const std::vector<DeckCardDetail> &skillDetails
    );

    /**
     * 获取卡组Live分数
     * @param deckDetail 卡组
     * @param musicMeta 歌曲信息
     * @param liveType Live类型
     */
    int getLiveScoreByDeck(
        const DeckDetail &deckDetail, 
        const MusicMeta &musicMeta, 
        int liveType,
        LiveSkillOrder liveSkillOrder,
        std::optional<std::vector<int>> specificSkillOrder = std::nullopt,
        std::optional<int> multiTeammateScoreUp = std::nullopt,
        std::optional<int> multiTeammatePower = std::nullopt
    );

    /**
     * 获取计算歌曲分数的函数
     * @param liveType Live类型
     */
    ScoreFunction getLiveScoreFunction(
        int liveType,
        LiveSkillOrder liveSkillOrder,
        std::optional<std::vector<int>> specificSkillOrder = std::nullopt,
        std::optional<int> multiTeammateScoreUp = std::nullopt,
        std::optional<int> multiTeammatePower = std::nullopt
    );
    
};

#endif // LIVE_CALCULATOR_H