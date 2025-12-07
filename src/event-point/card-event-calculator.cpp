#include "event-point/card-event-calculator.h"

double CardEventCalculator::getEventDeckBonus(int eventId, const Card &card)
{
    auto& eventDeckBonuses = this->dataProvider.masterData->eventDeckBonuses;
    auto& gameCharacterUnits = this->dataProvider.masterData->gameCharacterUnits;
    double maxBonus = 0;

    for (const auto& it : eventDeckBonuses) {
        if (it.eventId == eventId && (it.cardAttr == Enums::Attr::null || it.cardAttr == card.attr)) {
            // 无指定角色
            if (it.gameCharacterUnitId == 0) {
                maxBonus = std::max(maxBonus, it.bonusRate);
            } else {
                auto unit = findOrThrow(gameCharacterUnits, [it](const GameCharacterUnit& a) { 
                    return a.id == it.gameCharacterUnitId; 
                }, [&]() { return "Game character unit not found for gameCharacterUnitId=" + std::to_string(it.gameCharacterUnitId); });
                // 角色不匹配
                if (unit.gameCharacterId != card.characterId) continue;
                // 非虚拟歌手或者组合正确（或者无组合）的虚拟歌手，享受全量加成
                if (card.characterId < 21 || card.supportUnit == unit.unit || card.supportUnit == Enums::Unit::none) {
                    maxBonus = std::max(maxBonus, it.bonusRate);
                }
            }
        }
    }
    return maxBonus;
}

CardEventBonusInfo CardEventCalculator::getCardEventBonus(const UserCard &userCard, int eventId)
{
    auto& cards = this->dataProvider.masterData->cards;
    auto card = findOrThrow(cards, [&](const Card& it) { 
        return it.id == userCard.cardId; 
    }, [&]() { return "Card not found for cardId=" + std::to_string(userCard.cardId); });
    auto& eventCards = this->dataProvider.masterData->eventCards;
    auto& eventRarityBonusRates = this->dataProvider.masterData->eventRarityBonusRates;

    // 无活动组卡
    if (eventId == this->dataProvider.masterData->getNoEventFakeEventId(Enums::EventType::marathon)
     || eventId == this->dataProvider.masterData->getNoEventFakeEventId(Enums::EventType::cheerful)) {
        return {};
    }

    // 计算角色、属性加成
    double basicBonus = this->getEventDeckBonus(eventId, card);
    // 计算突破等级加成
    auto masterRankBonus = findOrThrow(eventRarityBonusRates, [&](const EventRarityBonusRate& it) { 
        return it.cardRarityType == card.cardRarityType && it.masterRank == userCard.masterRank; 
    }, [&]() { return "Event Rarity Bonus Rate not found for cardRarityType=" + std::to_string(card.cardRarityType) + " masterRank=" + std::to_string(userCard.masterRank); });
    basicBonus += masterRankBonus.bonusRate;

    // 计算当期卡牌加成
    double limitedBonus = 0.0;
    for (const auto& it : eventCards) {
        if (it.eventId == eventId && it.cardId == card.id) {
            limitedBonus = it.bonusRate;
            break;
        }
    }

    // 终章机制
    if (eventId == finalChapterEventId) {
        // 1k牌加成
        double leaderHonorBonus = this->dataProvider.userData->userCharacterFinalChapterHonorEventBonusMap[card.characterId];
        // 当期队长加成
        double leaderLimitBonus = 0.0;
        for (const auto& it : eventCards) {
            if (it.eventId == eventId && it.cardId == card.id) {
                leaderLimitBonus = 20.0;
                break;
            }
        }
        return CardEventBonusInfo{
            .maxBonus = basicBonus + limitedBonus + leaderHonorBonus + leaderLimitBonus,
            .minBonus = basicBonus,     // 终章的当期加成和队长相关的加成都可能不生效
            .limitedBonus = limitedBonus,
            .leaderHonorBonus = leaderHonorBonus,
            .leaderLimitBonus = leaderLimitBonus,
        };
    }
    else {
        return CardEventBonusInfo{
            .maxBonus = basicBonus + limitedBonus,
            .minBonus = basicBonus + limitedBonus,
            .limitedBonus = limitedBonus,
            .leaderHonorBonus = 0.0,
            .leaderLimitBonus = 0.0,
        };
    }
}
