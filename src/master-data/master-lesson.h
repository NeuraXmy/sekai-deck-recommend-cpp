#ifndef MASTER_LESSON_H
#define MASTER_LESSON_H

#include "common/collection-utils.h"


struct MasterLesson {
    int cardRarityType = 0;
    int masterRank = 0;
    int power1BonusFixed = 0;
    int power2BonusFixed = 0;
    int power3BonusFixed = 0;

    static inline std::vector<MasterLesson> fromJsonList(const json& jsonData) {
        std::vector<MasterLesson> masterLessons;
        for (const auto& item : jsonData) {
            MasterLesson masterLesson;
            masterLesson.cardRarityType = mapEnum(EnumMap::cardRarityType, item.value("cardRarityType", ""));
            masterLesson.masterRank = item.value("masterRank", 0);
            masterLesson.power1BonusFixed = item.value("power1BonusFixed", 0);
            masterLesson.power2BonusFixed = item.value("power2BonusFixed", 0);
            masterLesson.power3BonusFixed = item.value("power3BonusFixed", 0);
            masterLessons.push_back(masterLesson);
        }
        return masterLessons;
    }
};


#endif  // MASTER_LESSON_H