#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include "common/collection-utils.h"

/*
export interface GameCharacter {
  id: number
  seq: number
  resourceId: number
  unit: string
  supportUnitType: string
}
*/

struct GameCharacter {
    int id = 0;
    int seq = 0;
    int resourceId = 0;
    int unit = 0;
    int supportUnitType = 0;

    inline static std::vector<GameCharacter> fromJsonList(const json& jsonData) {
        std::vector<GameCharacter> gameCharacters;
        for (const auto& item : jsonData) {
            GameCharacter gameCharacter;
            gameCharacter.id = item.value("id", 0);
            gameCharacter.seq = item.value("seq", 0);
            gameCharacter.resourceId = item.value("resourceId", 0);
            gameCharacter.unit = mapEnum(EnumMap::unit, item.value("unit", ""));
            gameCharacter.supportUnitType = mapEnum(EnumMap::supportUnitType, item.value("supportUnitType", ""));
            gameCharacters.push_back(gameCharacter);
        }
        return gameCharacters;
    }
};

#endif // GAME_CHARACTER_H