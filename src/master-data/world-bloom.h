#ifndef WORLD_BLOOM_H
#define WORLD_BLOOM_H

#include "common/collection-utils.h"

struct WorldBloom {
    int id = 0;
    int eventId = 0;
    int gameCharacterId = 0;
    int chapterNo = 0;
    int chapterStartAt = 0;
    int aggregateAt = 0;
    int chapterEndAt = 0;
    int costume2dId = 0;
    bool isSupplemental = false;

    static inline std::vector<WorldBloom> fromJsonList(const json& jsonData) {
        std::vector<WorldBloom> worldBlooms;
        for (const auto& item : jsonData) {
            WorldBloom worldBloom;
            worldBloom.id = item.value("id", 0);
            worldBloom.eventId = item.value("eventId", 0);
            worldBloom.gameCharacterId = item.value("gameCharacterId", 0);
            worldBloom.chapterNo = item.value("chapterNo", 0);
            worldBloom.chapterStartAt = item.value("chapterStartAt", 0);
            worldBloom.aggregateAt = item.value("aggregateAt", 0);
            worldBloom.chapterEndAt = item.value("chapterEndAt", 0);
            worldBloom.costume2dId = item.value("costume2dId", 0);
            worldBloom.isSupplemental = item.value("isSupplemental", false);
            worldBlooms.push_back(worldBloom);
        }
        return worldBlooms;
    }
};

#endif // WORLD_BLOOM_H