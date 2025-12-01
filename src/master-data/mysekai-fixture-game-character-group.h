#ifndef MYSEKAI_FIXTURE_GAME_CHARACTER_GROUP_H
#define MYSEKAI_FIXTURE_GAME_CHARACTER_GROUP_H

#include "common/collection-utils.h"

struct MysekaiFixtureGameCharacterGroup {
    int id;
    int groupId;
    int gameCharacterId;

    static inline std::vector<MysekaiFixtureGameCharacterGroup> fromJsonList(const json& jsonData) {
        std::vector<MysekaiFixtureGameCharacterGroup> groups;
        for (const auto& item : jsonData) {
            MysekaiFixtureGameCharacterGroup group;
            group.id = item.value("id", 0);
            group.groupId = item.value("groupId", 0);
            group.gameCharacterId = item.value("gameCharacterId", 0);
            groups.push_back(group);
        }
        return groups;
    }
};

#endif