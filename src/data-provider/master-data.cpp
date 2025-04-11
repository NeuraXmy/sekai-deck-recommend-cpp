#include "data-provider/master-data.h"

#include <fstream>
#include <iostream>

template <typename T>
std::vector<T> loadMasterData(const std::string& baseDir, const std::string& fileName, bool required = true) {
    std::string filePath = baseDir + "/" + fileName;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        if (required) {
            throw std::runtime_error("Failed to open file: " + filePath);
        } else {
            std::cerr << "[sekai-deck-recommend-cpp] warning: master data not found: " + filePath << std::endl;
            return {};
        }
    }
    json j;
    file >> j;
    file.close();
    return T::fromJsonList(j);
}

MasterData::MasterData(const std::string& baseDir) {
    this->baseDir = baseDir;

    this->areaItemLevels = loadMasterData<AreaItemLevel>(baseDir, "areaItemLevels.json");
    this->areaItems = loadMasterData<AreaItem>(baseDir, "areaItems.json");
    this->areas = loadMasterData<Area>(baseDir, "areas.json");
    this->cardEpisodes = loadMasterData<CardEpisode>(baseDir, "cardEpisodes.json");
    this->cards = loadMasterData<Card>(baseDir, "cards.json");
    this->cardRarities = loadMasterData<CardRarity>(baseDir, "cardRarities.json");
    this->characterRanks = loadMasterData<CharacterRank>(baseDir, "characterRanks.json");
    this->eventCards = loadMasterData<EventCard>(baseDir, "eventCards.json");
    this->eventDeckBonuses = loadMasterData<EventDeckBonus>(baseDir, "eventDeckBonuses.json");
    this->eventExchangeSummaries = loadMasterData<EventExchangeSummary>(baseDir, "eventExchangeSummaries.json");
    this->events = loadMasterData<Event>(baseDir, "events.json");
    this->eventItems = loadMasterData<EventItem>(baseDir, "eventItems.json");
    this->eventRarityBonusRates = loadMasterData<EventRarityBonusRate>(baseDir, "eventRarityBonusRates.json");
    this->gameCharacters = loadMasterData<GameCharacter>(baseDir, "gameCharacters.json");
    this->gameCharacterUnits = loadMasterData<GameCharacterUnit>(baseDir, "gameCharacterUnits.json");
    this->honors = loadMasterData<Honor>(baseDir, "honors.json");
    this->masterLessons = loadMasterData<MasterLesson>(baseDir, "masterLessons.json");
    this->musicDifficulties = loadMasterData<MusicDifficulty>(baseDir, "musicDifficulties.json");
    this->musics = loadMasterData<Music>(baseDir, "musics.json");
    this->musicVocals = loadMasterData<MusicVocal>(baseDir, "musicVocals.json");
    this->shopItems = loadMasterData<ShopItem>(baseDir, "shopItems.json");
    this->skills = loadMasterData<Skill>(baseDir, "skills.json");
    this->worldBloomDifferentAttributeBonuses = loadMasterData<WorldBloomDifferentAttributeBonus>(baseDir, "worldBloomDifferentAttributeBonuses.json");
    this->worldBlooms = loadMasterData<WorldBloom>(baseDir, "worldBlooms.json");
    this->worldBloomSupportDeckBonuses = loadMasterData<WorldBloomSupportDeckBonus>(baseDir, "worldBloomSupportDeckBonuses.json");

    this->worldBloomSupportDeckUnitEventLimitedBonuses = loadMasterData<WorldBloomSupportDeckUnitEventLimitedBonus>(baseDir, "worldBloomSupportDeckUnitEventLimitedBonuses.json", false);

    this->cardMysekaiCanvasBonuses = loadMasterData<CardMysekaiCanvasBonus>(baseDir, "cardMysekaiCanvasBonuses.json", false);
    this->mysekaiFixtureGameCharacterGroups = loadMasterData<MysekaiFixtureGameCharacterGroup>(baseDir, "mysekaiFixtureGameCharacterGroups.json", false);
    this->mysekaiFixtureGameCharacterGroupPerformanceBonuses = loadMasterData<MysekaiFixtureGameCharacterGroupPerformanceBonus>(baseDir, "mysekaiFixtureGameCharacterGroupPerformanceBonuses.json", false);
    this->mysekaiGates = loadMasterData<MysekaiGate>(baseDir, "mysekaiGates.json", false);
    this->mysekaiGateLevels = loadMasterData<MysekaiGateLevel>(baseDir, "mysekaiGateLevels.json", false);
}