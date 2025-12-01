#ifndef GAME_CHARACTER_UNIT_H
#define GAME_CHARACTER_UNIT_H

#include "common/collection-utils.h"

struct GameCharacterUnit {
    int id = 0;
    int gameCharacterId = 0;
    int unit = 0;

    static inline std::vector<GameCharacterUnit> fromJsonList(const json& jsonData) {
        std::vector<GameCharacterUnit> gameCharacterUnits;
        for (const auto& item : jsonData) {
            GameCharacterUnit gameCharacterUnit;
            gameCharacterUnit.id = item.value("id", 0);
            gameCharacterUnit.gameCharacterId = item.value("gameCharacterId", 0);
            gameCharacterUnit.unit = mapEnum(EnumMap::unit, item.value("unit", ""));
            gameCharacterUnits.push_back(gameCharacterUnit);
        }
        return gameCharacterUnits;
    }
};

#endif // GAME_CHARACTER_UNIT_H

