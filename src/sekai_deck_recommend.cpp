#include "deck-recommend/event-deck-recommend.h"
#include "deck-recommend/challenge-live-deck-recommend.h"

#include <iostream>
#include <chrono>
#include <fstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;


static const std::map<std::string, Region> REGION_ENUM_MAP = {
    {"jp", Region::JP},
    {"tw", Region::TW},
    {"en", Region::EN},
    {"kr", Region::KR},
    {"cn", Region::CN},
};

static const std::set<std::string> VALID_ALGORITHMS = {
    "sa",
    "dfs"
};
static const std::set<std::string> VALID_MUSIC_DIFFS = {
    "easy",
    "normal",
    "hard",
    "expert",
    "master",
    "append"
};
static const std::set<std::string> VALID_LIVE_TYPES = {
    "multi",
    "solo",
    "challenge",
    "cheerful",
    "auto"
};
static const std::set<std::string> VALID_RARITY_TYPES = {
    "rarity_4",
    "rarity_birthday",
    "rarity_3",
    "rarity_2",
    "rarity_1"
};
static const std::set<std::string> VALID_UNIT_TYPES = {
    "light_sound",
    "idol",
    "street",
    "theme_park",
    "school_refusal",
    "piapro",
};
static const std::set<std::string> VALID_EVENT_ATTRS = {
    "mysterious",
    "cool",
    "pure",
    "cute",
    "happy",
};

// python传入的card config
struct PyCardConfig {
    std::optional<bool> disable;
    std::optional<bool> level_max;
    std::optional<bool> episode_read;
    std::optional<bool> master_max;
    std::optional<bool> skill_max;
};

// python传入的模拟退火参数
struct PySaOptions {
    std::optional<int> run_num;
    std::optional<int> seed;
    std::optional<int> max_iter;
    std::optional<int> max_no_improve_iter;
    std::optional<int> time_limit_ms;
    std::optional<double> start_temprature;
    std::optional<double> cooling_rate;
    std::optional<bool> debug;
};

// python传入的推荐参数
struct PyDeckRecommendOptions {
    std::optional<std::string> algorithm;
    std::optional<std::string> region;
    std::optional<std::string> user_data_file_path;
    std::optional<std::string> live_type;
    std::optional<int> music_id;
    std::optional<std::string> music_diff;
    std::optional<int> event_id;
    std::optional<std::string> event_attr;
    std::optional<std::string> event_unit;
    std::optional<int> world_bloom_character_id;
    std::optional<int> challenge_live_character_id;
    std::optional<int> limit;
    std::optional<int> member;
    std::optional<int> timeout_ms;
    std::optional<PyCardConfig> rarity_1_config;
    std::optional<PyCardConfig> rarity_2_config;
    std::optional<PyCardConfig> rarity_3_config;
    std::optional<PyCardConfig> rarity_birthday_config;
    std::optional<PyCardConfig> rarity_4_config;
    std::optional<PySaOptions> sa_options;
};

// 单个Card推荐结果
struct PyRecommendCard {
    int card_id;
    int total_power;
    int base_power;
    double event_bonus_rate;
    int master_rank;
    int level;
    int skill_level;
    int skill_score_up;
    int skill_life_recovery;
};

// 单个Deck推荐结果
struct PyRecommendDeck {
    int score;
    int total_power;
    int base_power;
    int area_item_bonus_power;
    int character_bonus_power;
    int honor_bonus_power;
    int fixture_bonus_power;
    int gate_bonus_power;
    double event_bonus_rate;
    double support_deck_bonus_rate;
    std::vector<PyRecommendCard> cards;
};

// 返回python的推荐结果
struct PyDeckRecommendResult {
    std::vector<PyRecommendDeck> decks;
};


class SekaiDeckRecommend {

    mutable std::map<Region, std::shared_ptr<MasterData>> region_masterdata;
    mutable std::map<Region, std::shared_ptr<MusicMetas>> region_musicmetas;

    struct DeckRecommendOptions {
        int liveType = 0;
        int eventId = 0;
        int worldBloomCharacterId = 0;
        int challengeLiveCharacterId = 0;
        DeckRecommendConfig config = {};
        DataProvider dataProvider = {};
    };
    
