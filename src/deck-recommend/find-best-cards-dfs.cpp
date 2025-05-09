#include "deck-recommend/base-deck-recommend.h"


template<typename T>
bool containsAny(const std::vector<T>& collection, const std::vector<T>& contains) {
    for (const auto& item : collection) {
        if (std::find(contains.begin(), contains.end(), item) != contains.end()) {
            return true;
        }
    }
    return false;
}


void BaseDeckRecommend::findBestCards(
    const std::vector<CardDetail> &cardDetails, 
    const std::vector<CardDetail> &allCards, 
    const std::function<int(const DeckDetail &)> &scoreFunc, 
    RecommendCalcInfo& dfsInfo,
    int limit, 
    bool isChallengeLive, 
    int member, 
    int honorBonus, 
    std::optional<int> eventType,
    std::optional<int> eventId
)
{
    auto& deckCards = dfsInfo.deckCards;
    auto& deckCharacters = dfsInfo.deckCharacters;

    // 防止挑战Live卡的数量小于允许上场的数量导致无法组队
    if (isChallengeLive) {
        member = std::min(member, int(cardDetails.size()));
    }
    // 已经是完整卡组，计算当前卡组的值
    if (int(deckCards.size()) == member) {
        dfsInfo.update(
            getBestPermutation(
                this->deckCalculator, deckCards, allCards, scoreFunc, 
                honorBonus, eventType, eventId
            ), limit
        );
        return;
    }

    // 超时
    if (dfsInfo.isTimeout()) 
        return;

    // 非完整卡组，继续遍历所有情况
    std::optional<CardDetail> preCard = std::nullopt;

    for (const auto& card : cardDetails) {
        // 跳过已经重复出现过的卡牌
        bool has_card = false;
        for (const auto& deckCard : deckCards) {
            if (deckCard->cardId == card.cardId) {
                has_card = true;
                break;
            }
        }
        if (has_card) continue;
        // 跳过重复角色
        if (!isChallengeLive && deckCharacters.count(card.characterId)) continue;
        // C位一定是技能最好的卡牌，跳过技能比C位还好的
        if (deckCards.size() >= 1 && deckCards[0]->skill.isCertainlyLessThan(card.skill)) continue;
        // 为了优化性能，必须和C位同色或同组
        if (deckCards.size() >= 1 && card.attr != deckCards[0]->attr && !containsAny(deckCards[0]->units, card.units)) {
            continue;
        }
        // 要求生成的卡组后面4个位置按强弱排序、同强度按卡牌ID排序
        // 如果上一张卡肯定小，那就不符合顺序；在旗鼓相当的前提下（因为两两组合有四种情况，再排除掉这张卡肯定小的情况，就是旗鼓相当），要ID大
        if (deckCards.size() >= 2 && this->cardCalculator.isCertainlyLessThan(*deckCards[deckCards.size() - 1], card)) continue;
        if (deckCards.size() >= 2 && !this->cardCalculator.isCertainlyLessThan(card, *deckCards[deckCards.size() - 1]) &&
            card.cardId < deckCards[deckCards.size() - 1]->cardId) {
            continue;
        }
        // 如果肯定比上一次选定的卡牌要弱，那么舍去，让这张卡去后面再选
        if (preCard.has_value() && this->cardCalculator.isCertainlyLessThan(card, preCard.value())) continue;
        preCard = card;

        // 递归，寻找所有情况
        deckCards.push_back(&card);
        deckCharacters.insert(card.characterId);
        findBestCards(
            cardDetails, allCards, scoreFunc, dfsInfo,
            limit, isChallengeLive, member, honorBonus, eventType, eventId
        );
        deckCards.pop_back();
        deckCharacters.erase(card.characterId);
    }
}