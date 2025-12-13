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


void BaseDeckRecommend::findBestCardsDFS(
    int liveType,
    const DeckRecommendConfig& cfg,
    const std::vector<CardDetail> &cardDetails, 
    std::map<int, std::vector<SupportDeckCard>>& supportCards,
    const std::function<Score(const DeckDetail &)> &scoreFunc, 
    RecommendCalcInfo& dfsInfo,
    int limit, 
    bool isChallengeLive, 
    int member, 
    int honorBonus, 
    std::optional<int> eventType,
    std::optional<int> eventId,
    const std::vector<CardDetail>& fixedCards
)
{
    // 超时
    if (dfsInfo.isTimeout()) {
        return;
    }

    auto& deckCards = dfsInfo.deckCards;
    auto& deckCharacters = dfsInfo.deckCharacters;

    // 防止挑战Live卡的数量小于允许上场的数量导致无法组队
    if (isChallengeLive) {
        member = std::min(member, int(cardDetails.size()));
    }
    // 已经是完整卡组，计算当前卡组的值
    if (int(deckCards.size()) == member) {
        auto ret = getBestPermutation(
            this->deckCalculator, deckCards, supportCards, scoreFunc, 
            honorBonus, eventType, eventId, liveType, cfg
        );
        if (ret.bestDeck.has_value())
            dfsInfo.update(ret.bestDeck.value(), limit);
        return;
    }

    // 非完整卡组，继续遍历所有情况
    const CardDetail* preCard = nullptr;

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
        if (!isChallengeLive && deckCharacters.test(card.characterId)) continue;
        // 强制角色限制（不需要考虑固定卡牌，两个参数不允许同时存在）
        if (cfg.fixedCharacters.size() > deckCards.size() && cfg.fixedCharacters[deckCards.size()] != card.characterId) {
            continue;
        }
        
        // C位相关优化，如果使用固定卡牌，则认为C位是第一个不固定的位置，后面的同理（即固定卡牌不参加剪枝）
        auto cIndex = fixedCards.size() + cfg.fixedCharacters.size();
        // C位一定是技能最好的卡牌，跳过技能比C位还好的
        if (deckCards.size() >= cIndex + 1 && deckCards[cIndex]->skill.isCertainlyLessThan(card.skill)) continue;
        // 为了优化性能，必须和C位同色或同组
        if (deckCards.size() >= cIndex + 1 && card.attr != deckCards[cIndex]->attr && !containsAny(deckCards[cIndex]->units, card.units)) {
            continue;
        }

        if (deckCards.size() >= cIndex + 2) {
            auto& last = *deckCards.back();
            bool lessThan = false;
            bool greaterThan = false;
            if (cfg.target == RecommendTarget::Score) {
                lessThan = this->cardCalculator.isCertainlyLessThan(last, card);
                greaterThan = this->cardCalculator.isCertainlyLessThan(card, last);
            } else if(cfg.target == RecommendTarget::Power) {
                lessThan = last.power.isCertainlyLessThan(card.power);
                greaterThan = card.power.isCertainlyLessThan(last.power);
            } else if (cfg.target == RecommendTarget::Skill) {
                lessThan = last.skill.isCertainlyLessThan(card.skill);
                greaterThan = card.skill.isCertainlyLessThan(last.skill);
            }
            // 要求生成的卡组后面4个位置按强弱排序、同强度按卡牌ID排序
            // 如果上一张卡肯定小，那就不符合顺序；
            if (lessThan) continue;
            // 在旗鼓相当的前提下（因为两两组合有四种情况，再排除掉这张卡肯定小的情况，就是旗鼓相当），要ID小
            if (!greaterThan && card.cardId > last.cardId) continue;
        }
        
        if (preCard) {
            auto& pre = *preCard;
            bool lessThan = false;

            if (cfg.target == RecommendTarget::Score) {
                lessThan = this->cardCalculator.isCertainlyLessThan(card, pre);
            } else if (cfg.target == RecommendTarget::Power) {
                lessThan = card.power.isCertainlyLessThan(pre.power);
            } else if (cfg.target == RecommendTarget::Skill) {
                lessThan = card.skill.isCertainlyLessThan(pre.skill);
            } else if (cfg.target == RecommendTarget::Mysekai) {
                lessThan = this->cardCalculator.isCertainlyLessThan(card, pre, true, false, true);
            }

            if (cfg.target == RecommendTarget::Score) {
                // 如果肯定比上一次选定的卡牌要弱，那么舍去，让这张卡去后面再选
                // 该优化较为激进，未考虑卡的协同效应，在计算分数最优的情况下才使用
                if (lessThan) continue;
            } else {
                // 计算实效或综合力最优时性能够用，使用较温和的优化
                if (lessThan && deckCards.size() != member - 1) continue;
            }
        }
        preCard = &card;

        // 递归，寻找所有情况
        deckCards.push_back(&card);
        deckCharacters.flip(card.characterId);

        findBestCardsDFS(
            liveType, cfg, cardDetails, supportCards, scoreFunc, dfsInfo,
            limit, isChallengeLive, member, honorBonus, eventType, eventId, fixedCards
        );

        deckCards.pop_back();
        deckCharacters.flip(card.characterId);
    }
}