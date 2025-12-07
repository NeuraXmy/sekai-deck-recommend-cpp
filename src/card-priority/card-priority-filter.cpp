#include "card-priority/card-priority-filter.h"
#include "card-priority/bloom-event-card-priority.h"
#include "card-priority/marathon-cheerful-event-card-priority.h"
#include "card-priority/challenge-live-card-priority.h"


bool checkAttrForBloomDfs(std::unordered_map<int, std::unordered_set<int>> &attrMap, std::unordered_map<int, int> &attrs, std::unordered_map<int, int> &chars, std::unordered_map<int, int> &visit, int round, int attr)
{
    visit[attr] = round;
    auto charForAttr = attrMap.find(attr);
    if (charForAttr == attrMap.end())
        throw std::runtime_error(std::to_string(attr) + " not found in attrMap");
    // 如果还有角色未选择属性，直接选择
    for (const auto &charId : charForAttr->second) {
        auto charIt = chars.find(charId);
        if (charIt == chars.end()) {
            chars[charId] = attr;
            attrs[attr] = charId;
            return true;
        }
    }
    // 不然就要判断有没有角色的属性可以变更
    for (const auto &charId : charForAttr->second) {
        auto attrForCharIt = chars.find(charId);
        if (attrForCharIt == chars.end())
            throw std::runtime_error(std::to_string(charId) + " not found in chars");
        auto& attrForChar = attrForCharIt->second;

        auto attrForCharRoundIt = visit.find(attrForChar);
        if (attrForCharRoundIt == visit.end())
            throw std::runtime_error(std::to_string(attrForChar) + " not found in visit");
        auto& attrForCharRound = attrForCharRoundIt->second;

        if (attrForCharRound != round && checkAttrForBloomDfs(attrMap, attrs, chars, visit, round, attrForChar)) {
            chars[charId] = attr;
            attrs[attr] = charId;
            return true;
        }
    }
    return false;
}

bool checkAttrForBloom(std::unordered_map<int, std::unordered_set<int>> &attrMap)
{
   // 满足不了5色的肯定不行
    if (attrMap.size() < 5)
        return false;
    int min = 114514;
    for (const auto &v : attrMap) {
        min = std::min(min, int(v.second.size()));
    }
    // 如果所有属性的可选角色都大于等于5个，那肯定能满足5色队，不然就要进一步判断
    if (min >= 5)
        return true;

    // 使用二分图最大匹配算法，左半边为5种属性、右半边为26位角色
    // 复杂度O(nm)约等于130
    std::unordered_map<int, int> attrs{};
    std::unordered_map<int, int> chars{};
    std::unordered_map<int, int> visit{};
    int ans = 0;
    int round = 0;
    while (true) {
        round++;
        int count = 0;
        for (const auto &attr : attrMap) {
            if (visit.find(attr.first) == visit.end() 
            && checkAttrForBloomDfs(attrMap, attrs, chars, visit, round, attr.first))
                count++;
        }
        if (count == 0)
            break;
        ans += count;
    }
    return ans == 5;
}

bool canMakeDeck(int liveType, int eventType, std::vector<CardDetail> &cardDetails, int member)
{
    // 统计组合或者属性的不同角色出现次数
    std::unordered_map<int, std::unordered_set<int>> attrMap{};
    std::unordered_map<int, std::unordered_set<int>> unitMap{};
    for (const auto &cardDetail : cardDetails) {
        // 因为挑战Live的卡牌可以重复，所以属性要按卡的数量算
        attrMap[cardDetail.attr].insert(
            Enums::LiveType::isChallenge(liveType) ? cardDetail.cardId : cardDetail.characterId
        );
        for (const auto &unit : cardDetail.units) {
            unitMap[unit].insert(cardDetail.characterId);
        }
    }
    if (Enums::LiveType::isChallenge(liveType) ) {
        // 对于挑战Live来说，如果卡组数量小于5只要有卡够就可以组队了
        if (member < 5) {
            return int(cardDetails.size()) >= member;
        }
        // 不然就要判断能否组出同色队伍
        for (const auto &v : attrMap) {
            if (v.second.size() >= 5)
                return true;
        }
        return false;
    }

    if (eventType == Enums::EventType::marathon || eventType == Enums::EventType::cheerful) {
        // 对于马拉松、欢乐嘉年华活动来说，如果有任何一个大于等于5（能组出同色或同队），就没问题
        for (const auto &v : attrMap) {
            if (v.second.size() >= 5)
                return true;
        }
        for (const auto &v : unitMap) {
            if (v.second.size() >= 5)
                return true;
        }
        return false;
    } else if (eventType == Enums::EventType::world_bloom) {
        // 对于世界开花活动，必须要满足能组出5种属性的队伍，且能组出一个团队
        if (!checkAttrForBloom(attrMap))
            return false;
        // 需要组出至少一个团队
        for (const auto &v : unitMap) {
            if (v.second.size() >= 5)
                return true;
        }
        return false;
    } else {
        // 未知活动类型，只能先认为无论如何都组不出合理队伍，要求全卡计算
        return false;
    }
}

std::vector<CardDetail> filterCardPriority(int liveType, int eventType, std::vector<CardDetail> &cardDetails, std::vector<CardDetail> &preCardDetails, int member)
{
    auto cardPriorities = getCardPriorities(liveType, eventType);
    std::vector<CardDetail> cards{};
    int latestPriority = -114514;
    std::unordered_set<int> cardIds{};
    for (const auto &cardPriority : cardPriorities) {
        // 检查是否已经是符合优先级条件的完整卡组
        // 因为同一个优先级可能有不止一个情况，所以要等遍历到下个优先级后才能决定是否返回
        if (cardPriority.priority > latestPriority && cards.size() > preCardDetails.size() && canMakeDeck(liveType, eventType, cards, member)) {
            return cards;
        }
        latestPriority = cardPriority.priority;
        // 追加符合优先级限制的卡牌
        // 要保证不添加额外的重复卡牌
        for (const auto &it : cardDetails) {
            if (cardIds.find(it.cardId) == cardIds.end() &&
                it.cardRarityType == cardPriority.cardRarityType &&
                it.masterRank >= cardPriority.masterRank &&
                (!it.maxEventBonus.has_value() || it.maxEventBonus >= cardPriority.eventBonus)) {
                cardIds.insert(it.cardId);
                cards.push_back(it);
            }
        }
    }
    // 所有优先级已经结束，直接返回全部卡牌
    return cardDetails;
}

std::vector<CardPriority> getCardPriorities(int liveType, int eventType)
{
    if (Enums::LiveType::isChallenge(liveType))
        return challengeLiveCardPriorities;
    if (eventType == Enums::EventType::world_bloom)
        return bloomCardPriorities;
    if (eventType == Enums::EventType::marathon || eventType == Enums::EventType::cheerful)
        return marathonCheerfulCardPriorities;
    // 如果都不满足，那就只能都不筛选，所有卡全上
    return std::vector<CardPriority>();
}
