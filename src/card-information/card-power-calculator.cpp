#include "card-information/card-power-calculator.h"

CardDetailMap<DeckCardPowerDetail> CardPowerCalculator::getCardPower(
    const UserCard &userCard, 
    const Card &card, 
    const std::vector<int> &cardUnits, 
    const std::vector<AreaItemLevel> &userAreaItemLevels, 
    bool hasCanvasBonus, 
    const std::vector<MysekaiGateBonus> &userGateBonuses,
    std::optional<int> fixtureBonusLimit
)
{
    auto ret = CardDetailMap<DeckCardPowerDetail>();
    BasePower basePower = getBasePower(userCard, card, hasCanvasBonus);
    int characterBonus = getCharacterBonusPower(basePower, card.characterId);
    int fixtureBonus = getFixtureBonusPower(basePower, card.characterId, fixtureBonusLimit);
    int gateBonus = getGateBonusPower(basePower, userGateBonuses, cardUnits);
    for (auto unit : cardUnits) {
        // 同组合、同属性
        DeckCardPowerDetail power = getPower(card, basePower, characterBonus, fixtureBonus, gateBonus, userAreaItemLevels, unit, true, true);
        ret.set(unit, 5, 5, power.total, power);
        // 同组合、混属性
        power = getPower(card, basePower, characterBonus, fixtureBonus, gateBonus, userAreaItemLevels, unit, true, false);
        ret.set(unit, 5, 1, power.total, power);
        // 混组合、同属性
        power = getPower(card, basePower, characterBonus, fixtureBonus, gateBonus, userAreaItemLevels, unit, false, true);
        ret.set(unit, 1, 5, power.total, power);
        // 混组合、混属性
        power = getPower(card, basePower, characterBonus, fixtureBonus, gateBonus, userAreaItemLevels, unit, false, false);
        ret.set(unit, 1, 1, power.total, power);
    }
    return ret;
}

DeckCardPowerDetail CardPowerCalculator::getPower(const Card &card, const BasePower &basePower, int characterBonus, int fixtureBonus, int gateBonus, const std::vector<AreaItemLevel> &userAreaItemLevels, int unit, bool sameUnit, bool sameAttr)
{
    int base = sumPower(basePower);
    int areaItemBonus = getAreaItemBonusPower(userAreaItemLevels, basePower, card.characterId, unit, sameUnit, card.attr, sameAttr);
    int total = base + areaItemBonus + characterBonus + fixtureBonus + gateBonus;
    return DeckCardPowerDetail{
        base,
        areaItemBonus,
        characterBonus,
        fixtureBonus,
        gateBonus,
        total
    };
}

BasePower CardPowerCalculator::getBasePower(const UserCard &userCard, const Card &card, bool hasMysekaiCanvas)
{
    auto& cardEpisodes = dataProvider.masterData->cardEpisodes;
    auto& masterLessons = dataProvider.masterData->masterLessons;

    BasePower ret = {0, 0, 0};
    // 等级
    for (auto& it : card.cardParameters) {
        if (it.cardLevel == userCard.level) {
            if (it.cardParameterType == Enums::CardParameterType::param1)
                ret[0] = it.power;
            else if (it.cardParameterType == Enums::CardParameterType::param2)
                ret[1] = it.power;
            else if (it.cardParameterType == Enums::CardParameterType::param3)
                ret[2] = it.power;
        }
    }
    // 觉醒
    if (userCard.specialTrainingStatus == Enums::SpecialTrainingStatus::done) {
        ret[0] += card.specialTrainingPower1BonusFixed;
        ret[1] += card.specialTrainingPower2BonusFixed;
        ret[2] += card.specialTrainingPower3BonusFixed;
    }
    // 剧情
    for (auto& it : userCard.episodes) {
        if (it.scenarioStatus == Enums::ScenarioStatus::already_read) {
            auto episode = findOrThrow(cardEpisodes, [&](auto& e) {
                return e.id == it.cardEpisodeId;
            }, [&]() { return "Card episode not found for cardId=" + std::to_string(card.id) + " episodeId=" + std::to_string(it.cardEpisodeId); });
            ret[0] += episode.power1BonusFixed;
            ret[1] += episode.power2BonusFixed;
            ret[2] += episode.power3BonusFixed;
        }
    }
    // 突破
    for (auto& it : masterLessons) {
        if (it.cardRarityType == card.cardRarityType && it.masterRank <= userCard.masterRank) {
            ret[0] += it.power1BonusFixed;
            ret[1] += it.power2BonusFixed;
            ret[2] += it.power3BonusFixed;
        }
    }
    // 从5.1.0版本开始，画布加成直接算进基础综合力中
    if (hasMysekaiCanvas) {
        auto& cardMysekaiCanvasBonuses = dataProvider.masterData->cardMysekaiCanvasBonuses;
        auto canvasBonus = findOrThrow(cardMysekaiCanvasBonuses, [&](auto& it) {
            return it.cardRarityType == card.cardRarityType;
        }, [&]() { return "Card mysekai canvas bonus not found for cardRarityType=" + std::to_string(card.cardRarityType); });
        ret[0] += canvasBonus.power1BonusFixed;
        ret[1] += canvasBonus.power2BonusFixed;
        ret[2] += canvasBonus.power3BonusFixed;
    }
    return ret;
}

