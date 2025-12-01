#ifndef MUSIC_META_H
#define MUSIC_META_H

#include "common/collection-utils.h"

struct MusicMeta {
    int music_id;
    int difficulty;
    double music_time;
    double event_rate;
    double base_score;
    double base_score_auto;
    std::vector<double> skill_score_solo;
    std::vector<double> skill_score_auto;
    std::vector<double> skill_score_multi;
    double fever_score;
    double fever_end_time;
    int tap_count;

    static inline std::vector<MusicMeta> fromJsonList(const json& jsonData) {
        std::vector<MusicMeta> musicMetas;
        for (const auto& item : jsonData) {
            MusicMeta musicMeta;
            musicMeta.music_id = item.value("music_id", 0);
            musicMeta.difficulty = mapEnum(EnumMap::musicDifficulty, item.value("difficulty", ""));
            musicMeta.music_time = item.value("music_time", 0.0);
            musicMeta.event_rate = item.value("event_rate", 0.0);
            musicMeta.base_score = item.value("base_score", 0.0);
            musicMeta.base_score_auto = item.value("base_score_auto", 0.0);
            musicMeta.skill_score_solo = item.value("skill_score_solo", std::vector<double>());
            musicMeta.skill_score_auto = item.value("skill_score_auto", std::vector<double>());
            musicMeta.skill_score_multi = item.value("skill_score_multi", std::vector<double>());
            musicMeta.fever_score = item.value("fever_score", 0.0);
            musicMeta.fever_end_time = item.value("fever_end_time", 0.0);
            musicMeta.tap_count = item.value("tap_count", 0);
            musicMetas.push_back(musicMeta);
        }
        return musicMetas;
    }
};

#endif // MUSIC_META_H