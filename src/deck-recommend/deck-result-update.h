#ifndef DECK_RESULT_UPDATE_H
#define DECK_RESULT_UPDATE_H

#include "deck-information/deck-calculator.h"
#include "live-score/live-calculator.h"
#include <set>
#include <queue>
#include <bitset>

enum class RecommendTarget {
    Score,
    Power,
    Skill,
    Bonus,
    Mysekai,
};

constexpr double SCORE_MAX = 10000000;
constexpr double POWER_MAX = 500000;
constexpr double SKILL_MAX = 500 * 10;  // 实效可能有一位小数点

struct RecommendDeck : DeckDetail {
    // 实际分数或pt
    int score;
    // 游玩歌曲分数
    int liveScore;
    // 多人技能加成（实效）
    double multiLiveScoreUp;
    // 优化目标值（不一定是分数）
    double targetValue;
    // Mysekai活动点数
    int mysekaiEventPoint;

    RecommendDeck() = default;

    RecommendDeck(const DeckDetail &deckDetail, RecommendTarget target, Score s)
        : DeckDetail(deckDetail) {
            if (target == RecommendTarget::Mysekai) {
                // 烤森目标值
                this->targetValue = s.mysekaiInternalPoint;
                this->mysekaiEventPoint = s.mysekaiEventPoint;
                this->score = 0;
                this->liveScore = 0;
                this->multiLiveScoreUp = 0;
            } 
            else {
                this->score = s.score;
                this->liveScore = s.liveScore;
                this->multiLiveScoreUp = deckDetail.multiLiveScoreUp;
                this->mysekaiEventPoint = 0;

                int power = deckDetail.power.total;
                // 根据不同优化目标计算目标值
                if (target == RecommendTarget::Power) {
                    targetValue = power + double(score) / SCORE_MAX;
                } else if (target == RecommendTarget::Skill) {
                    targetValue = multiLiveScoreUp + double(score) / SCORE_MAX;
                } else {
                    targetValue = score + double(liveScore) / SCORE_MAX;
                }
            }
        }

    bool operator>(const RecommendDeck &other) const;
};


// 存储卡组推荐计算的结果以及过程中需要记录的信息
struct RecommendCalcInfo {
    long long start_ts = 0;
    long long timeout = std::numeric_limits<long long>::max();
    int timeout_check_count = 0;
    bool is_timeout = false;
    std::priority_queue<RecommendDeck, std::vector<RecommendDeck>, std::greater<>> deckQueue = {};
    std::unordered_set<uint64_t> deckQueueHashSet = {};
    
    std::vector<const CardDetail*> deckCards = {};
    std::bitset<32> deckCharacters = 0;
    std::unordered_map<uint64_t, double> deckTargetValueMap{};

    // 添加一个新结果
    void update(const RecommendDeck &deck, int limit);

    // 检查是否超时
    bool isTimeout();
};


#endif