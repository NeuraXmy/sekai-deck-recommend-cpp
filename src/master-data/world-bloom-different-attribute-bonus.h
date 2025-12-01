#ifndef WORLD_BLOOM_DIFFERENT_ATTRIBUTE_BONUS_H
#define WORLD_BLOOM_DIFFERENT_ATTRIBUTE_BONUS_H

#include "common/collection-utils.h"

struct WorldBloomDifferentAttributeBonus {
    int attributeCount = 0;
    double bonusRate = 0.0;

    static inline std::vector<WorldBloomDifferentAttributeBonus> fromJsonList(const json& jsonData) {
        std::vector<WorldBloomDifferentAttributeBonus> worldBloomDifferentAttributeBonuses;
        for (const auto& item : jsonData) {
            WorldBloomDifferentAttributeBonus worldBloomDifferentAttributeBonus;
            worldBloomDifferentAttributeBonus.attributeCount = item.value("attributeCount", 0);
            worldBloomDifferentAttributeBonus.bonusRate = item.value("bonusRate", 0.0);
            worldBloomDifferentAttributeBonuses.push_back(worldBloomDifferentAttributeBonus);
        }
        return worldBloomDifferentAttributeBonuses;
    }
};

#endif // WORLD_BLOOM_DIFFERENT_ATTRIBUTE_BONUS_H