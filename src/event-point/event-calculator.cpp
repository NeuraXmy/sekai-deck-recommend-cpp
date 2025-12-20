#include "event-point/event-calculator.h"

int EventCalculator::getEventPoint(int liveType, int eventType, int selfScore, double musicRate, double deckBonus, double boostRate, int otherScore, int life)
{
    double musicRate0 = musicRate / 100.;
    double deckRate = deckBonus / 100. + 1;
    int otherScore0 = otherScore == 0 ? 4 * selfScore : otherScore;

    double baseScore = 0;
    double lifeRate = 0;

    if (Enums::LiveType::isChallenge(liveType)) {
        baseScore = 100 + int(selfScore / 20000);
        return baseScore * 120;
    } 
    else if (!Enums::LiveType::isMulti(liveType)) {
        baseScore = 100 + int(selfScore / 20000);
        return int(baseScore * musicRate0 * deckRate) * boostRate;
    }
    else if (liveType == Enums::LiveType::multi_live) {
        if (eventType == Enums::EventType::cheerful) 
            throw std::runtime_error("Multi live is not playable in cheerful event.");
        baseScore = (110 + int(selfScore / 17000.) + std::min(13, int(otherScore0 / 340000.)));
        return int(baseScore * musicRate0 * deckRate) * boostRate;
    } 
    else if (liveType == Enums::LiveType::cheerful_live) {
        if (eventType != Enums::EventType::cheerful)
            throw std::runtime_error("Cheerful live is only playable in cheerful event.");
        baseScore = (110 + int(selfScore / 17000.) + std::min(13, int(otherScore0 / 340000.)));
        lifeRate = 1.15 + std::min(std::max(life / 5000., 0.1), 0.2);
        return int(int(baseScore * musicRate0 * deckRate) * lifeRate) * boostRate;
    }
    else {
        throw std::runtime_error("Invalid live type");
    }
}

Score EventCalculator::getDeckScoreAndEventPoint(
    const DeckDetail &deckDetail, 
    const MusicMeta &musicMeta, 
    int liveType, 
    int eventType,
    LiveSkillOrder liveSkillOrder,
    std::optional<std::vector<int>> specificSkillOrder,
    std::optional<int> multiTeammateScoreUp,
    std::optional<int> multiTeammatePower
)
{
    auto deckBonus = deckDetail.eventBonus;
    if (!Enums::LiveType::isChallenge(liveType) && !deckBonus.has_value()) 
        throw std::runtime_error("Deck bonus is undefined");

    auto supportDeckBonus = deckDetail.supportDeckBonus;
    if (eventType == Enums::EventType::world_bloom && !supportDeckBonus.has_value()) 
        throw std::runtime_error("Support deck bonus is undefined");

    auto liveScore = this->liveCalculator.getLiveScoreByDeck(
        deckDetail, musicMeta, liveType, 
        liveSkillOrder, specificSkillOrder,
        multiTeammateScoreUp, multiTeammatePower
    );
    auto eventPoint = this->getEventPoint(liveType, eventType, liveScore, musicMeta.event_rate,
        deckBonus.value_or(0) + supportDeckBonus.value_or(0));

    Score ret{};
    ret.score = eventPoint;
    ret.liveScore = liveScore;
    return ret;
}

ScoreFunction EventCalculator::getEventPointFunction(
    int liveType, 
    int eventType,
    LiveSkillOrder liveSkillOrder,
    std::optional<std::vector<int>> specificSkillOrder,
    std::optional<int> multiTeammateScoreUp,
    std::optional<int> multiTeammatePower
)
{
    return [this, liveType, eventType, liveSkillOrder, specificSkillOrder, multiTeammateScoreUp, multiTeammatePower]
        (const MusicMeta &musicMeta, const DeckDetail &deckDetail) {
        return this->getDeckScoreAndEventPoint(
            deckDetail, musicMeta, liveType, eventType,
            liveSkillOrder, specificSkillOrder,
            multiTeammateScoreUp, multiTeammatePower
        );
    };
}
