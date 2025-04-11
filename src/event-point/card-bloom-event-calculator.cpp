#include "event-point/card-bloom-event-calculator.h"

std::optional<double> CardBloomEventCalculator::getCardSupportDeckBonus(const UserCard &userCard, int eventId, int specialCharacterId)
{
    if (specialCharacterId <= 0) return std::nullopt;
    auto& cards = dataProvider.masterData->cards;
    auto card = findOrThrow(cards, [&](const Card& it) { 
        return it.id == userCard.cardId; 
    });

    // 需要先判断一张卡牌是否是指定组合，如果不是的话不使用支援加成
    auto& gameCharacterUnits = dataProvider.masterData->gameCharacterUnits;
    auto specialUnit = findOrThrow(gameCharacterUnits, [&](const GameCharacterUnit& it) { 
        return it.gameCharacterId == specialCharacterId; 
    }).unit;
    auto cardUnits = this->cardService.getCardUnits(card);
    if (std::find(cardUnits.begin(), cardUnits.end(), specialUnit) == cardUnits.end())
        return std::nullopt;
    
    auto& worldBloomSupportDeckBonuses = dataProvider.masterData->worldBloomSupportDeckBonuses;
    auto bonus = findOrThrow(worldBloomSupportDeckBonuses, [&](const WorldBloomSupportDeckBonus& it) { 
        return it.cardRarityType == card.cardRarityType; 
    });
    double total = 0;

    // 角色加成
    int type = mapEnum(EnumMap::worldBloomSupportDeckCharacterType, (card.characterId == specialCharacterId) ? "specific" : "others");
    total += findOrThrow(bonus.worldBloomSupportDeckCharacterBonuses, [&](const WorldBloomSupportDeckCharacterBonus& it) { 
        return it.worldBloomSupportDeckCharacterType == type; 
    }).bonusRate;
    // 专精等级加成
    total += findOrThrow(bonus.worldBloomSupportDeckMasterRankBonuses, [&](const WorldBloomSupportDeckMasterRankBonus& it) { 
        return it.masterRank == userCard.masterRank; 
    }).bonusRate;
    // 技能等级加成
    total += findOrThrow(bonus.worldBloomSupportDeckSkillLevelBonuses, [&](const WorldBloomSupportDeckSkillLevelBonus& it) { 
        return it.skillLevel == userCard.skillLevel; 
    }).bonusRate;

    // 4.5周年，新增了上一期WL卡牌额外加成
    auto& worldBloomSupportDeckUnitEventLimitedBonuses = dataProvider.masterData->worldBloomSupportDeckUnitEventLimitedBonuses;
    for (const auto& bonus : worldBloomSupportDeckUnitEventLimitedBonuses) {
        // 找到合适的卡牌加成
        if (bonus.eventId == eventId && bonus.gameCharacterId == specialCharacterId && bonus.cardId == card.id) {
            total += bonus.bonusRate;
        }
    }
    return total;
}