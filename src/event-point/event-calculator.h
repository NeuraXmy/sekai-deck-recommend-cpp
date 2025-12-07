#ifndef EVENT_CALCULATOR_H
#define EVENT_CALCULATOR_H

#include "data-provider/data-provider.h"
#include "deck-information/deck-service.h"
#include "event-point/card-event-calculator.h"
#include "live-score/live-calculator.h"
#include <optional>


class EventCalculator {

    DataProvider dataProvider;
    CardEventCalculator cardEventCalculator;
    LiveCalculator liveCalculator;

public:

    EventCalculator(const DataProvider& dataProvider) : 
        dataProvider(dataProvider),
        cardEventCalculator(dataProvider),
        liveCalculator(dataProvider) {}

    /**
    * 计算活动PT
    * @param liveType Live类型
    * @param eventType 活动类型
    * @param selfScore 个人分数
    * @param musicRate 歌曲系数（百分比、挑战Live无用）
    * @param deckBonus 卡组加成（百分比、挑战Live无用）
    * @param boostRate 消耗系数（挑战Live无用）
    * @param otherScore 他人分数（用于多人Live、留空用4倍自己的分数）
    * @param life 剩余血量（用于对战Live）
    */
    int getEventPoint(
        int liveType,
        int eventType,
        int selfScore,
        double musicRate = 100,
        double deckBonus = 0,
        double boostRate = 1,
        int otherScore = 0,
        int life = 1000
    );

    /**
     * 获得卡组分数和活动点数
     * @param deckDetail 卡组
     * @param musicMeta 歌曲信息
     * @param liveType Live类型
     * @param eventType 活动类型
     */
    Score getDeckScoreAndEventPoint(
        const DeckDetail& deckDetail,
        const MusicMeta& musicMeta,
        int liveType,
        int eventType,
        LiveSkillOrder liveSkillOrder,
        std::optional<std::vector<int>> specificSkillOrder = std::nullopt,
        std::optional<int> multiTeammateScoreUp = std::nullopt,
        std::optional<int> multiTeammatePower = std::nullopt
    );

    /**
     * 获取计算活动PT的函数
     * @param liveType Live类型
     * @param eventType 活动类型
     */
    ScoreFunction getEventPointFunction(
        int liveType, 
        int eventType,
        LiveSkillOrder liveSkillOrder,
        std::optional<std::vector<int>> specificSkillOrder = std::nullopt,
        std::optional<int> multiTeammateScoreUp = std::nullopt,
        std::optional<int> multiTeammatePower = std::nullopt
    );

};

#endif  // EVENT_CALCULATOR_H