    DeckRecommendOptions construct_options_from_py(const PyDeckRecommendOptions& pyoptions) const {
        DeckRecommendOptions options = {};

        // region
        if (!pyoptions.region.has_value())
            throw std::invalid_argument("region is required.");
        if (!REGION_ENUM_MAP.count(pyoptions.region.value()))
            throw std::invalid_argument("Invalid region: " + pyoptions.region.value());
        Region region = REGION_ENUM_MAP.at(pyoptions.region.value());

        // data_provider
        if (!pyoptions.user_data_file_path.has_value())
            throw std::invalid_argument("user_data_file_path is required.");
        auto userdata = std::make_shared<UserData>(pyoptions.user_data_file_path.value());

        if (!region_masterdata.count(region))
            throw std::invalid_argument("Master data not found for region: " + pyoptions.region.value());
        auto masterdata = region_masterdata[region];

        if (!region_musicmetas.count(region))
            throw std::invalid_argument("Music metas not found for region: " + pyoptions.region.value());
        auto musicmetas = region_musicmetas[region];

        options.dataProvider = DataProvider{
            region,
            masterdata,
            userdata,
            musicmetas
        };

        // liveType
        if (!pyoptions.live_type.has_value())
            throw std::invalid_argument("live_type is required.");
        if (!VALID_LIVE_TYPES.count(pyoptions.live_type.value()))
            throw std::invalid_argument("Invalid live type: " + pyoptions.live_type.value());
        options.liveType = mapEnum(EnumMap::liveType, pyoptions.live_type.value());
        
        // eventId
        if (pyoptions.event_id.has_value()) {
            if (pyoptions.live_type == "challenge")
                throw std::invalid_argument("event_id is not valid for challenge live.");
            options.eventId = pyoptions.event_id.value();
            findOrThrow(options.dataProvider.masterData->events, [&](const Event& it) {
                return it.id == options.eventId;
            }, "Event not found for eventId: " + std::to_string(options.eventId));
        }
        else {
            if (pyoptions.live_type != "challenge") {
                if (pyoptions.event_attr.has_value() || pyoptions.event_unit.has_value()) {
                    // liveType非挑战，没有传入eventId时，尝试指定团+颜色组卡
                    if (!pyoptions.event_attr.has_value() || !pyoptions.event_unit.has_value())
                        throw std::invalid_argument("event_attr and event_unit must be specified together.");
                    if (!VALID_EVENT_ATTRS.count(pyoptions.event_attr.value()))
                        throw std::invalid_argument("Invalid event attr: " + pyoptions.event_attr.value());
                    if (!VALID_UNIT_TYPES.count(pyoptions.event_unit.value()))
                        throw std::invalid_argument("Invalid event unit: " + pyoptions.event_unit.value());
                    auto unit = mapEnum(EnumMap::unit, pyoptions.event_unit.value());
                    auto attr = mapEnum(EnumMap::attr, pyoptions.event_attr.value());
                    options.eventId = options.dataProvider.masterData->getUnitAttrFakeEventId(unit, attr);
                } else {
                    // 无活动组卡
                    options.eventId = options.dataProvider.masterData->getNoEventFakeEventId();
                }
            } else {
                options.eventId = 0;    
            }
        }

        // challengeLiveCharacterId
        if (pyoptions.challenge_live_character_id.has_value()) {
            options.challengeLiveCharacterId = pyoptions.challenge_live_character_id.value();
            if (options.challengeLiveCharacterId < 1 || options.challengeLiveCharacterId > 26)
                throw std::invalid_argument("Invalid challenge character ID: " + std::to_string(options.challengeLiveCharacterId));
        }
        else {
            if (pyoptions.live_type == "challenge")
                throw std::invalid_argument("challenge_live_character_id is required for challenge live.");
        }

        // worldBloomCharacterId
        if (pyoptions.world_bloom_character_id.has_value()) {
            options.worldBloomCharacterId = pyoptions.world_bloom_character_id.value();
            if (options.worldBloomCharacterId < 1 || options.worldBloomCharacterId > 26)
                throw std::invalid_argument("Invalid world bloom character ID: " + std::to_string(options.worldBloomCharacterId));
            findOrThrow(options.dataProvider.masterData->worldBlooms, [&](const WorldBloom& it) {
                return it.eventId == options.eventId && it.gameCharacterId == options.worldBloomCharacterId;
            }, "World bloom chapter not found for eventId: " + std::to_string(options.eventId) + ", characterId: " + std::to_string(options.worldBloomCharacterId));
        }

        // config
        {
            auto config = DeckRecommendConfig();

            // algorithm
            std::string algorithm = pyoptions.algorithm.value_or("sa");
            if (!VALID_ALGORITHMS.count(algorithm))
                throw std::invalid_argument("Invalid algorithm: " + algorithm);
            if (algorithm == "sa")
                config.useSa = true;
            else if (algorithm == "dfs")
                config.useSa = false;

            // music
            if (!pyoptions.music_id.has_value())
                throw std::invalid_argument("music_id is required.");
            if (!pyoptions.music_diff.has_value())
                throw std::invalid_argument("music_diff is required.");
            config.musicId = pyoptions.music_id.value();
            if (!VALID_MUSIC_DIFFS.count(pyoptions.music_diff.value()))
                throw std::invalid_argument("Invalid music difficulty: " + pyoptions.music_diff.value());
            config.musicDiff = mapEnum(EnumMap::musicDifficulty, pyoptions.music_diff.value());
            findOrThrow(options.dataProvider.musicMetas->metas, [&](const MusicMeta& it) {
                return it.music_id == config.musicId && it.difficulty == config.musicDiff;
            }, "Music meta not found for musicId: " + std::to_string(config.musicId) + ", difficulty: " + pyoptions.music_diff.value());
            
            // limit
            if (pyoptions.limit.has_value()) {
                config.limit = pyoptions.limit.value();
                if (config.limit < 1)
                    throw std::invalid_argument("Invalid limit: " + std::to_string(config.limit));
            }
            else {
                config.limit = 10;
            }

            // member
            if (pyoptions.member.has_value()) {
                config.member = pyoptions.member.value();
                if (config.member < 2 || config.member > 5)
                    throw std::invalid_argument("Invalid member count: " + std::to_string(config.member));
            }
            else {
                config.member = 5;
            }

            // timeout
            if (pyoptions.timeout_ms.has_value()) {
                config.timeout_ms = pyoptions.timeout_ms.value();
                if (config.timeout_ms < 0)
                    throw std::invalid_argument("Invalid timeout: " + std::to_string(config.timeout_ms));
            }

            // card config
            auto card_config_map = std::map<std::string, std::optional<PyCardConfig>>{
                {"rarity_1", pyoptions.rarity_1_config},
                {"rarity_2", pyoptions.rarity_2_config},
                {"rarity_3", pyoptions.rarity_3_config},
                {"rarity_birthday", pyoptions.rarity_birthday_config},
                {"rarity_4", pyoptions.rarity_4_config}
            };
            for (const auto& [key, value] : card_config_map) {
                auto card_config = CardConfig();
                if (value.has_value()) {
                    if (value->disable.has_value())
                        card_config.disable = value->disable.value();
                    if (value->level_max.has_value())
                        card_config.rankMax = value->level_max.value();
                    if (value->episode_read.has_value())
                        card_config.episodeRead = value->episode_read.value();
                    if (value->master_max.has_value())
                        card_config.masterMax = value->master_max.value();
                    if (value->skill_max.has_value())
                        card_config.skillMax = value->skill_max.value();
                }
                config.cardConfig[mapEnum(EnumMap::cardRarityType, key)] = card_config;
            }

            // sa config
            if (config.useSa && pyoptions.sa_options.has_value()) {
                auto sa_options = pyoptions.sa_options.value();

                if (sa_options.run_num.has_value())
                    config.saRunCount = sa_options.run_num.value();
                if (config.saRunCount < 1)
                    throw std::invalid_argument("Invalid sa run count: " + std::to_string(config.saRunCount));
                
                if (sa_options.seed.has_value())
                    config.saSeed = sa_options.seed.value();
                
                if (sa_options.max_iter.has_value())
                    config.saMaxIter = sa_options.max_iter.value();
                if (config.saMaxIter < 1)
                    throw std::invalid_argument("Invalid sa max iter: " + std::to_string(config.saMaxIter));

                if (sa_options.max_no_improve_iter.has_value())
                    config.saMaxIterNoImprove = sa_options.max_no_improve_iter.value();
                if (config.saMaxIterNoImprove < 1)
                    throw std::invalid_argument("Invalid sa max no improve iter: " + std::to_string(config.saMaxIterNoImprove));

                if (sa_options.time_limit_ms.has_value())
                    config.saMaxTimeMs = sa_options.time_limit_ms.value();
                if (config.saMaxTimeMs < 0)
                    throw std::invalid_argument("Invalid sa max time ms: " + std::to_string(config.saMaxTimeMs));

                if (sa_options.start_temprature.has_value())
                    config.saStartTemperature = sa_options.start_temprature.value();
                if (config.saStartTemperature < 0)
                    throw std::invalid_argument("Invalid sa start temperature: " + std::to_string(config.saStartTemperature));

                if (sa_options.cooling_rate.has_value())
                    config.saCoolingRate = sa_options.cooling_rate.value();
                if (config.saCoolingRate < 0 || config.saCoolingRate > 1)
                    throw std::invalid_argument("Invalid sa cooling rate: " + std::to_string(config.saCoolingRate));

                if (sa_options.debug.has_value())
                    config.saDebug = sa_options.debug.value();
            }

            options.config = config;
        }
        return options;
    }

