#include "deck-recommend/deck-result-update.h"

bool RecommendDeck::operator>(const RecommendDeck &other) const
{
    // 先按目标值
    if (targetValue != other.targetValue) return targetValue > other.targetValue;
    // 目标值一样，按C位CardID
    return cards[0].cardId < other.cards[0].cardId;
}

uint64_t getRecommendDeckHash(const RecommendDeck &deck)
{
    // 计算卡组的哈希值
    // 如果分数或者综合不一样，说明肯定不是同一队
    // 如果C位不一样，也不认为是同一队
    uint64_t hash = 0;
    constexpr uint64_t base = 10007;
    hash = hash * base + deck.score;
    hash = hash * base + deck.power.total;
    hash = hash * base + deck.cards[0].cardId;
    return hash;
}

void RecommendCalcInfo::update(const RecommendDeck &deck, int limit)
{
    // 如果已经足够，判断是否劣于当前最差的
    if (int(deckQueue.size()) >= limit && deckQueue.top() > deck)
        return;

    // 判断是否已经存在
    uint64_t hash = getRecommendDeckHash(deck);
    if (deckQueueHashSet.count(hash)) 
        return; 
    deckQueueHashSet.insert(hash);

    deckQueue.push(deck);
    while (int(deckQueue.size()) > limit) {
        deckQueue.pop();
    }
}

bool RecommendCalcInfo::isTimeout()
{
    if (++timeout_check_count % 256 != 0) 
        return is_timeout;
    if (std::chrono::high_resolution_clock::now().time_since_epoch().count() - start_ts > timeout) 
        is_timeout = true;
    return is_timeout;
}
