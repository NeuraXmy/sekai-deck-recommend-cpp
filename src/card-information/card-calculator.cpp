#include "card-information/card-calculator.h"
#include "card-calculator.h"

std::optional<CardDetail> CardCalculator::getCardDetail(
    const UserCard& userCard,
    const std::vector<AreaItemLevel>& userAreaItemLevels,
    const std::unordered_map<int, CardConfig>& config,
    const std::unordered_map<int, CardConfig>& singleCardConfig,
    const std::optional<EventConfig>& eventConfig,
    bool hasCanvasBonus,
    const std::vector<MysekaiGateBonus>& userGateBonuses,
    std::optional<double> scoreUpLimit
)
{
    auto& cards = this->dataProvider.masterData->cards;

    Card card{};
    try {
        card = findOrThrow(cards, [&](const auto &it) { 
            return it.id == userCard.cardId; 
        });
    } catch (const ElementNoFoundError& e) {
        std::cerr << "[warning] card id " << userCard.cardId << " appears in user data but not in master data." << std::endl;
        return std::nullopt;
    }

    CardConfig cfg{};
    // 单独卡配置覆盖稀有度卡配置
    if (singleCardConfig.count(card.id))
        cfg = singleCardConfig.at(card.id);
    else if (config.count(card.cardRarityType))
        cfg = config.at(card.cardRarityType);
    
    // 判断禁用
    if (cfg.disable)
        return std::nullopt;

    // 判断强制使用画布
    hasCanvasBonus |= cfg.canvas;

    // 是否是wl终章
    bool isFinalChapter = eventConfig.has_value() ? eventConfig->eventId == finalChapterEventId : false;

    auto userCard0 = this->cardService.applyCardConfig(userCard, card, cfg);
    auto units = this->cardService.getCardUnits(card);
    auto skill = this->skillCalculator.getCardSkill(userCard0, card, scoreUpLimit);
    auto power = this->powerCalculator.getCardPower(
        userCard0, card, units, userAreaItemLevels, hasCanvasBonus, userGateBonuses,
        isFinalChapter ? std::optional<int>(20) : std::nullopt  // 终章限制玩偶加成2%
    );

    CardEventBonusInfo eventBonus{};
    if (eventConfig && eventConfig->eventId != 0) {
        eventBonus = this->eventCalculator.getCardEventBonus(userCard0, eventConfig->eventId);
    }

    // 支援加成延后到组卡前计算

    bool episode1Read = false;
    if (userCard0.episodes.size() > 0) 
        episode1Read = userCard0.episodes[0].scenarioStatus == Enums::ScenarioStatus::already_read;
    bool episode2Read = false;
    if (userCard0.episodes.size() > 1) 
        episode2Read = userCard0.episodes[1].scenarioStatus == Enums::ScenarioStatus::already_read;

    bool afterTraining = userCard0.specialTrainingStatus == Enums::SpecialTrainingStatus::done;

    return CardDetail{
        .cardId = card.id,
        .level = userCard0.level,
        .skillLevel = userCard0.skillLevel,
        .masterRank = userCard0.masterRank,
        .cardRarityType = card.cardRarityType,
        .characterId = card.characterId,
        .units = units,
        .attr = card.attr,
        .power = power,
        .skill = skill,
        .maxEventBonus = eventBonus.maxBonus,
        .minEventBonus = eventBonus.minBonus,
        .limitedEventBonus = eventBonus.limitedBonus,
        .leaderHonorEventBonus = eventBonus.leaderHonorBonus,
        .leaderLimitEventBonus = eventBonus.leaderLimitBonus,
        .supportDeckBonus = std::nullopt,
        .hasCanvasBonus = hasCanvasBonus,
        .episode1Read = episode1Read,
        .episode2Read = episode2Read,
        .afterTraining = afterTraining,
        .defaultImage = userCard.defaultImage
    };
}

std::vector<CardDetail> CardCalculator::batchGetCardDetail(
    const std::vector<UserCard>& userCards,
    const std::unordered_map<int, CardConfig>& config,
    const std::unordered_map<int, CardConfig>& singleCardConfig,
    const std::optional<EventConfig>& eventConfig,
    const std::vector<AreaItemLevel>& areaItemLevels,
    std::optional<double> scoreUpLimit
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
            userCard, areaItemLevels0, config, singleCardConfig, eventConfig, 
            userCanvasBonusCards.find(userCard.cardId) != userCanvasBonusCards.end(),
            userGateBonuses, scoreUpLimit
        );
        if (cardDetail.has_value()) {
            ret.push_back(cardDetail.value());
        }
    }
    return ret;
}

bool CardCalculator::isCertainlyLessThan(
    const CardDetail &cardDetail0, 
    const CardDetail &cardDetail1,
    bool checkPower,
    bool checkSkill,
    bool checkEventBonus
)
{
    bool ret = false;
    if (checkPower)
        ret = (ret && cardDetail0.power.isCertainlyLessThan(cardDetail1.power));
    if (checkSkill)
        ret = (ret && cardDetail0.skill.isCertainlyLessThan(cardDetail1.skill));
    if (checkEventBonus)
        ret = (ret && (cardDetail0.maxEventBonus == std::nullopt || cardDetail1.minEventBonus == std::nullopt ||
            cardDetail0.maxEventBonus.value() < cardDetail1.minEventBonus.value()));
    return ret;
}

SupportDeckCard CardCalculator::getSupportDeckCard(const UserCard &card, int eventId, int specialCharacterId)
{
    auto bonus = this->bloomEventCalculator.getCardSupportDeckBonus(card, eventId, specialCharacterId);
    return SupportDeckCard{
        .cardId = card.cardId,
        .bonus = bonus.value_or(0.0),
    };
}
