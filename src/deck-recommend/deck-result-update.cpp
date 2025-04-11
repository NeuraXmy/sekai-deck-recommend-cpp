#include "deck-recommend/deck-result-update.h"

bool RecommendDeck::operator>(const RecommendDeck &other) const
{
    // 先按分数
    if (score != other.score) return score > other.score;
    // 分数一样，按综合
    if (power.total != other.power.total) return power.total > other.power.total;
    // 分数、综合一样，按C位CardID
    return cards[0].cardId < other.cards[0].cardId;
}

long long getDeckHash(const RecommendDeck &deck)
{
    // 计算卡组的哈希值
    // 如果分数或者综合不一样，说明肯定不是同一队
    // 如果C位不一样，也不认为是同一队
    long long hash = 0;
    constexpr long long base = 10007;
    constexpr long long mod = 1e9 + 7;
    hash = (hash * base + deck.score) % mod;
    hash = (hash * base + deck.power.total) % mod;
    hash = (hash * base + deck.cards[0].cardId) % mod;
    return hash;
}

void RecommendDeckDfsInfo::reset()
{
    while(!deckQueue.empty()) {
        deckQueue.pop();
    }
    deckHashSet.clear();
    deckCards.clear();
    deckCharacters.clear();
}

void RecommendDeckDfsInfo::update(const RecommendDeck &deck, int limit)
{
    // 添加一个新结果
    long long hash = getDeckHash(deck);
    if (deckHashSet.count(hash)) return; // 已经存在了
    deckHashSet.insert(hash);
    deckQueue.push(deck);
    while (int(deckQueue.size()) > limit) {
        deckQueue.pop();
    }
}

