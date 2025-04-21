#include "event-point/card-event-calculator.h"

double CardEventCalculator::getEventDeckBonus(int eventId, const Card &card)
{
    auto& eventDeckBonuses = this->dataProvider.masterData->eventDeckBonuses;
    auto& gameCharacterUnits = this->dataProvider.masterData->gameCharacterUnits;
    double maxBonus = 0;

    int no_attr = mapEnum(EnumMap::attr, "");
    int none_unit = mapEnum(EnumMap::unit, "none");

    for (const auto& it : eventDeckBonuses) {
        if (it.eventId == eventId && (it.cardAttr == no_attr || it.cardAttr == card.attr)) {
            // 无指定角色
            if (it.gameCharacterUnitId == 0) {
                maxBonus = std::max(maxBonus, it.bonusRate);
            } else {
                auto unit = findOrThrow(gameCharacterUnits, [it](const GameCharacterUnit& a) { 
                    return a.id == it.gameCharacterUnitId; 
                });
                // 角色不匹配
                if (unit.gameCharacterId != card.characterId) continue;
                // 非虚拟歌手或者组合正确（或者无组合）的虚拟歌手，享受全量加成
                if (card.characterId < 21 || card.supportUnit == unit.unit || card.supportUnit == none_unit) {
                    maxBonus = std::max(maxBonus, it.bonusRate);
                }
            }
        }
    }
    return maxBonus;
}

double CardEventCalculator::getCardEventBonus(const UserCard &userCard, int eventId)
{
    auto& cards = this->dataProvider.masterData->cards;
    auto& eventCards = this->dataProvider.masterData->eventCards;
    auto& eventRarityBonusRates = this->dataProvider.masterData->eventRarityBonusRates;

    // 无活动组卡
    if (eventId == this->dataProvider.masterData->getNoEventFakeEventId()) {
        return 0;
    }

    // 计算角色、属性加成
    double eventBonus = 0;
    auto card = findOrThrow(cards, [&](const Card& it) { 
        return it.id == userCard.cardId; 
    });
    eventBonus += this->getEventDeckBonus(eventId, card);

    // 计算当期卡牌加成
    for (const auto& it : eventCards) {
        if (it.eventId == eventId && it.cardId == card.id) {
            eventBonus += it.bonusRate;
            break;
        }
    }

    // 计算突破等级加成
    auto masterRankBonus = findOrThrow(eventRarityBonusRates, [&](const EventRarityBonusRate& it) { 
        return it.cardRarityType == card.cardRarityType && it.masterRank == userCard.masterRank; 
    });
    eventBonus += masterRankBonus.bonusRate;

    // 实际使用的时候还得/100
    return eventBonus;
}
