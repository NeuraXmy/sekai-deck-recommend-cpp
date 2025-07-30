#include "event-point/event-calculator.h"

static int solo_enum = mapEnum(EnumMap::liveType, "solo");
static int auto_enum = mapEnum(EnumMap::liveType, "auto");
static int challenge_enum = mapEnum(EnumMap::liveType, "challenge");
static int multi_enum = mapEnum(EnumMap::liveType, "multi");
static int cheerful_enum = mapEnum(EnumMap::liveType, "cheerful");

static int cheerful_carnival_enum = mapEnum(EnumMap::eventType, "cheerful_carnival");
static int world_bloom_enum = mapEnum(EnumMap::eventType, "world_bloom");


int EventCalculator::getDeckEventBonus(const std::vector<UserCard> &deckCards, int eventId)
{
    int totalBonus = 0;
    for (const auto &card : deckCards) {
        int cardBonus = this->cardEventCalculator.getCardEventBonus(card, eventId);
        totalBonus += cardBonus;
    }
    return totalBonus;
}

int EventCalculator::getEventPoint(int liveType, int eventType, int selfScore, double musicRate, double deckBonus, double boostRate, int otherScore, int life)
{
    double musicRate0 = musicRate / 100.;
    double deckRate = deckBonus / 100. + 1;
    int otherScore0 = otherScore == 0 ? 4 * selfScore : otherScore;

    double baseScore = 0;
    double lifeRate = 0;

    if (liveType == solo_enum || liveType == auto_enum) {
        baseScore = 100 + int(selfScore / 20000);
        return int(baseScore * musicRate0 * deckRate) * boostRate;
    } 
    else if (liveType == challenge_enum) {
        baseScore = 100 + int(selfScore / 20000);
        return baseScore * 120;
    } 
    else if (liveType == multi_enum) {
        if (eventType == cheerful_carnival_enum) 
            throw std::runtime_error("Multi live is not playable in cheerful event.");
        baseScore = (110 + int(selfScore / 17000.) + std::min(13, int(otherScore0 / 340000.)));
        return int(baseScore * musicRate0 * deckRate) * boostRate;
    } 
    else if (liveType == cheerful_enum) {
        if (eventType != cheerful_carnival_enum) 
            throw std::runtime_error("Cheerful live is only playable in cheerful event.");
        baseScore = (110 + int(selfScore / 17000.) + std::min(13, int(otherScore0 / 340000.)));
        lifeRate = 1.15 + std::min(std::max(life / 5000., 0.1), 0.2);
        return int(int(baseScore * musicRate0 * deckRate) * lifeRate) * boostRate;
    }
    else {
        throw std::runtime_error("Invalid live type");
    }
}

int EventCalculator::getDeckEventPoint(
    const DeckDetail &deckDetail, 
    const MusicMeta &musicMeta, 
    int liveType, 
    int eventType,
    std::optional<int> multiTeammateScoreUp,
    std::optional<int> multiTeammatePower
)
{
    auto deckBonus = deckDetail.eventBonus;
    if (liveType != challenge_enum && !deckBonus.has_value()) 
        throw std::runtime_error("Deck bonus is undefined");

    auto supportDeckBonus = deckDetail.supportDeckBonus;
    if (eventType == world_bloom_enum && !supportDeckBonus.has_value()) 
        throw std::runtime_error("Support deck bonus is undefined");

    auto score = this->liveCalculator.getLiveScoreByDeck(
        deckDetail, musicMeta, liveType, 
        multiTeammateScoreUp, multiTeammatePower
    );
    return this->getEventPoint(liveType, eventType, score, musicMeta.event_rate,
        deckBonus.value_or(0) + supportDeckBonus.value_or(0));
}

ScoreFunction EventCalculator::getEventPointFunction(
    int liveType, 
    int eventType,
    std::optional<int> multiTeammateScoreUp,
    std::optional<int> multiTeammatePower
)
{
    return [this, liveType, eventType, multiTeammateScoreUp, multiTeammatePower]
        (const MusicMeta &musicMeta, const DeckDetail &deckDetail) {
        return this->getDeckEventPoint(
            deckDetail, musicMeta, liveType, eventType,
            multiTeammateScoreUp, multiTeammatePower
        );
    };
}
