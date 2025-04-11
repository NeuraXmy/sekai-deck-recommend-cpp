#include "card-information/card-calculator.h"

std::optional<CardDetail> CardCalculator::getCardDetail(
    const UserCard& userCard,
    const std::vector<AreaItemLevel>& userAreaItemLevels,
    const std::unordered_map<int, CardConfig>& config,
    const std::optional<EventConfig>& eventConfig,
    bool hasCanvasBonus,
    const std::vector<MysekaiGateBonus>& userGateBonuses
)
{
    auto& cards = this->dataProvider.masterData->cards;
    auto card = findOrThrow(cards, [&](const auto &it) { 
        return it.id == userCard.cardId; 
    });

    auto it1 = config.find(card.cardRarityType);
    if (it1 != config.end() && it1->second.disable) 
        return std::nullopt; // 忽略被禁用的稀有度卡牌
    auto config0 = it1->second;

    auto userCard0 = this->cardService.applyCardConfig(userCard, card, config0);
    auto units = this->cardService.getCardUnits(card);
    auto skill = this->skillCalculator.getCardSkill(userCard0, card);
    auto power = this->powerCalculator.getCardPower(
        userCard0, card, units, userAreaItemLevels, hasCanvasBonus, userGateBonuses
    );
    std::optional<double> eventBonus = std::nullopt;
    if (eventConfig && eventConfig->eventId != 0) {
        eventBonus = this->eventCalculator.getCardEventBonus(userCard0, eventConfig->eventId);
    }
    std::optional<double> supportDeckBonus = std::nullopt;
    if (eventConfig && eventConfig->eventId != 0) {
        supportDeckBonus = this->bloomEventCalculator.getCardSupportDeckBonus(
            userCard0, eventConfig->eventId, eventConfig->specialCharacterId
        );
    }
    return CardDetail{
        card.id,
        userCard0.level,
        userCard0.skillLevel,
        userCard0.masterRank,
        card.cardRarityType,
        card.characterId,
        units,
        card.attr,
        power,
        skill,
        eventBonus,
        supportDeckBonus,
        hasCanvasBonus
    };
}

std::vector<CardDetail> CardCalculator::batchGetCardDetail(
    const std::vector<UserCard>& userCards,
    const std::unordered_map<int, CardConfig>& config,
    const std::optional<EventConfig>& eventConfig,
    const std::vector<AreaItemLevel>& areaItemLevels
)
{
    std::vector<CardDetail> ret{};
    auto areaItemLevels0 = areaItemLevels.empty() ? this->areaItemService.getAreaItemLevels() : areaItemLevels;
    // 自定义世界专项加成
    auto userCanvasBonusCards = this->mysekaiService.getMysekaiCanvasBonusCards();
    auto userGateBonuses = this->mysekaiService.getMysekaiGateBonuses();
    // 每张卡单独计算
    for (const auto &userCard : userCards) {
        auto cardDetail = this->getCardDetail(
            userCard, areaItemLevels0, config, eventConfig, 
            userCanvasBonusCards.find(userCard.cardId) != userCanvasBonusCards.end(),
            userGateBonuses
        );
        if (cardDetail.has_value()) {
            ret.push_back(cardDetail.value());
        }
    }
    // 如果是给世界开花活动算的话，allCards一定要按支援加成从大到小排序
    if (eventConfig && eventConfig->specialCharacterId > 0) {
        std::sort(ret.begin(), ret.end(), [&](const CardDetail &a, const CardDetail &b) {
            return a.supportDeckBonus.value_or(0) > b.supportDeckBonus.value_or(0);
        });
    }
    return ret;
}

bool CardCalculator::isCertainlyLessThan(const CardDetail &cardDetail0, const CardDetail &cardDetail1)
{
    return cardDetail0.power.isCertainlyLessThan(cardDetail1.power) &&
        cardDetail0.skill.isCertainlyLessThan(cardDetail1.skill) &&
        (cardDetail0.eventBonus == std::nullopt || cardDetail1.eventBonus == std::nullopt ||
            cardDetail0.eventBonus.value() <= cardDetail1.eventBonus.value());
}