    PyDeckRecommendResult construct_result_to_py(const std::vector<RecommendDeck>& result) const {
        auto ret = PyDeckRecommendResult();
        for (const auto& deck : result) {
            auto py_deck = PyRecommendDeck();
            py_deck.score = deck.score;
            py_deck.total_power = deck.power.total;
            py_deck.base_power = deck.power.base;
            py_deck.area_item_bonus_power = deck.power.areaItemBonus;
            py_deck.character_bonus_power = deck.power.characterBonus;
            py_deck.honor_bonus_power = deck.power.honorBonus;
            py_deck.fixture_bonus_power = deck.power.fixtureBonus;
            py_deck.gate_bonus_power = deck.power.gateBonus;
            py_deck.event_bonus_rate = deck.eventBonus.value_or(0);
            py_deck.support_deck_bonus_rate = deck.supportDeckBonus.value_or(0);

            for (const auto& card : deck.cards) {
                auto py_card = PyRecommendCard();
                py_card.card_id = card.cardId;
                py_card.total_power = card.power.total;
                py_card.base_power = card.power.base;
                py_card.event_bonus_rate = card.eventBonus.value_or(0);
                py_card.master_rank = card.masterRank;
                py_card.level = card.level;
                py_card.skill_level = card.skillLevel;
                py_card.skill_score_up = card.skill.scoreUp;
                py_card.skill_life_recovery = card.skill.lifeRecovery;
                py_deck.cards.push_back(py_card);
            }

            ret.decks.push_back(py_deck);
        }
        return ret;
    }

public:

