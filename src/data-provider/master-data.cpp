#include "data-provider/master-data.h"

#include <fstream>
#include <iostream>
#include "master-data.h"

static int event_type_marathon = mapEnum(EnumMap::eventType, "marathon");
static int event_type_cheerful = mapEnum(EnumMap::eventType, "cheerful_carnival");
static int event_type_world_bloom = mapEnum(EnumMap::eventType, "world_bloom");

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


// 添加用于无活动组卡和指定团+颜色组卡的假活动
void MasterData::addFakeEvent(int eventType) {
    if (eventType != event_type_marathon && eventType != event_type_cheerful) {
        throw std::invalid_argument("Not supported event type for fake event");
    }

    // 无活动组卡
    Event noEvent;
    noEvent.id = getNoEventFakeEventId(eventType);
    noEvent.eventType = eventType;
    events.push_back(noEvent);

    // 指定团名+指定颜色组卡
    for (auto unit : mapEnumList(EnumMap::unit)) {
        for (auto attr : mapEnumList(EnumMap::attr)) {
            Event e;
            e.id = getUnitAttrFakeEventId(eventType, unit, attr);
            e.eventType = event_type_marathon;
            events.push_back(e);
            // 相同团的角色加成
            for (auto& charaUnit : gameCharacterUnits) {
                if (charaUnit.unit == unit) {
                    // 同团同色
                    EventDeckBonus b;
                    b.eventId = e.id;
                    b.gameCharacterUnitId = charaUnit.id;
                    b.cardAttr = attr;
                    b.bonusRate = 50.0;
                    eventDeckBonuses.push_back(b);
                    // 同团不同色
                    EventDeckBonus b2;
                    b2.eventId = e.id;
                    b2.gameCharacterUnitId = charaUnit.id;
                    b2.cardAttr = mapEnum(EnumMap::attr, "");
                    b2.bonusRate = 25.0;
                    eventDeckBonuses.push_back(b2);
                }
            }
            // 不同团同色加成
            EventDeckBonus b;
            b.eventId = e.id;
            b.gameCharacterUnitId = 0;
            b.cardAttr = attr;
            b.bonusRate = 25.0;
            eventDeckBonuses.push_back(b);
        }
    }
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

    addFakeEvent(event_type_marathon);
    addFakeEvent(event_type_cheerful);
}


int MasterData::getNoEventFakeEventId(int eventType) const
{
    if (eventType != event_type_marathon && eventType != event_type_cheerful) {
        throw std::invalid_argument("Not supported event type for fake event");
    }
    return 2000000 + eventType * 100000;
}

int MasterData::getUnitAttrFakeEventId(int eventType, int unit, int attr) const
{
    if (eventType != event_type_marathon && eventType != event_type_cheerful) {
        throw std::invalid_argument("Not supported event type for fake event");
    }
    return 1000000 + unit * 100 + attr + eventType * 100000;
}