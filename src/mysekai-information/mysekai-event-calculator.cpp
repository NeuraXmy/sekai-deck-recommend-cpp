#include "mysekai-information/mysekai-event-calculator.h"

Score MysekaiEventCalculator::getDeckMysekaiEventPoint(const DeckDetail &deckDetail)
{
    // 公式来源：@SYLVIA0x0
    int power = deckDetail.power.total;
    double event_bonus = deckDetail.eventBonus.value_or(0) + deckDetail.supportDeckBonus.value_or(0);

    double power_bonus = 1 + (power / 450000.0);
    power_bonus = std::floor(power_bonus * 10 + 1e-6) / 10.0;

    event_bonus = std::floor(event_bonus + 1e-6) / 100.0;

    Score ret;
    // 烤森活动点数（分段）
    ret.mysekaiEventPoint = std::floor(power_bonus * (1 + event_bonus) + 1e-6) * 500;
    // 未分段的内部点数用于优化
    ret.mysekaiInternalPoint = power_bonus * (1 + event_bonus) * 500;
    return ret;
}

ScoreFunction MysekaiEventCalculator::getMysekaiEventPointFunction()
{
    return [this] (const MusicMeta &musicMeta, const DeckDetail &deckDetail) {
        return this->getDeckMysekaiEventPoint(deckDetail);
    };
}