    // 从指定目录更新区服masterdata数据
    void update_masterdata(const std::string& base_dir, const std::string& region) {
        if (!REGION_ENUM_MAP.count(region)) 
            throw std::invalid_argument("Invalid region: " + region);
        region_masterdata[REGION_ENUM_MAP.at(region)] = std::make_shared<MasterData>(base_dir);
    }

    // 从指定文件更新区服musicmetas数据
    void update_musicmetas(const std::string& file_path, const std::string& region) {
        if (!REGION_ENUM_MAP.count(region)) 
            throw std::invalid_argument("Invalid region: " + region);
        region_musicmetas[REGION_ENUM_MAP.at(region)] = std::make_shared<MusicMetas>(file_path);
    }

    // 推荐卡组
    PyDeckRecommendResult recommend(const PyDeckRecommendOptions& pyoptions) {
        auto options = construct_options_from_py(pyoptions);

        std::vector<RecommendDeck> result;
        
        if (options.liveType != mapEnum(EnumMap::liveType, "challenge")) {
            EventDeckRecommend eventDeckRecommend(options.dataProvider);
            result = eventDeckRecommend.recommendEventDeck(
                options.eventId,
                options.liveType,
                options.config,
                options.worldBloomCharacterId
            );
        } else {
            ChallengeLiveDeckRecommend challengeLiveDeckRecommend(options.dataProvider);
            result = challengeLiveDeckRecommend.recommendChallengeLiveDeck(
                options.challengeLiveCharacterId,
                options.config
            );
        }

        return construct_result_to_py(result);
    }

};


