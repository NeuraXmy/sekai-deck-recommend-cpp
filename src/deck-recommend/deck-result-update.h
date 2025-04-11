#ifndef DECK_RESULT_UPDATE_H
#define DECK_RESULT_UPDATE_H

#include "deck-information/deck-calculator.h"
#include <set>
#include <queue>


struct RecommendDeck : DeckDetail {
    int score;

    RecommendDeck() = default;

    RecommendDeck(const DeckDetail &deckDetail, int score)
        : DeckDetail(deckDetail), score(score) {}

    bool operator>(const RecommendDeck &other) const;
};


// 存储卡组推荐DFS的结果以及过程中需要记录的信息
struct RecommendDeckDfsInfo {
    std::priority_queue<RecommendDeck, std::vector<RecommendDeck>, std::greater<>> deckQueue = {};
    std::unordered_set<long long> deckHashSet = {};
    std::vector<CardDetail> deckCards = {};
    std::unordered_set<int> deckCharacters = {};

    // 重置
    void reset();

    // 添加一个新结果
    void update(const RecommendDeck &deck, int limit);
};


#endif