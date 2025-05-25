#ifndef DECK_RESULT_UPDATE_H
#define DECK_RESULT_UPDATE_H

#include "deck-information/deck-calculator.h"
#include <set>
#include <queue>

enum class RecommendTarget {
    Score,
    Power,
    Skill,
    Bonus,
};

constexpr double SCORE_MAX = 3000000;
constexpr double POWER_MAX = 500000;
constexpr double SKILL_MAX = 500 * 10;  // 实效可能有一位小数点

struct RecommendDeck : DeckDetail {
    // 实际分数
    int score;
    // 期望技能加成（实效）
    double expectSkillBonus;
    // 优化目标值（不一定是分数）
    double targetValue;

    RecommendDeck() = default;

    RecommendDeck(const DeckDetail &deckDetail, RecommendTarget target, int score, double expectSkillBonus)
        : DeckDetail(deckDetail), score(score), expectSkillBonus(expectSkillBonus) {
            int power = deckDetail.power.total;
            // 根据不同优化目标计算目标值
            if (target == RecommendTarget::Power) {
                targetValue = power
                    + double(score) / SCORE_MAX
                    + double(expectSkillBonus) / (SCORE_MAX * SKILL_MAX);
            } else if (target == RecommendTarget::Skill) {
                targetValue = expectSkillBonus
                    + double(score) / SCORE_MAX
                    + double(power) / (SCORE_MAX * POWER_MAX);
            } else {
                targetValue = score
                    + double(power) / POWER_MAX 
                    + double(expectSkillBonus) / (POWER_MAX * SKILL_MAX);
            }
        }

    bool operator>(const RecommendDeck &other) const;
};


// 存储卡组推荐计算的结果以及过程中需要记录的信息
struct RecommendCalcInfo {
    long long start_ts = 0;
    long long timeout = std::numeric_limits<long long>::max();
    std::priority_queue<RecommendDeck, std::vector<RecommendDeck>, std::greater<>> deckQueue = {};
    std::unordered_set<long long> deckHashSet = {};
    
    std::vector<const CardDetail*> deckCards = {};
    std::unordered_set<int> deckCharacters = {};
    std::map<long long, double> deckTargetValueMap{};

    // 添加一个新结果
    void update(const RecommendDeck &deck, int limit);

    // 检查是否超时
    bool isTimeout() const;
};


#endif