int CardPowerCalculator::getAreaItemBonusPower(const std::vector<AreaItemLevel> &userAreaItemLevels, const BasePower &basePower, int characterId, int unit, bool sameUnit, int attr, bool sameAttr)
{
    double areaItemBonus[3] = {0, 0, 0};
    for (auto& it : userAreaItemLevels) {
        if ((it.targetUnit == Enums::Unit::any || it.targetUnit == unit) &&
            (it.targetCardAttr == Enums::Attr::any || it.targetCardAttr == attr) &&
            (it.targetGameCharacterId == 0 || it.targetGameCharacterId == characterId)) {
            bool allMatch = (it.targetUnit != Enums::Unit::any && sameUnit) ||
                            (it.targetCardAttr != Enums::Attr::any && sameAttr);
            double rates[3] = {0, 0, 0};
            if (allMatch) {
                rates[0] = it.power1AllMatchBonusRate;
                rates[1] = it.power2AllMatchBonusRate;
                rates[2] = it.power3AllMatchBonusRate;
            } else {
                rates[0] = it.power1BonusRate;
                rates[1] = it.power2BonusRate;
                rates[2] = it.power3BonusRate;
            }
            for (int i = 0; i < 3; ++i) {
                areaItemBonus[i] += rates[i] * 0.01 * basePower[i];
            }
        }
    }
    // 三个维度单独计算后向下取整再累加
    int total = 0;
    for (int i = 0; i < 3; ++i) {
        total += std::floor(areaItemBonus[i]);
    }
    return total;
}

int CardPowerCalculator::getCharacterBonusPower(const BasePower &basePower, int characterId)
{
    auto& characterRanks = dataProvider.masterData->characterRanks;
    auto& userCharacters = dataProvider.userData->userCharacters;

    auto userCharacter = findOrThrow(userCharacters, [&](auto& it) {
        return it.characterId == characterId;
    }, [&]() { return "User character not found for characterId=" + std::to_string(characterId); });
    auto characterRank = findOrThrow(characterRanks, [&](auto& it) {
        return it.characterId == userCharacter.characterId &&
               it.characterRank == userCharacter.characterRank;
    }, [&]() { return "Character rank not found for characterId=" + std::to_string(userCharacter.characterId) + " rank=" + std::to_string(userCharacter.characterRank); });
    double rates[3] = {
        characterRank.power1BonusRate,
        characterRank.power2BonusRate,
        characterRank.power3BonusRate
    };
    int total = 0;
    for (int i = 0; i < 3; ++i) {
        total += std::floor(float(rates[i]) * float(0.01) * float(basePower[i]));
    }
    return total;
}

int CardPowerCalculator::getFixtureBonusPower(const BasePower &basePower, int characterId, std::optional<int> limit)
{
    auto& userFixtureBonuses = dataProvider.userData->userMysekaiFixtureGameCharacterPerformanceBonuses;
    if (userFixtureBonuses.empty()) {
        return 0;
    }
    // 寻找对应的加成，如果没有任何加成会空
    try {
        auto& fixtureBonus = findOrThrow(userFixtureBonuses, [&](auto& it) {
            return it.gameCharacterId == characterId;
        });
        double rate = fixtureBonus.totalBonusRate;
        if (limit.has_value()) 
            rate = std::min(rate, double(limit.value()));
        // 按各个综合分别计算加成，其中totalBonusRate单位是0.1%
        int total = sumPower(basePower) * rate * 0.001;
        return std::floor(total);
    } catch (const ElementNoFoundError &e) {
        return 0;
    }
}

int CardPowerCalculator::getGateBonusPower(const BasePower &basePower, const std::vector<MysekaiGateBonus> &userGateBonuses, const std::vector<int> &cardUnits)
{
    bool isOnlyPiapro = cardUnits.size() == 1 && cardUnits[0] == Enums::Unit::piapro;
    double powerBonusRate = 0;
    for (auto& bonus : userGateBonuses) {
        if (isOnlyPiapro || std::find(cardUnits.begin(), cardUnits.end(), bonus.unit) != cardUnits.end()) {
            powerBonusRate = std::max(powerBonusRate, bonus.powerBonusRate);
        }
    }
    // 按各个综合分别计算加成，其中powerBonusRate单位是1%
    double total = sumPower(basePower) * powerBonusRate * 0.01;
    return std::floor(total);
}

int CardPowerCalculator::sumPower(const BasePower &power)
{
    return power[0] + power[1] + power[2];
}
