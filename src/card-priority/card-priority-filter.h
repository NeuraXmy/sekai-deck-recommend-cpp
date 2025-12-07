#ifndef CARD_PRIORITY_FILTER_H
#define CARD_PRIORITY_FILTER_H

#include <map>
#include <set>
#include <vector>
#include "card-information/card-calculator.h"
#include "live-score/live-calculator.h"

// 卡牌优先级
struct CardPriority {
    // 活动加成下界
    int eventBonus;
    // 卡牌稀有度
    int cardRarityType;
    // 突破（专精特训）等级下界
    int masterRank;
    // 优先级从0开始，优先级越高的数字越低
    int priority;
};

/**
 * 使用DFS搜索增广路
 * @param attrMap 属性->角色 多重映射
 * @param attrs 属性->角色 唯一映射
 * @param chars 角色->属性 唯一映射
 * @param visit 属性->轮次 唯一映射
 * @param round 当前轮次
 * @param attr 属性
 */
bool checkAttrForBloomDfs(
    std::unordered_map<int, std::unordered_set<int>>& attrMap,
    std::unordered_map<int, int>& attrs,
    std::unordered_map<int, int>& chars,
    std::unordered_map<int, int>& visit,
    int round,
    int attr
);

/**
 * 为世界开花活动检查是否可以组出5种属性的队伍
 * @param attrMap
 */
bool checkAttrForBloom(
    std::unordered_map<int, std::unordered_set<int>>& attrMap
);

/**
 * 判断某属性或者组合角色数量至少5个
 * @param liveType Live类型
 * @param eventType 活动类型
 * @param cardDetails 卡牌
 * @param member 卡组成员限制
 */
bool canMakeDeck(
    int liveType,
    int eventType,
    std::vector<CardDetail>& cardDetails,
    int member = 5
);

/**
 * 根据给定优先级过滤卡牌
 * @param liveType Live类型
 * @param eventType 活动类型
 * @param cardDetails 卡牌
 * @param preCardDetails 上一次的卡牌，保证返回卡牌数量大于等于它，且能组成队伍
 * @param member 卡组成员限制
 */
std::vector<CardDetail> filterCardPriority(
    int liveType,
    int eventType,
    std::vector<CardDetail>& cardDetails,
    std::vector<CardDetail>& preCardDetails,
    int member = 5
);

/**
 * 卡牌优先级
 * 获取到的卡牌优先级需要根据priority从小到大排列
 * @param liveType Live类型
 * @param eventType 活动类型
 */
std::vector<CardPriority> getCardPriorities(
    int liveType,
    int eventType
);

#endif