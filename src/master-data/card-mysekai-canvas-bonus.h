#ifndef CARD_MYSEKAI_CANVAS_BONUS_H
#define CARD_MYSEKAI_CANVAS_BONUS_H

#include "common/collection-utils.h"

struct CardMysekaiCanvasBonus {
    int id = 0;
    int cardRarityType = 0;
    int power1BonusFixed = 0;
    int power2BonusFixed = 0;
    int power3BonusFixed = 0;

    static inline std::vector<CardMysekaiCanvasBonus> fromJsonList(const json& jsonData) {
        std::vector<CardMysekaiCanvasBonus> bonuses;
        for (const auto& item : jsonData) {
            CardMysekaiCanvasBonus bonus;
            bonus.id = item.value("id", 0);
            bonus.cardRarityType = mapEnum(EnumMap::cardRarityType, item.value("cardRarityType", ""));
            bonus.power1BonusFixed = item.value("power1BonusFixed", 0);
            bonus.power2BonusFixed = item.value("power2BonusFixed", 0);
            bonus.power3BonusFixed = item.value("power3BonusFixed", 0);
            bonuses.push_back(bonus);
        }
        return bonuses;
    }
};

#endif // CARD_MYSEKAI_CANVAS_BONUS_H