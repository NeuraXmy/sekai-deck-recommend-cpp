#include "card-information/card-service.h"

std::vector<int> CardService::getCardUnits(const Card &card)
{
    auto& gameCharacters = dataProvider.masterData->gameCharacters;
    // 组合（V家支援组合、角色原始组合）
    std::vector<int> cardUnits{};
    if (card.supportUnit != mapEnum(EnumMap::unit, "none")) {
        cardUnits.push_back(card.supportUnit);
    }
    cardUnits.push_back(findOrThrow(gameCharacters, [&](const GameCharacter& it) {
        return it.id == card.characterId;
    }).unit);
    return cardUnits;
}

UserCard CardService::applyCardConfig(const UserCard &userCard, const Card &card, const CardConfig &cardConfig)
{   
    bool rankMax = cardConfig.rankMax;
    bool episodeRead = cardConfig.episodeRead;
    bool masterMax = cardConfig.masterMax;
    bool skillMax = cardConfig.skillMax;

    // 都按原样，那就什么都无需调整
    if (!rankMax && !episodeRead && !masterMax && !skillMax) 
        return userCard;

    auto cardRarities = dataProvider.masterData->cardRarities;
    auto cardRarity = findOrThrow(cardRarities, [&](const CardRarity& it) {
        return it.cardRarityType == card.cardRarityType;
    });

    UserCard ret = userCard;

    // 处理最大等级 
    if (rankMax) {
        // 是否可以觉醒
        if (cardRarity.trainingMaxLevel != 0) {
            ret.level = cardRarity.trainingMaxLevel;
            ret.specialTrainingStatus = mapEnum(EnumMap::specialTrainingStatus, "done");
        } else {
            ret.level = cardRarity.maxLevel;
        }
    }

    // 处理前后篇解锁
    if (episodeRead) {
        for (auto& it : ret.episodes) {
            it.scenarioStatus = mapEnum(EnumMap::scenarioStatus, "already_read");
        }
    }

    // 突破
    if (masterMax) {
        ret.masterRank = 5;
    }

    // 技能
    if (skillMax) {
        ret.skillLevel = cardRarity.maxSkillLevel;
    }

    return ret;
}
