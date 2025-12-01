#include "mysekai-information/mysekai-service.h"

std::unordered_set<int> MySekaiService::getMysekaiCanvasBonusCards()
{
    auto& userMysekaiCanvas = this->dataProvider.userData->userMysekaiCanvases;
    std::unordered_set<int> result = {};
    for (auto& it : userMysekaiCanvas)
        result.insert(it.cardId);
    return result;
}

std::vector<UserMysekaiFixtureGameCharacterPerformanceBonus> MySekaiService::getMysekaiFixtureBonuses()
{
    return this->dataProvider.userData->userMysekaiFixtureGameCharacterPerformanceBonuses;
}

std::vector<MysekaiGateBonus> MySekaiService::getMysekaiGateBonuses()
{
    auto& userMysekaiGates = this->dataProvider.userData->userMysekaiGates;
    auto& mysekaiGates = this->dataProvider.masterData->mysekaiGates;
    auto& mysekaiGateLevels = this->dataProvider.masterData->mysekaiGateLevels;
    std::vector<MysekaiGateBonus> result = {};
    for (auto& it : userMysekaiGates) {
        auto& gate = findOrThrow(mysekaiGates, [&](const MysekaiGate& g) {
            return g.id == it.mysekaiGateId;
        }, [&]() { return "Mysekai gate not found for mysekaiGateId=" + std::to_string(it.mysekaiGateId); });
        auto& gateLevel = findOrThrow(mysekaiGateLevels, [&](const MysekaiGateLevel& l) {
            return l.mysekaiGateId == it.mysekaiGateId && l.level == it.mysekaiGateLevel;
        }, [&]() { return "Mysekai gate level not found for mysekaiGateId=" + std::to_string(it.mysekaiGateId) + " level=" + std::to_string(it.mysekaiGateLevel); });
        result.push_back(MysekaiGateBonus{
            gate.unit,
            gateLevel.powerBonusRate
        });
    }
    return result;
}
