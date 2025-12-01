#ifndef MUSIC_DIFFICULTY_H
#define MUSIC_DIFFICULTY_H

#include "common/collection-utils.h"

struct MusicDifficulty {
    int id;
    int musicId;
    int musicDifficulty;
    int playLevel;
    int releaseConditionId;
    int totalNoteCount;

    static inline std::vector<MusicDifficulty> fromJsonList(const json& jsonData) {
        std::vector<MusicDifficulty> musicDifficulties;
        for (const auto& item : jsonData) {
            MusicDifficulty musicDifficulty;
            musicDifficulty.id = item.value("id", 0);
            musicDifficulty.musicId = item.value("musicId", 0);
            musicDifficulty.musicDifficulty = mapEnum(EnumMap::musicDifficulty, item.value("musicDifficulty", ""));
            musicDifficulty.playLevel = item.value("playLevel", 0);
            musicDifficulty.releaseConditionId = item.value("releaseConditionId", 0);
            musicDifficulty.totalNoteCount = item.value("totalNoteCount", 0);
            musicDifficulties.push_back(musicDifficulty);
        }
        return musicDifficulties;
    }
};


#endif  // MUSIC_DIFFICULTY_H