PYBIND11_MODULE(sekai_deck_recommend, m) {
    m.doc() = "pybind11 sekai_deck_recommend plugin";

    py::class_<PyCardConfig>(m, "DeckRecommendCardConfig")
        .def(py::init<>())
        .def(py::init<const PyCardConfig&>())
        .def_readwrite("disable", &PyCardConfig::disable)
        .def_readwrite("level_max", &PyCardConfig::level_max)
        .def_readwrite("episode_read", &PyCardConfig::episode_read)
        .def_readwrite("master_max", &PyCardConfig::master_max)
        .def_readwrite("skill_max", &PyCardConfig::skill_max);

    py::class_<PySaOptions>(m, "DeckRecommendSaOptions")
        .def(py::init<>())
        .def(py::init<const PySaOptions&>())
        .def_readwrite("run_num", &PySaOptions::run_num)
        .def_readwrite("seed", &PySaOptions::seed)
        .def_readwrite("max_iter", &PySaOptions::max_iter)
        .def_readwrite("max_no_improve_iter", &PySaOptions::max_no_improve_iter)
        .def_readwrite("time_limit_ms", &PySaOptions::time_limit_ms)
        .def_readwrite("start_temprature", &PySaOptions::start_temprature)
        .def_readwrite("cooling_rate", &PySaOptions::cooling_rate)
        .def_readwrite("debug", &PySaOptions::debug);
    
    py::class_<PyDeckRecommendOptions>(m, "DeckRecommendOptions")
        .def(py::init<>())
        .def(py::init<const PyDeckRecommendOptions&>())
        .def_readwrite("algorithm", &PyDeckRecommendOptions::algorithm)
        .def_readwrite("region", &PyDeckRecommendOptions::region)
        .def_readwrite("user_data_file_path", &PyDeckRecommendOptions::user_data_file_path)
        .def_readwrite("live_type", &PyDeckRecommendOptions::live_type)
        .def_readwrite("music_id", &PyDeckRecommendOptions::music_id)
        .def_readwrite("music_diff", &PyDeckRecommendOptions::music_diff)
        .def_readwrite("event_id", &PyDeckRecommendOptions::event_id)
        .def_readwrite("event_attr", &PyDeckRecommendOptions::event_attr)
        .def_readwrite("event_unit", &PyDeckRecommendOptions::event_unit)
        .def_readwrite("world_bloom_character_id", &PyDeckRecommendOptions::world_bloom_character_id)
        .def_readwrite("challenge_live_character_id", &PyDeckRecommendOptions::challenge_live_character_id)
        .def_readwrite("limit", &PyDeckRecommendOptions::limit)
        .def_readwrite("member", &PyDeckRecommendOptions::member)
        .def_readwrite("timeout_ms", &PyDeckRecommendOptions::timeout_ms)
        .def_readwrite("rarity_1_config", &PyDeckRecommendOptions::rarity_1_config)
        .def_readwrite("rarity_2_config", &PyDeckRecommendOptions::rarity_2_config)
        .def_readwrite("rarity_3_config", &PyDeckRecommendOptions::rarity_3_config)
        .def_readwrite("rarity_birthday_config", &PyDeckRecommendOptions::rarity_birthday_config)
        .def_readwrite("rarity_4_config", &PyDeckRecommendOptions::rarity_4_config)
        .def_readwrite("sa_options", &PyDeckRecommendOptions::sa_options);

    py::class_<PyRecommendCard>(m, "RecommendCard")
        .def(py::init<>())
        .def(py::init<const PyRecommendCard&>())
        .def_readwrite("card_id", &PyRecommendCard::card_id)
        .def_readwrite("total_power", &PyRecommendCard::total_power)
        .def_readwrite("base_power", &PyRecommendCard::base_power)
        .def_readwrite("event_bonus_rate", &PyRecommendCard::event_bonus_rate)
        .def_readwrite("master_rank", &PyRecommendCard::master_rank)
        .def_readwrite("level", &PyRecommendCard::level)
        .def_readwrite("skill_level", &PyRecommendCard::skill_level)
        .def_readwrite("skill_score_up", &PyRecommendCard::skill_score_up)
        .def_readwrite("skill_life_recovery", &PyRecommendCard::skill_life_recovery);

    py::class_<PyRecommendDeck>(m, "RecommendDeck")
        .def(py::init<>())
        .def(py::init<const PyRecommendDeck&>())
        .def_readwrite("score", &PyRecommendDeck::score)
        .def_readwrite("total_power", &PyRecommendDeck::total_power)
        .def_readwrite("base_power", &PyRecommendDeck::base_power)
        .def_readwrite("area_item_bonus_power", &PyRecommendDeck::area_item_bonus_power)
        .def_readwrite("character_bonus_power", &PyRecommendDeck::character_bonus_power)
        .def_readwrite("honor_bonus_power", &PyRecommendDeck::honor_bonus_power)
        .def_readwrite("fixture_bonus_power", &PyRecommendDeck::fixture_bonus_power)
        .def_readwrite("gate_bonus_power", &PyRecommendDeck::gate_bonus_power)
        .def_readwrite("event_bonus_rate", &PyRecommendDeck::event_bonus_rate)
        .def_readwrite("support_deck_bonus_rate", &PyRecommendDeck::support_deck_bonus_rate)
        .def_readwrite("cards", &PyRecommendDeck::cards);
    
    py::class_<PyDeckRecommendResult>(m, "DeckRecommendResult")
        .def(py::init<>())
        .def(py::init<const PyDeckRecommendResult&>())
        .def_readwrite("decks", &PyDeckRecommendResult::decks);

    py::class_<SekaiDeckRecommend>(m, "SekaiDeckRecommend")
        .def(py::init<>())
        .def(py::init<const SekaiDeckRecommend&>())
        .def("update_masterdata", &SekaiDeckRecommend::update_masterdata)
        .def("update_musicmetas", &SekaiDeckRecommend::update_musicmetas)
        .def("recommend", &SekaiDeckRecommend::recommend);
}

