#include "deck-recommend/event-deck-recommend.h"
#include "deck-recommend/challenge-live-deck-recommend.h"
#include "deck-recommend/mysekai-deck-recommend.h"
#include "data-provider/static-data.h"

#include <iostream>
#include <chrono>
#include <fstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;


// 初始化静态数据路径
void init_data_path(const std::string& path) {
    setStaticDataDir(path);
}


static const std::map<std::string, Region> REGION_ENUM_MAP = {
    {"jp", Region::JP},
    {"tw", Region::TW},
    {"en", Region::EN},
    {"kr", Region::KR},
    {"cn", Region::CN},
};

static const std::string DEFAULT_TARGET = "score";
static const std::set<std::string> VALID_TARGETS = {
    "score",
    "skill",
    "power",
    "bonus",
};

static const std::string DEFAULT_ALGORITHM = "ga";
static const std::set<std::string> VALID_ALGORITHMS = {
    "sa",
    "dfs",
    "ga",
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
    "auto",
    "mysekai",
    "challenge_auto",
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
static const std::set<std::string> VALID_EVENT_TYPES = {
    "marathon",
    "cheerful_carnival",
    "world_bloom",
};

static const std::string DEFAULT_SKILL_REFERENCE_CHOOSE_STRATEGY = "average";
static const std::set<std::string> VALID_SKILL_REFERENCE_CHOOSE_STRATEGIES = {
    "average",
    "max",
    "min",
};

static const std::string DEFAULT_SKILL_ORDER_CHOOSE_STRATEGY = "average";
static const std::set<std::string> VALID_SKILL_ORDER_CHOOSE_STRATEGIES = {
    "average",
    "max",
    "min",
    "specific",
};

// 用户数据缓存
struct PyUserData {
    std::shared_ptr<UserData> user_data;

    void load_from_file(const std::string& path) {
        user_data = std::make_shared<UserData>();
        user_data->loadFromFile(path);
    }

    void load_from_bytes(const std::string& bytes) {
        user_data = std::make_shared<UserData>();
        user_data->loadFromString(bytes);
    }
};

// python传入的card config
struct PyCardConfig {
    std::optional<bool> disable;
    std::optional<bool> level_max;
    std::optional<bool> episode_read;
    std::optional<bool> master_max;
    std::optional<bool> skill_max;
    std::optional<bool> canvas;

    py::dict to_dict() const {
        py::dict result;
        if (disable.has_value())        result["disable"] = disable.value();
        if (level_max.has_value())      result["level_max"] = level_max.value();
        if (episode_read.has_value())   result["episode_read"] = episode_read.value();
        if (master_max.has_value())     result["master_max"] = master_max.value();
        if (skill_max.has_value())      result["skill_max"] = skill_max.value();
        if (canvas.has_value())         result["canvas"] = canvas.value();
        return result;
    }
    static PyCardConfig from_dict(const py::dict& dict) {
        PyCardConfig config;
        if (dict.contains("disable"))        config.disable = dict["disable"].cast<bool>();
        if (dict.contains("level_max"))      config.level_max = dict["level_max"].cast<bool>();
        if (dict.contains("episode_read"))   config.episode_read = dict["episode_read"].cast<bool>();
        if (dict.contains("master_max"))     config.master_max = dict["master_max"].cast<bool>();
        if (dict.contains("skill_max"))      config.skill_max = dict["skill_max"].cast<bool>();
        if (dict.contains("canvas"))         config.canvas = dict["canvas"].cast<bool>();
        return config;
    }
};

// python传入的单独card config
struct PySingleCardConfig {
    int card_id;
    std::optional<bool> disable;
    std::optional<bool> level_max;
    std::optional<bool> episode_read;
    std::optional<bool> master_max;
    std::optional<bool> skill_max;
    std::optional<bool> canvas;

    py::dict to_dict() const {
        py::dict result;
        result["card_id"] = card_id;
        if (disable.has_value())        result["disable"] = disable.value();
        if (level_max.has_value())      result["level_max"] = level_max.value();
        if (episode_read.has_value())   result["episode_read"] = episode_read.value();
        if (master_max.has_value())     result["master_max"] = master_max.value();
        if (skill_max.has_value())      result["skill_max"] = skill_max.value();
        if (canvas.has_value())         result["canvas"] = canvas.value();
        return result;
    }
    static PySingleCardConfig from_dict(const py::dict& dict) {
        PySingleCardConfig config;
        config.card_id = dict["card_id"].cast<int>();
        if (dict.contains("disable"))        config.disable = dict["disable"].cast<bool>();
        if (dict.contains("level_max"))      config.level_max = dict["level_max"].cast<bool>();
        if (dict.contains("episode_read"))   config.episode_read = dict["episode_read"].cast<bool>();
        if (dict.contains("master_max"))     config.master_max = dict["master_max"].cast<bool>();
        if (dict.contains("skill_max"))      config.skill_max = dict["skill_max"].cast<bool>();
        if (dict.contains("canvas"))         config.canvas = dict["canvas"].cast<bool>();
        return config;
    }
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

    py::dict to_dict() const {
        py::dict result;
        if (run_num.has_value())                result["run_num"] = run_num.value();
        if (seed.has_value())                   result["seed"] = seed.value();
        if (max_iter.has_value())               result["max_iter"] = max_iter.value();
        if (max_no_improve_iter.has_value())    result["max_no_improve_iter"] = max_no_improve_iter.value();
        if (time_limit_ms.has_value())          result["time_limit_ms"] = time_limit_ms.value();
        if (start_temprature.has_value())       result["start_temprature"] = start_temprature.value();
        if (cooling_rate.has_value())           result["cooling_rate"] = cooling_rate.value();
        if (debug.has_value())                  result["debug"] = debug.value();
        return result;
    }
    static PySaOptions from_dict(const py::dict& dict) {
        PySaOptions options;
        if (dict.contains("run_num"))                options.run_num = dict["run_num"].cast<int>();
        if (dict.contains("seed"))                   options.seed = dict["seed"].cast<int>();
        if (dict.contains("max_iter"))               options.max_iter = dict["max_iter"].cast<int>();
        if (dict.contains("max_no_improve_iter"))    options.max_no_improve_iter = dict["max_no_improve_iter"].cast<int>();
        if (dict.contains("time_limit_ms"))          options.time_limit_ms = dict["time_limit_ms"].cast<int>();
        if (dict.contains("start_temprature"))       options.start_temprature = dict["start_temprature"].cast<double>();
        if (dict.contains("cooling_rate"))           options.cooling_rate = dict["cooling_rate"].cast<double>();
        if (dict.contains("debug"))                  options.debug = dict["debug"].cast<bool>();
        return options;
    }
};

// python传入的遗传算法参数
struct PyGaOptions {
    std::optional<int> seed;
    std::optional<bool> debug;
    std::optional<int> max_iter;
    std::optional<int> max_no_improve_iter;
    std::optional<int> pop_size;
    std::optional<int> parent_size;
    std::optional<int> elite_size;
    std::optional<double> crossover_rate;
    std::optional<double> base_mutation_rate;
    std::optional<double> no_improve_iter_to_mutation_rate;

    py::dict to_dict() const {
        py::dict result;
        if (seed.has_value())                        result["seed"] = seed.value();
        if (debug.has_value())                       result["debug"] = debug.value();
        if (max_iter.has_value())                    result["max_iter"] = max_iter.value();
        if (max_no_improve_iter.has_value())         result["max_no_improve_iter"] = max_no_improve_iter.value();
        if (pop_size.has_value())                    result["pop_size"] = pop_size.value();
        if (parent_size.has_value())                 result["parent_size"] = parent_size.value();
        if (elite_size.has_value())                  result["elite_size"] = elite_size.value();
        if (crossover_rate.has_value())              result["crossover_rate"] = crossover_rate.value();
        if (base_mutation_rate.has_value())          result["base_mutation_rate"] = base_mutation_rate.value();
        if (no_improve_iter_to_mutation_rate.has_value())
            result["no_improve_iter_to_mutation_rate"] = no_improve_iter_to_mutation_rate.value();
        return result;
    }
    static PyGaOptions from_dict(const py::dict& dict) {
        PyGaOptions options;
        if (dict.contains("seed"))                        options.seed = dict["seed"].cast<int>();
        if (dict.contains("debug"))                       options.debug = dict["debug"].cast<bool>();
        if (dict.contains("max_iter"))                    options.max_iter = dict["max_iter"].cast<int>();
        if (dict.contains("max_no_improve_iter"))         options.max_no_improve_iter = dict["max_no_improve_iter"].cast<int>();
        if (dict.contains("pop_size"))                    options.pop_size = dict["pop_size"].cast<int>();
        if (dict.contains("parent_size"))                 options.parent_size = dict["parent_size"].cast<int>();
        if (dict.contains("elite_size"))                  options.elite_size = dict["elite_size"].cast<int>();
        if (dict.contains("crossover_rate"))              options.crossover_rate = dict["crossover_rate"].cast<double>();
        if (dict.contains("base_mutation_rate"))          options.base_mutation_rate = dict["base_mutation_rate"].cast<double>();
        if (dict.contains("no_improve_iter_to_mutation_rate"))
            options.no_improve_iter_to_mutation_rate = dict["no_improve_iter_to_mutation_rate"].cast<double>();
        return options;
    }
};

// python传入的推荐参数
struct PyDeckRecommendOptions {
    std::optional<std::string> target;
    std::optional<std::string> algorithm;
    std::optional<std::string> region;
    std::optional<std::string> user_data_file_path;
    std::optional<std::string> user_data_str;
    std::optional<PyUserData> user_data;
    std::optional<std::string> live_type;
    std::optional<int> music_id;
    std::optional<std::string> music_diff;
    std::optional<int> event_id;
    std::optional<std::string> event_attr;
    std::optional<std::string> event_unit;
    std::optional<std::string> event_type;
    std::optional<int> world_bloom_event_turn;
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
    std::optional<std::vector<PySingleCardConfig>> single_card_configs;
    std::optional<bool> filter_other_unit;
    std::optional<std::vector<int>> fixed_cards;
    std::optional<std::vector<int>> fixed_characters;
    std::optional<std::vector<int>> target_bonus_list;
    std::optional<std::string> skill_reference_choose_strategy;
    std::optional<bool> keep_after_training_state;
    std::optional<int> multi_live_teammate_score_up;
    std::optional<int> multi_live_teammate_power;
    std::optional<bool> best_skill_as_leader;
    std::optional<double> multi_live_score_up_lower_bound;
    std::optional<std::string> skill_order_choose_strategy;
    std::optional<std::vector<int>> specific_skill_order;
    std::optional<PySaOptions> sa_options;
    std::optional<PyGaOptions> ga_options;

    py::dict to_dict() const {
        if (user_data.has_value()) 
            throw std::runtime_error("Cannot be converted to dict when user_data is set.");
        py::dict result;
        if (target.has_value())                result["target"] = target.value();
        if (algorithm.has_value())             result["algorithm"] = algorithm.value();
        if (region.has_value())                result["region"] = region.value();
        if (user_data_file_path.has_value())   result["user_data_file_path"] = user_data_file_path.value();
        if (user_data_str.has_value())         result["user_data_str"] = user_data_str.value();       
        if (live_type.has_value())             result["live_type"] = live_type.value();
        if (music_id.has_value())              result["music_id"] = music_id.value();
        if (music_diff.has_value())            result["music_diff"] = music_diff.value();
        if (event_id.has_value())              result["event_id"] = event_id.value();
        if (event_attr.has_value())            result["event_attr"] = event_attr.value();
        if (event_unit.has_value())            result["event_unit"] = event_unit.value();
        if (event_type.has_value())            result["event_type"] = event_type.value();
        if (world_bloom_event_turn.has_value())
            result["world_bloom_event_turn"] = world_bloom_event_turn.value();
        if (world_bloom_character_id.has_value())
            result["world_bloom_character_id"] = world_bloom_character_id.value();
        if (challenge_live_character_id.has_value())
            result["challenge_live_character_id"] = challenge_live_character_id.value();
        if (limit.has_value())                 result["limit"] = limit.value();
        if (member.has_value())                result["member"] = member.value();
        if (timeout_ms.has_value())            result["timeout_ms"] = timeout_ms.value();
        if (rarity_1_config.has_value())
            result["rarity_1_config"] = rarity_1_config->to_dict();
        if (rarity_2_config.has_value())
            result["rarity_2_config"] = rarity_2_config->to_dict();
        if (rarity_3_config.has_value())
            result["rarity_3_config"] = rarity_3_config->to_dict();
        if (rarity_birthday_config.has_value())
            result["rarity_birthday_config"] = rarity_birthday_config->to_dict();
        if (rarity_4_config.has_value())
            result["rarity_4_config"] = rarity_4_config->to_dict();
        if (single_card_configs.has_value()) {
            py::list configs;
            for (const auto& config : single_card_configs.value()) 
                configs.append(config.to_dict());
            result["single_card_configs"] = configs;
        }
        if (filter_other_unit.has_value())
            result["filter_other_unit"] = filter_other_unit.value();
        if (fixed_cards.has_value())
            result["fixed_cards"] = fixed_cards.value();
        if (fixed_characters.has_value())
            result["fixed_characters"] = fixed_characters.value();
        if (target_bonus_list.has_value())
            result["target_bonus_list"] = target_bonus_list.value();
        if (skill_reference_choose_strategy.has_value())
            result["skill_reference_choose_strategy"] = skill_reference_choose_strategy.value();
        if (keep_after_training_state.has_value())
            result["keep_after_training_state"] = keep_after_training_state.value();
        if (multi_live_teammate_score_up.has_value())
            result["multi_live_teammate_score_up"] = multi_live_teammate_score_up.value();
        if (multi_live_teammate_power.has_value())
            result["multi_live_teammate_power"] = multi_live_teammate_power.value();
        if (best_skill_as_leader.has_value())
            result["best_skill_as_leader"] = best_skill_as_leader.value();
        if (multi_live_score_up_lower_bound.has_value())
            result["multi_live_score_up_lower_bound"] = multi_live_score_up_lower_bound.value();
        if (skill_order_choose_strategy.has_value())
            result["skill_order_choose_strategy"] = skill_order_choose_strategy.value();
        if (specific_skill_order.has_value())
            result["specific_skill_order"] = specific_skill_order.value();
        
        if (sa_options.has_value())
            result["sa_options"] = sa_options->to_dict();
        if (ga_options.has_value())
            result["ga_options"] = ga_options->to_dict();
        return result;
    }

    static PyDeckRecommendOptions from_dict(const py::dict& dict) {
        PyDeckRecommendOptions options;
        if (dict.contains("target"))                options.target = dict["target"].cast<std::string>();
        if (dict.contains("algorithm"))             options.algorithm = dict["algorithm"].cast<std::string>();
        if (dict.contains("region"))                options.region = dict["region"].cast<std::string>();
        if (dict.contains("user_data_file_path"))   options.user_data_file_path = dict["user_data_file_path"].cast<std::string>();
        if (dict.contains("user_data_str"))         options.user_data_str = dict["user_data_str"].cast<py::bytes>();
        if (dict.contains("live_type"))             options.live_type = dict["live_type"].cast<std::string>();
        if (dict.contains("music_id"))              options.music_id = dict["music_id"].cast<int>();
        if (dict.contains("music_diff"))            options.music_diff = dict["music_diff"].cast<std::string>();
        if (dict.contains("event_id"))              options.event_id = dict["event_id"].cast<int>();
        if (dict.contains("event_attr"))            options.event_attr = dict["event_attr"].cast<std::string>();
        if (dict.contains("event_unit"))            options.event_unit = dict["event_unit"].cast<std::string>();
        if (dict.contains("event_type"))            options.event_type = dict["event_type"].cast<std::string>();
        if (dict.contains("world_bloom_character_id"))
            options.world_bloom_character_id = dict["world_bloom_character_id"].cast<int>();
        if (dict.contains("world_bloom_event_turn"))
            options.world_bloom_event_turn = dict["world_bloom_event_turn"].cast<int>();
        if (dict.contains("challenge_live_character_id"))
            options.challenge_live_character_id = dict["challenge_live_character_id"].cast<int>();
        if (dict.contains("limit"))                 options.limit = dict["limit"].cast<int>();
        if (dict.contains("member"))                options.member = dict["member"].cast<int>();
        if (dict.contains("timeout_ms"))            options.timeout_ms = dict["timeout_ms"].cast<int>();

        if (dict.contains("rarity_1_config"))
            options.rarity_1_config = PyCardConfig::from_dict(dict["rarity_1_config"].cast<py::dict>());
        if (dict.contains("rarity_2_config"))
            options.rarity_2_config = PyCardConfig::from_dict(dict["rarity_2_config"].cast<py::dict>());
        if (dict.contains("rarity_3_config"))
            options.rarity_3_config = PyCardConfig::from_dict(dict["rarity_3_config"].cast<py::dict>());
        if (dict.contains("rarity_birthday_config"))
            options.rarity_birthday_config = PyCardConfig::from_dict(dict["rarity_birthday_config"].cast<py::dict>());
        if (dict.contains("rarity_4_config"))
            options.rarity_4_config = PyCardConfig::from_dict(dict["rarity_4_config"].cast<py::dict>());    
        if (dict.contains("single_card_configs")) {
            options.single_card_configs = std::vector<PySingleCardConfig>();
            for (const auto& item : dict["single_card_configs"].cast<py::list>()) {
                options.single_card_configs->push_back(PySingleCardConfig::from_dict(item.cast<py::dict>()));
            }
        }
        
        if (dict.contains("filter_other_unit"))
            options.filter_other_unit = dict["filter_other_unit"].cast<bool>();
        if (dict.contains("fixed_cards"))
            options.fixed_cards = dict["fixed_cards"].cast<std::vector<int>>();
        if (dict.contains("fixed_characters"))
            options.fixed_characters = dict["fixed_characters"].cast<std::vector<int>>();
        if (dict.contains("target_bonus_list"))
            options.target_bonus_list = dict["target_bonus_list"].cast<std::vector<int>>();
        if (dict.contains("skill_reference_choose_strategy"))
            options.skill_reference_choose_strategy = dict["skill_reference_choose_strategy"].cast<std::string>();
        if (dict.contains("keep_after_training_state"))
            options.keep_after_training_state = dict["keep_after_training_state"].cast<bool>();
        if (dict.contains("multi_live_teammate_score_up"))
            options.multi_live_teammate_score_up = dict["multi_live_teammate_score_up"].cast<int>();
        if (dict.contains("multi_live_teammate_power"))
            options.multi_live_teammate_power = dict["multi_live_teammate_power"].cast<int>();
        if (dict.contains("best_skill_as_leader"))
            options.best_skill_as_leader = dict["best_skill_as_leader"].cast<bool>();
        if (dict.contains("multi_live_score_up_lower_bound"))
            options.multi_live_score_up_lower_bound = dict["multi_live_score_up_lower_bound"].cast<double>();
        if (dict.contains("skill_order_choose_strategy"))
            options.skill_order_choose_strategy = dict["skill_order_choose_strategy"].cast<std::string>();
        if (dict.contains("specific_skill_order"))
            options.specific_skill_order = dict["specific_skill_order"].cast<std::vector<int>>();

        if (dict.contains("sa_options"))
            options.sa_options = PySaOptions::from_dict(dict["sa_options"].cast<py::dict>());
        if (dict.contains("ga_options"))
            options.ga_options = PyGaOptions::from_dict(dict["ga_options"].cast<py::dict>());
        return options;
    }
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
    bool episode1_read;
    bool episode2_read;
    bool after_training;
    std::string default_image;
    bool has_canvas_bonus;

    py::dict to_dict() const {
        py::dict result;
        result["card_id"] = card_id;
        result["total_power"] = total_power;
        result["base_power"] = base_power;
        result["event_bonus_rate"] = event_bonus_rate;
        result["master_rank"] = master_rank;
        result["level"] = level;
        result["skill_level"] = skill_level;
        result["skill_score_up"] = skill_score_up;
        result["skill_life_recovery"] = skill_life_recovery;
        result["episode1_read"] = episode1_read;
        result["episode2_read"] = episode2_read;
        result["after_training"] = after_training;
        result["default_image"] = default_image;
        result["has_canvas_bonus"] = has_canvas_bonus;
        return result;
    }
    static PyRecommendCard from_dict(const py::dict& dict) {
        PyRecommendCard card;
        card.card_id = dict["card_id"].cast<int>();
        card.total_power = dict["total_power"].cast<int>();
        card.base_power = dict["base_power"].cast<int>();
        card.event_bonus_rate = dict["event_bonus_rate"].cast<double>();
        card.master_rank = dict["master_rank"].cast<int>();
        card.level = dict["level"].cast<int>();
        card.skill_level = dict["skill_level"].cast<int>();
        card.skill_score_up = dict["skill_score_up"].cast<int>();
        card.skill_life_recovery = dict["skill_life_recovery"].cast<int>();
        card.episode1_read = dict["episode1_read"].cast<bool>();
        card.episode2_read = dict["episode2_read"].cast<bool>();
        card.after_training = dict["after_training"].cast<bool>();
        card.default_image = dict["default_image"].cast<std::string>();
        card.has_canvas_bonus = dict["has_canvas_bonus"].cast<bool>();
        return card;
    }
};

// 单个Deck推荐结果
struct PyRecommendDeck {
    int score;
    int live_score;
    int mysekai_event_point;
    int total_power;
    int base_power;
    int area_item_bonus_power;
    int character_bonus_power;
    int honor_bonus_power;
    int fixture_bonus_power;
    int gate_bonus_power;
    double event_bonus_rate;
    double support_deck_bonus_rate;
    double multi_live_score_up;
    std::vector<PyRecommendCard> cards;

    py::dict to_dict() const {
        py::dict result;
        result["score"] = score;
        result["live_score"] = live_score;
        result["mysekai_event_point"] = mysekai_event_point;
        result["total_power"] = total_power;
        result["base_power"] = base_power;
        result["area_item_bonus_power"] = area_item_bonus_power;
        result["character_bonus_power"] = character_bonus_power;
        result["honor_bonus_power"] = honor_bonus_power;
        result["fixture_bonus_power"] = fixture_bonus_power;
        result["gate_bonus_power"] = gate_bonus_power;
        result["event_bonus_rate"] = event_bonus_rate;
        result["support_deck_bonus_rate"] = support_deck_bonus_rate;
        result["multi_live_score_up"] = multi_live_score_up;

        py::list card_list;
        for (const auto& card : cards) {
            card_list.append(card.to_dict());
        }
        result["cards"] = card_list;

        return result;
    }
    static PyRecommendDeck from_dict(const py::dict& dict) {
        PyRecommendDeck deck;
        deck.score = dict["score"].cast<int>();
        deck.live_score = dict["live_score"].cast<int>();
        deck.mysekai_event_point = dict["mysekai_event_point"].cast<int>();
        deck.total_power = dict["total_power"].cast<int>();
        deck.base_power = dict["base_power"].cast<int>();
        deck.area_item_bonus_power = dict["area_item_bonus_power"].cast<int>();
        deck.character_bonus_power = dict["character_bonus_power"].cast<int>();
        deck.honor_bonus_power = dict["honor_bonus_power"].cast<int>();
        deck.fixture_bonus_power = dict["fixture_bonus_power"].cast<int>();
        deck.gate_bonus_power = dict["gate_bonus_power"].cast<int>();
        deck.event_bonus_rate = dict["event_bonus_rate"].cast<double>();
        deck.support_deck_bonus_rate = dict["support_deck_bonus_rate"].cast<double>();
        deck.multi_live_score_up = dict["multi_live_score_up"].cast<double>();

        auto card_list = dict["cards"].cast<py::list>();
        for (const auto& item : card_list) {
            deck.cards.push_back(PyRecommendCard::from_dict(item.cast<py::dict>()));
        }
        
        return deck;
    }
};

// 返回python的推荐结果
struct PyDeckRecommendResult {
    std::vector<PyRecommendDeck> decks;

    py::dict to_dict() const {
        py::dict result;
        py::list deck_list;
        for (const auto& deck : decks) {
            deck_list.append(deck.to_dict());
        }
        result["decks"] = deck_list;
        return result;
    }
    static PyDeckRecommendResult from_dict(const py::dict& dict) {
        PyDeckRecommendResult result;
        auto deck_list = dict["decks"].cast<py::list>();
        for (const auto& item : deck_list) {
            result.decks.push_back(PyRecommendDeck::from_dict(item.cast<py::dict>()));
        }
        return result;
    }
};

// 推荐主类
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

        // user data
        auto userdata = std::make_shared<UserData>();
        if (pyoptions.user_data.has_value())
            userdata = pyoptions.user_data.value().user_data;
        else if (pyoptions.user_data_file_path.has_value())
            userdata->loadFromFile(pyoptions.user_data_file_path.value());
        else if (pyoptions.user_data_str.has_value()) 
            userdata->loadFromString(pyoptions.user_data_str.value());
        else 
            throw std::invalid_argument("Either user_data / user_data_file_path / user_data_str is required.");

        // region master data and music metas
        if (!region_masterdata.count(region))
            throw std::invalid_argument("Master data not found for region: " + pyoptions.region.value());
        auto masterdata = region_masterdata[region];

        if (!region_musicmetas.count(region))
            throw std::invalid_argument("Music metas not found for region: " + pyoptions.region.value());
        auto musicmetas = region_musicmetas[region];

        // dataProvider
        options.dataProvider = DataProvider{
            region,
            masterdata,
            userdata,
            musicmetas
        };

        // liveType
        bool is_mysekai = false;
        if (!pyoptions.live_type.has_value())
            throw std::invalid_argument("live_type is required.");
        if (!VALID_LIVE_TYPES.count(pyoptions.live_type.value()))
            throw std::invalid_argument("Invalid live type: " + pyoptions.live_type.value());
        if (pyoptions.live_type.value() == "mysekai") {
            is_mysekai = true;
            options.liveType = mapEnum(EnumMap::liveType, "multi");
        }
        else
            options.liveType = mapEnum(EnumMap::liveType, pyoptions.live_type.value());
        bool is_challenge_live = Enums::LiveType::isChallenge(options.liveType);
        
        // eventId
        if (pyoptions.event_id.has_value()) {
            if (is_challenge_live)
                throw std::invalid_argument("event_id is not valid for challenge live.");
            options.eventId = pyoptions.event_id.value();
            findOrThrow(options.dataProvider.masterData->events, [&](const Event& it) {
                return it.id == options.eventId;
            }, "Event not found for eventId: " + std::to_string(options.eventId));
        }
        else {
            if (!is_challenge_live) {
                // 活动类型，没有指定则默认马拉松
                auto event_type = pyoptions.event_type.value_or("marathon");
                if (!VALID_EVENT_TYPES.count(event_type))
                    throw std::invalid_argument("Invalid event type: " + event_type);    
                auto event_type_enum = mapEnum(EnumMap::eventType, event_type);
                
                if (pyoptions.world_bloom_event_turn.has_value()) {
                    // liveType非挑战，没有传入eventId时，首先尝试模拟WL组卡
                    if (!pyoptions.event_unit.has_value())
                        throw std::invalid_argument("event_unit is required for world bloom fake event.");
                    if (!VALID_UNIT_TYPES.count(pyoptions.event_unit.value()))
                        throw std::invalid_argument("Invalid event unit: " + pyoptions.event_unit.value());
                    int turn = pyoptions.world_bloom_event_turn.value();
                    if (turn < 1 || turn > 2)
                        throw std::invalid_argument("Invalid world bloom event turn: " + std::to_string(turn));
                    auto unit = mapEnum(EnumMap::unit, pyoptions.event_unit.value());
                    options.eventId = options.dataProvider.masterData->getWorldBloomFakeEventId(turn, unit);

                } else if (pyoptions.event_attr.has_value() || pyoptions.event_unit.has_value()) {
                    // 然后尝试指定团+颜色组卡
                    if (!pyoptions.event_attr.has_value() || !pyoptions.event_unit.has_value())
                        throw std::invalid_argument("event_attr and event_unit must be specified together.");
                    if (!VALID_EVENT_ATTRS.count(pyoptions.event_attr.value()))
                        throw std::invalid_argument("Invalid event attr: " + pyoptions.event_attr.value());
                    if (!VALID_UNIT_TYPES.count(pyoptions.event_unit.value()))
                        throw std::invalid_argument("Invalid event unit: " + pyoptions.event_unit.value());
                    auto unit = mapEnum(EnumMap::unit, pyoptions.event_unit.value());
                    auto attr = mapEnum(EnumMap::attr, pyoptions.event_attr.value());
                    options.eventId = options.dataProvider.masterData->getUnitAttrFakeEventId(event_type_enum, unit, attr);

                } else {
                    // 最后无活动组卡
                    options.eventId = options.dataProvider.masterData->getNoEventFakeEventId(event_type_enum);
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
            if (is_challenge_live)
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

            // target
            if (is_mysekai) {
                // 虽然传参的时候用live类型，但是暂时用target标记Mysekai组卡
                config.target = RecommendTarget::Mysekai;
            } else {
                std::string target = pyoptions.target.value_or(DEFAULT_TARGET);
                if (!VALID_TARGETS.count(target))
                    throw std::invalid_argument("Invalid target: " + target);
                if (target == "score")
                    config.target = RecommendTarget::Score;
                else if (target == "skill")
                    config.target = RecommendTarget::Skill;
                else if (target == "power")
                    config.target = RecommendTarget::Power;
                else if (target == "bonus")
                    config.target = RecommendTarget::Bonus;
            }

            // bonus list for target == bonus
            if (pyoptions.target_bonus_list.value_or(std::vector<int>{}).size()) {
                if (config.target != RecommendTarget::Bonus)
                    throw std::invalid_argument("target_bonus_list is only valid for bonus target.");
                config.bonusList = pyoptions.target_bonus_list.value();
            } else {
                if (config.target == RecommendTarget::Bonus)
                    throw std::invalid_argument("target_bonus_list is required for bonus target.");
                config.bonusList = {};
            }

            // algorithm
            std::string algorithm = pyoptions.algorithm.value_or(DEFAULT_ALGORITHM);
            if (!VALID_ALGORITHMS.count(algorithm))
                throw std::invalid_argument("Invalid algorithm: " + algorithm);
            if (algorithm == "sa")
                config.algorithm = RecommendAlgorithm::SA;
            else if (algorithm == "dfs")
                config.algorithm = RecommendAlgorithm::DFS;
            else if (algorithm == "ga")
                config.algorithm = RecommendAlgorithm::GA;

            // filter other unit
            if (pyoptions.filter_other_unit.has_value()) {
                config.filterOtherUnit = pyoptions.filter_other_unit.value();
            }
            else {
                config.filterOtherUnit = false;
            }

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

            // fixed cards
            if (pyoptions.fixed_cards.has_value()) {
                auto fixed_cards = pyoptions.fixed_cards.value();
                if (int(fixed_cards.size()) > config.member)
                    throw std::invalid_argument("Fixed cards size exceeds member count.");
                for (const auto& card_id : fixed_cards) {
                    findOrThrow(options.dataProvider.masterData->cards, [&](const Card& it) {
                        return it.id == card_id;
                    }, "Invalid fixed card ID: " + std::to_string(card_id));
                }
                config.fixedCards = fixed_cards;
            }

            // fixed characters
            if (pyoptions.fixed_characters.has_value()) {
                auto fixed_characters = pyoptions.fixed_characters.value();
                if (int(fixed_characters.size()) > config.member)
                    throw std::invalid_argument("Fixed characters size exceeds member count.");
                if (config.fixedCards.size()) 
                    throw std::invalid_argument("fixed_characters and fixed_cards cannot be used together.");
                if (is_challenge_live)
                    throw std::invalid_argument("fixed_characters is not valid for challenge live.");
                for (const auto& character_id : fixed_characters) {
                    if (character_id < 1 || character_id > 26)
                        throw std::invalid_argument("Invalid fixed character ID: " + std::to_string(character_id));
                }
                config.fixedCharacters = fixed_characters;
            }

            // skill reference choose strategy
            std::string skill_reference_choose_strategy = pyoptions.skill_reference_choose_strategy.value_or(DEFAULT_SKILL_REFERENCE_CHOOSE_STRATEGY);
            if (!VALID_SKILL_REFERENCE_CHOOSE_STRATEGIES.count(skill_reference_choose_strategy))
                throw std::invalid_argument("Invalid skill reference choose strategy: " + skill_reference_choose_strategy);
            if (skill_reference_choose_strategy == "average")
                config.skillReferenceChooseStrategy = SkillReferenceChooseStrategy::Average;
            else if (skill_reference_choose_strategy == "max")
                config.skillReferenceChooseStrategy = SkillReferenceChooseStrategy::Max;
            else if (skill_reference_choose_strategy == "min")
                config.skillReferenceChooseStrategy = SkillReferenceChooseStrategy::Min;

            // keep after training state
            if (pyoptions.keep_after_training_state.has_value()) {
                config.keepAfterTrainingState = pyoptions.keep_after_training_state.value();
            }

            // multi live teammate score up
            if (pyoptions.multi_live_teammate_score_up.has_value()) {
                config.multiTeammateScoreUp = pyoptions.multi_live_teammate_score_up.value();
                if (!Enums::LiveType::isMulti(options.liveType))
                    throw std::invalid_argument("multi_live_teammate_score_up is only valid for multi live.");
                if (config.multiTeammateScoreUp < 0 || config.multiTeammateScoreUp > 1000)
                    throw std::invalid_argument("Invalid multi live teammate score up: " + std::to_string(config.multiTeammateScoreUp.value()));
            }

            // multi live teammate power
            if (pyoptions.multi_live_teammate_power.has_value()) {
                config.multiTeammatePower = pyoptions.multi_live_teammate_power.value();
                if (!Enums::LiveType::isMulti(options.liveType))
                    throw std::invalid_argument("multi_live_teammate_power is only valid for multi live.");
                if (config.multiTeammatePower < 0 || config.multiTeammatePower > 10000000)
                    throw std::invalid_argument("Invalid multi live teammate power: " + std::to_string(config.multiTeammatePower.value()));
            }

            // best skill as leader
            if (pyoptions.best_skill_as_leader.has_value()) {
                config.bestSkillAsLeader = pyoptions.best_skill_as_leader.value();
            }

            // multi live score up lower bound
            if (pyoptions.multi_live_score_up_lower_bound.has_value()) {
                if (!Enums::LiveType::isMulti(options.liveType))
                    throw std::invalid_argument("multi_live_score_up_lower_bound is only valid for multi live.");
                config.multiScoreUpLowerBound = pyoptions.multi_live_score_up_lower_bound.value();
            }

            // skill order choose strategy
            std::string skill_order_choose_strategy = pyoptions.skill_order_choose_strategy.value_or(DEFAULT_SKILL_ORDER_CHOOSE_STRATEGY);
            if (!VALID_SKILL_ORDER_CHOOSE_STRATEGIES.count(skill_order_choose_strategy))
                throw std::invalid_argument("Invalid skill order choose strategy: " + skill_order_choose_strategy);
            if (skill_order_choose_strategy == "average")
                config.liveSkillOrder = LiveSkillOrder::average;
            else if (skill_order_choose_strategy == "max")
                config.liveSkillOrder = LiveSkillOrder::best;
            else if (skill_order_choose_strategy == "min")
                config.liveSkillOrder = LiveSkillOrder::worst;
            else if (skill_order_choose_strategy == "specific")
                config.liveSkillOrder = LiveSkillOrder::specific;

            // specific skill order
            if (pyoptions.specific_skill_order.has_value()) {
                if (skill_order_choose_strategy != "specific")
                    throw std::invalid_argument("specific_skill_order is only valid when skill_order_choose_strategy is specific.");
                auto specific_skill_order = pyoptions.specific_skill_order.value();
                if (int(specific_skill_order.size()) != config.member)
                    throw std::invalid_argument("specific_skill_order size must equal to member count.");
                for (const auto& skill_index : specific_skill_order) {
                    if (skill_index < 0 || skill_index >= config.member)
                        throw std::invalid_argument("Invalid specific skill order index: " + std::to_string(skill_index));
                }
                config.specificSkillOrder = specific_skill_order;
            } else {
                if (skill_order_choose_strategy == "specific")
                    throw std::invalid_argument("specific_skill_order is required when skill_order_choose_strategy is specific.");
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
                    if (value->canvas.has_value())
                        card_config.canvas = value->canvas.value();
                }
                config.cardConfig[mapEnum(EnumMap::cardRarityType, key)] = card_config;
            }

            // single card config
            if (pyoptions.single_card_configs.has_value()) {
                for (const auto& card_config : pyoptions.single_card_configs.value()) {
                    auto cfg = CardConfig();
                    if (card_config.disable.has_value())
                        cfg.disable = card_config.disable.value();
                    if (card_config.level_max.has_value())
                        cfg.rankMax = card_config.level_max.value();
                    if (card_config.episode_read.has_value())
                        cfg.episodeRead = card_config.episode_read.value();
                    if (card_config.master_max.has_value())
                        cfg.masterMax = card_config.master_max.value();
                    if (card_config.skill_max.has_value())
                        cfg.skillMax = card_config.skill_max.value();
                    if (card_config.canvas.has_value())
                        cfg.canvas = card_config.canvas.value();
                    config.singleCardConfig[card_config.card_id] = cfg;
                }
            }

            // sa config
            if (config.algorithm == RecommendAlgorithm::SA && pyoptions.sa_options.has_value()) {
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

            // ga config
            if (config.algorithm == RecommendAlgorithm::GA && pyoptions.ga_options.has_value()) {
                auto ga_options = pyoptions.ga_options.value();

                if (ga_options.seed.has_value())
                    config.gaSeed = ga_options.seed.value();
                
                if (ga_options.debug.has_value())
                    config.gaDebug = ga_options.debug.value();

                if (ga_options.max_iter.has_value())
                    config.gaMaxIter = ga_options.max_iter.value();
                if (config.gaMaxIter < 1)
                    throw std::invalid_argument("Invalid ga max iter: " + std::to_string(config.gaMaxIter));

                if (ga_options.max_no_improve_iter.has_value())
                    config.gaMaxIterNoImprove = ga_options.max_no_improve_iter.value();
                if (config.gaMaxIterNoImprove < 1)
                    throw std::invalid_argument("Invalid ga max no improve iter: " + std::to_string(config.gaMaxIterNoImprove));

                if (ga_options.pop_size.has_value())
                    config.gaPopSize = ga_options.pop_size.value();
                if (config.gaPopSize < 1)
                    throw std::invalid_argument("Invalid ga pop size: " + std::to_string(config.gaPopSize));

                if (ga_options.parent_size.has_value())
                    config.gaParentSize = ga_options.parent_size.value();
                if (config.gaParentSize < 1 || config.gaParentSize > config.gaPopSize)
                    throw std::invalid_argument("Invalid ga parent size: " + std::to_string(config.gaParentSize));

                if (ga_options.elite_size.has_value())
                    config.gaEliteSize = ga_options.elite_size.value();
                if (config.gaEliteSize < 0 || config.gaEliteSize > config.gaPopSize)
                    throw std::invalid_argument("Invalid ga elite size: " + std::to_string(config.gaEliteSize));

                if (ga_options.crossover_rate.has_value())
                    config.gaCrossoverRate = ga_options.crossover_rate.value();
                if (config.gaCrossoverRate < 0 || config.gaCrossoverRate > 1)
                    throw std::invalid_argument("Invalid ga crossover rate: " + std::to_string(config.gaCrossoverRate));

                if (ga_options.base_mutation_rate.has_value())
                    config.gaBaseMutationRate = ga_options.base_mutation_rate.value();
                if (config.gaBaseMutationRate < 0 || config.gaBaseMutationRate > 1)
                    throw std::invalid_argument("Invalid ga base mutation rate: " + std::to_string(config.gaBaseMutationRate));

                if (ga_options.no_improve_iter_to_mutation_rate.has_value())
                    config.gaNoImproveIterToMutationRate = ga_options.no_improve_iter_to_mutation_rate.value();
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
            py_deck.live_score = deck.liveScore;
            py_deck.mysekai_event_point = deck.mysekaiEventPoint;
            py_deck.total_power = deck.power.total;
            py_deck.base_power = deck.power.base;
            py_deck.area_item_bonus_power = deck.power.areaItemBonus;
            py_deck.character_bonus_power = deck.power.characterBonus;
            py_deck.honor_bonus_power = deck.power.honorBonus;
            py_deck.fixture_bonus_power = deck.power.fixtureBonus;
            py_deck.gate_bonus_power = deck.power.gateBonus;
            py_deck.event_bonus_rate = deck.eventBonus.value_or(0);
            py_deck.support_deck_bonus_rate = deck.supportDeckBonus.value_or(0);
            py_deck.multi_live_score_up = deck.multiLiveScoreUp;

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
                py_card.episode1_read = card.episode1Read;
                py_card.episode2_read = card.episode2Read;
                py_card.after_training = card.afterTraining;
                py_card.default_image = mappedEnumToString(EnumMap::defaultImage, card.defaultImage);
                py_card.has_canvas_bonus = card.hasCanvasBonus;
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
        region_masterdata[REGION_ENUM_MAP.at(region)] = std::make_shared<MasterData>();
        region_masterdata[REGION_ENUM_MAP.at(region)]->loadFromFiles(base_dir);
    }

    // 从指定string的dict更新区服masterdata数据
    void update_masterdata_from_strings(const py::dict& dict, const std::string& region) {
        if (!REGION_ENUM_MAP.count(region)) 
            throw std::invalid_argument("Invalid region: " + region);
        std::map<std::string, std::string> data;
        for (const auto& item : dict) {
            std::string key = item.first.cast<std::string>();
            data[key] = item.second.cast<std::string>();
        }
        region_masterdata[REGION_ENUM_MAP.at(region)] = std::make_shared<MasterData>();
        region_masterdata[REGION_ENUM_MAP.at(region)]->loadFromStrings(data);
    }

    // 从指定文件更新区服musicmetas数据
    void update_musicmetas(const std::string& file_path, const std::string& region) {
        if (!REGION_ENUM_MAP.count(region)) 
            throw std::invalid_argument("Invalid region: " + region);
        region_musicmetas[REGION_ENUM_MAP.at(region)] = std::make_shared<MusicMetas>();
        region_musicmetas[REGION_ENUM_MAP.at(region)]->loadFromFile(file_path);
    }

    // 从指定string更新区服musicmetas数据
    void update_musicmetas_from_string(const std::string& s, const std::string& region) {
        if (!REGION_ENUM_MAP.count(region)) 
            throw std::invalid_argument("Invalid region: " + region);
        region_musicmetas[REGION_ENUM_MAP.at(region)] = std::make_shared<MusicMetas>();
        region_musicmetas[REGION_ENUM_MAP.at(region)]->loadFromString(s);
    }

    // 推荐卡组
    PyDeckRecommendResult recommend(const PyDeckRecommendOptions& pyoptions) {
        auto options = construct_options_from_py(pyoptions);

        std::vector<RecommendDeck> result;
        
        if (options.config.target == RecommendTarget::Mysekai) {
            MysekaiDeckRecommend mysekaiDeckRecommend(options.dataProvider);
            result = mysekaiDeckRecommend.recommendMysekaiDeck(
                options.eventId,
                options.config,
                options.worldBloomCharacterId
            );
        } else if (Enums::LiveType::isChallenge(options.liveType)) {
            ChallengeLiveDeckRecommend challengeLiveDeckRecommend(options.dataProvider);
            result = challengeLiveDeckRecommend.recommendChallengeLiveDeck(
                options.liveType,
                options.challengeLiveCharacterId,
                options.config
            );
        } else {
            EventDeckRecommend eventDeckRecommend(options.dataProvider);
            result = eventDeckRecommend.recommendEventDeck(
                options.eventId,
                options.liveType,
                options.config,
                options.worldBloomCharacterId
            );
        }

        return construct_result_to_py(result);
    }

};


PYBIND11_MODULE(sekai_deck_recommend, m) {
    m.doc() = "pybind11 sekai_deck_recommend plugin";

    m.def("init_data_path", &init_data_path);

    py::class_<PyUserData>(m, "DeckRecommendUserData")
        .def(py::init<>())
        .def(py::init<const PyUserData&>())
        .def("load_from_file", &PyUserData::load_from_file)
        .def("load_from_bytes", &PyUserData::load_from_bytes);

    py::class_<PyCardConfig>(m, "DeckRecommendCardConfig")
        .def(py::init<>())
        .def(py::init<const PyCardConfig&>())
        .def("to_dict", &PyCardConfig::to_dict)
        .def_static("from_dict", &PyCardConfig::from_dict)
        .def_readwrite("disable", &PyCardConfig::disable)
        .def_readwrite("level_max", &PyCardConfig::level_max)
        .def_readwrite("episode_read", &PyCardConfig::episode_read)
        .def_readwrite("master_max", &PyCardConfig::master_max)
        .def_readwrite("skill_max", &PyCardConfig::skill_max)
        .def_readwrite("canvas", &PyCardConfig::canvas);

    py::class_<PySingleCardConfig>(m, "DeckRecommendSingleCardConfig")
        .def(py::init<>())
        .def(py::init<const PySingleCardConfig&>())
        .def("to_dict", &PySingleCardConfig::to_dict)
        .def_static("from_dict", &PySingleCardConfig::from_dict)
        .def_readwrite("card_id", &PySingleCardConfig::card_id)
        .def_readwrite("disable", &PySingleCardConfig::disable)
        .def_readwrite("level_max", &PySingleCardConfig::level_max)
        .def_readwrite("episode_read", &PySingleCardConfig::episode_read)
        .def_readwrite("master_max", &PySingleCardConfig::master_max)
        .def_readwrite("skill_max", &PySingleCardConfig::skill_max)
        .def_readwrite("canvas", &PySingleCardConfig::canvas);

    py::class_<PySaOptions>(m, "DeckRecommendSaOptions")
        .def(py::init<>())
        .def(py::init<const PySaOptions&>())
        .def("to_dict", &PySaOptions::to_dict)
        .def_static("from_dict", &PySaOptions::from_dict)
        .def_readwrite("run_num", &PySaOptions::run_num)
        .def_readwrite("seed", &PySaOptions::seed)
        .def_readwrite("max_iter", &PySaOptions::max_iter)
        .def_readwrite("max_no_improve_iter", &PySaOptions::max_no_improve_iter)
        .def_readwrite("time_limit_ms", &PySaOptions::time_limit_ms)
        .def_readwrite("start_temprature", &PySaOptions::start_temprature)
        .def_readwrite("cooling_rate", &PySaOptions::cooling_rate)
        .def_readwrite("debug", &PySaOptions::debug);

    py::class_<PyGaOptions>(m, "DeckRecommendGaOptions")
        .def(py::init<>())
        .def(py::init<const PyGaOptions&>())
        .def("to_dict", &PyGaOptions::to_dict)
        .def_static("from_dict", &PyGaOptions::from_dict)
        .def_readwrite("seed", &PyGaOptions::seed)
        .def_readwrite("debug", &PyGaOptions::debug)
        .def_readwrite("max_iter", &PyGaOptions::max_iter)
        .def_readwrite("max_no_improve_iter", &PyGaOptions::max_no_improve_iter)
        .def_readwrite("pop_size", &PyGaOptions::pop_size)
        .def_readwrite("parent_size", &PyGaOptions::parent_size)
        .def_readwrite("elite_size", &PyGaOptions::elite_size)
        .def_readwrite("crossover_rate", &PyGaOptions::crossover_rate)
        .def_readwrite("base_mutation_rate", &PyGaOptions::base_mutation_rate)
        .def_readwrite("no_improve_iter_to_mutation_rate", &PyGaOptions::no_improve_iter_to_mutation_rate);
    
    py::class_<PyDeckRecommendOptions>(m, "DeckRecommendOptions")
        .def(py::init<>())
        .def(py::init<const PyDeckRecommendOptions&>())
        .def("to_dict", &PyDeckRecommendOptions::to_dict)
        .def_static("from_dict", &PyDeckRecommendOptions::from_dict)
        .def_readwrite("target", &PyDeckRecommendOptions::target)
        .def_readwrite("algorithm", &PyDeckRecommendOptions::algorithm)
        .def_readwrite("region", &PyDeckRecommendOptions::region)
        .def_readwrite("user_data_file_path", &PyDeckRecommendOptions::user_data_file_path)
        .def_readwrite("user_data_str", &PyDeckRecommendOptions::user_data_str)
        .def_readwrite("user_data", &PyDeckRecommendOptions::user_data)
        .def_readwrite("live_type", &PyDeckRecommendOptions::live_type)
        .def_readwrite("music_id", &PyDeckRecommendOptions::music_id)
        .def_readwrite("music_diff", &PyDeckRecommendOptions::music_diff)
        .def_readwrite("event_id", &PyDeckRecommendOptions::event_id)
        .def_readwrite("event_attr", &PyDeckRecommendOptions::event_attr)
        .def_readwrite("event_unit", &PyDeckRecommendOptions::event_unit)
        .def_readwrite("event_type", &PyDeckRecommendOptions::event_type)
        .def_readwrite("world_bloom_event_turn", &PyDeckRecommendOptions::world_bloom_event_turn)
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
        .def_readwrite("single_card_configs", &PyDeckRecommendOptions::single_card_configs)
        .def_readwrite("filter_other_unit", &PyDeckRecommendOptions::filter_other_unit)
        .def_readwrite("fixed_cards", &PyDeckRecommendOptions::fixed_cards)
        .def_readwrite("fixed_characters", &PyDeckRecommendOptions::fixed_characters)
        .def_readwrite("target_bonus_list", &PyDeckRecommendOptions::target_bonus_list)
        .def_readwrite("skill_reference_choose_strategy", &PyDeckRecommendOptions::skill_reference_choose_strategy)
        .def_readwrite("keep_after_training_state", &PyDeckRecommendOptions::keep_after_training_state)
        .def_readwrite("multi_live_teammate_score_up", &PyDeckRecommendOptions::multi_live_teammate_score_up)
        .def_readwrite("multi_live_teammate_power", &PyDeckRecommendOptions::multi_live_teammate_power)
        .def_readwrite("best_skill_as_leader", &PyDeckRecommendOptions::best_skill_as_leader)
        .def_readwrite("multi_live_score_up_lower_bound", &PyDeckRecommendOptions::multi_live_score_up_lower_bound)
        .def_readwrite("skill_order_choose_strategy", &PyDeckRecommendOptions::skill_order_choose_strategy)
        .def_readwrite("specific_skill_order", &PyDeckRecommendOptions::specific_skill_order)
        .def_readwrite("sa_options", &PyDeckRecommendOptions::sa_options)
        .def_readwrite("ga_options", &PyDeckRecommendOptions::ga_options);

    py::class_<PyRecommendCard>(m, "RecommendCard")
        .def(py::init<>())
        .def(py::init<const PyRecommendCard&>())
        .def("to_dict", &PyRecommendCard::to_dict)
        .def_static("from_dict", &PyRecommendCard::from_dict)
        .def_readwrite("card_id", &PyRecommendCard::card_id)
        .def_readwrite("total_power", &PyRecommendCard::total_power)
        .def_readwrite("base_power", &PyRecommendCard::base_power)
        .def_readwrite("event_bonus_rate", &PyRecommendCard::event_bonus_rate)
        .def_readwrite("master_rank", &PyRecommendCard::master_rank)
        .def_readwrite("level", &PyRecommendCard::level)
        .def_readwrite("skill_level", &PyRecommendCard::skill_level)
        .def_readwrite("skill_score_up", &PyRecommendCard::skill_score_up)
        .def_readwrite("skill_life_recovery", &PyRecommendCard::skill_life_recovery)
        .def_readwrite("episode1_read", &PyRecommendCard::episode1_read)
        .def_readwrite("episode2_read", &PyRecommendCard::episode2_read)
        .def_readwrite("after_training", &PyRecommendCard::after_training)
        .def_readwrite("default_image", &PyRecommendCard::default_image)
        .def_readwrite("has_canvas_bonus", &PyRecommendCard::has_canvas_bonus);

    py::class_<PyRecommendDeck>(m, "RecommendDeck")
        .def(py::init<>())
        .def(py::init<const PyRecommendDeck&>())
        .def("to_dict", &PyRecommendDeck::to_dict)
        .def_static("from_dict", &PyRecommendDeck::from_dict)
        .def_readwrite("score", &PyRecommendDeck::score)
        .def_readwrite("live_score", &PyRecommendDeck::live_score)
        .def_readwrite("mysekai_event_point", &PyRecommendDeck::mysekai_event_point)
        .def_readwrite("total_power", &PyRecommendDeck::total_power)
        .def_readwrite("base_power", &PyRecommendDeck::base_power)
        .def_readwrite("area_item_bonus_power", &PyRecommendDeck::area_item_bonus_power)
        .def_readwrite("character_bonus_power", &PyRecommendDeck::character_bonus_power)
        .def_readwrite("honor_bonus_power", &PyRecommendDeck::honor_bonus_power)
        .def_readwrite("fixture_bonus_power", &PyRecommendDeck::fixture_bonus_power)
        .def_readwrite("gate_bonus_power", &PyRecommendDeck::gate_bonus_power)
        .def_readwrite("event_bonus_rate", &PyRecommendDeck::event_bonus_rate)
        .def_readwrite("support_deck_bonus_rate", &PyRecommendDeck::support_deck_bonus_rate)
        .def_readwrite("multi_live_score_up", &PyRecommendDeck::multi_live_score_up)
        .def_readwrite("cards", &PyRecommendDeck::cards);
    
    py::class_<PyDeckRecommendResult>(m, "DeckRecommendResult")
        .def(py::init<>())
        .def(py::init<const PyDeckRecommendResult&>())
        .def("to_dict", &PyDeckRecommendResult::to_dict)
        .def_static("from_dict", &PyDeckRecommendResult::from_dict)
        .def_readwrite("decks", &PyDeckRecommendResult::decks);

    py::class_<SekaiDeckRecommend>(m, "SekaiDeckRecommend")
        .def(py::init<>())
        .def(py::init<const SekaiDeckRecommend&>())
        .def("update_masterdata", &SekaiDeckRecommend::update_masterdata)
        .def("update_masterdata_from_strings", &SekaiDeckRecommend::update_masterdata_from_strings)
        .def("update_musicmetas", &SekaiDeckRecommend::update_musicmetas)
        .def("update_musicmetas_from_string", &SekaiDeckRecommend::update_musicmetas_from_string)
        .def("recommend", &SekaiDeckRecommend::recommend);
}

