#ifndef MUSIC_VOCAL_H
#define MUSIC_VOCAL_H

#include "common/collection-utils.h"

struct MusicVocalCharacter {
    int id = 0;
    int musicId = 0;
    int musicVocalId = 0;
    int characterType = 0;
    int characterId = 0;
    int seq = 0;

    static inline std::vector<MusicVocalCharacter> fromJsonList(const json& jsonData) {
        std::vector<MusicVocalCharacter> characters;
        for (const auto& item : jsonData) {
            MusicVocalCharacter character;
            character.id = item.value("id", 0);
            character.musicId = item.value("musicId", 0);
            character.musicVocalId = item.value("musicVocalId", 0);
            character.characterType = mapEnum(EnumMap::characterType, item.value("characterType", ""));
            character.characterId = item.value("characterId", 0);
            character.seq = item.value("seq", 0);
            characters.push_back(character);
        }
        return characters;
    }
};

struct MusicVocal {
    int id = 0;
    int musicId = 0;
    int musicVocalType = 0;
    int seq = 0;
    int releaseConditionId = 0;
    std::vector<MusicVocalCharacter> characters;

    static inline std::vector<MusicVocal> fromJsonList(const json& jsonData) {
        std::vector<MusicVocal> musicVocals;
        for (const auto& item : jsonData) {
            MusicVocal musicVocal;
            musicVocal.id = item.value("id", 0);
            musicVocal.musicId = item.value("musicId", 0);
            musicVocal.musicVocalType = mapEnum(EnumMap::musicVocalType, item.value("musicVocalType", ""));
            musicVocal.seq = item.value("seq", 0);
            musicVocal.releaseConditionId = item.value("releaseConditionId", 0);
            musicVocal.characters = MusicVocalCharacter::fromJsonList(item.value("characters", json::array()));
            musicVocals.push_back(musicVocal);
        }
        return musicVocals;
    }
};

#endif