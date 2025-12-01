#ifndef WORLD_BLOOM_SUPPORT_DECK_BONUS_H
#define WORLD_BLOOM_SUPPORT_DECK_BONUS_H

#include "common/collection-utils.h"


struct WorldBloomSupportDeckCharacterBonus {
    int id = 0;
    int worldBloomSupportDeckCharacterType = 0;
    double bonusRate = 0.0;

    static inline std::vector<WorldBloomSupportDeckCharacterBonus> fromJsonList(const json& jsonData) {
        std::vector<WorldBloomSupportDeckCharacterBonus> bonuses;
        for (const auto& item : jsonData) {
            WorldBloomSupportDeckCharacterBonus bonus;
            bonus.id = item.value("id", 0);
            bonus.worldBloomSupportDeckCharacterType = mapEnum(EnumMap::worldBloomSupportDeckCharacterType, item.value("worldBloomSupportDeckCharacterType", ""));
            bonus.bonusRate = item.value("bonusRate", 0.0);
            bonuses.push_back(bonus);
        }
        return bonuses;
    }
};


struct WorldBloomSupportDeckMasterRankBonus {
    int id = 0;
    int masterRank = 0;
    double bonusRate = 0.0;

    static inline std::vector<WorldBloomSupportDeckMasterRankBonus> fromJsonList(const json& jsonData) {
        std::vector<WorldBloomSupportDeckMasterRankBonus> bonuses;
        for (const auto& item : jsonData) {
            WorldBloomSupportDeckMasterRankBonus bonus;
            bonus.id = item.value("id", 0);
            bonus.masterRank = item.value("masterRank", 0);
            bonus.bonusRate = item.value("bonusRate", 0.0);
            bonuses.push_back(bonus);
        }
        return bonuses;
    }
};


struct WorldBloomSupportDeckSkillLevelBonus {
    int id = 0;
    int skillLevel = 0;
    double bonusRate = 0.0;

    static inline std::vector<WorldBloomSupportDeckSkillLevelBonus> fromJsonList(const json& jsonData) {
        std::vector<WorldBloomSupportDeckSkillLevelBonus> bonuses;
        for (const auto& item : jsonData) {
            WorldBloomSupportDeckSkillLevelBonus bonus;
            bonus.id = item.value("id", 0);
            bonus.skillLevel = item.value("skillLevel", 0);
            bonus.bonusRate = item.value("bonusRate", 0.0);
            bonuses.push_back(bonus);
        }
        return bonuses;
    }
};


struct WorldBloomSupportDeckBonus {
    int cardRarityType = 0;
    std::vector<WorldBloomSupportDeckCharacterBonus> worldBloomSupportDeckCharacterBonuses;
    std::vector<WorldBloomSupportDeckMasterRankBonus> worldBloomSupportDeckMasterRankBonuses;
    std::vector<WorldBloomSupportDeckSkillLevelBonus> worldBloomSupportDeckSkillLevelBonuses;

    static inline std::vector<WorldBloomSupportDeckBonus> fromJsonList(const json& jsonData) {
        std::vector<WorldBloomSupportDeckBonus> bonuses;
        for (const auto& item : jsonData) {
            WorldBloomSupportDeckBonus bonus;
            bonus.cardRarityType = mapEnum(EnumMap::cardRarityType, item.value("cardRarityType", ""));
            bonus.worldBloomSupportDeckCharacterBonuses = WorldBloomSupportDeckCharacterBonus::fromJsonList(item.value("worldBloomSupportDeckCharacterBonuses", json::array()));
            bonus.worldBloomSupportDeckMasterRankBonuses = WorldBloomSupportDeckMasterRankBonus::fromJsonList(item.value("worldBloomSupportDeckMasterRankBonuses", json::array()));
            bonus.worldBloomSupportDeckSkillLevelBonuses = WorldBloomSupportDeckSkillLevelBonus::fromJsonList(item.value("worldBloomSupportDeckSkillLevelBonuses", json::array()));
            bonuses.push_back(bonus);
        }
        return bonuses;
    }
};

#endif // WORLD_BLOOM_SUPPORT_DECK_BONUS_H