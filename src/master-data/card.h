#ifndef CARD_H
#define CARD_H

#include "common/collection-utils.h"

struct CardParameter {
    int cardLevel = 0;
    int cardParameterType = 0;
    int power = 0;

    static inline std::vector<CardParameter> fromJsonList(const json& jsonData) {
        std::vector<CardParameter> cardParameters;
        for (const auto& item : jsonData) {
            CardParameter cardParameter;
            cardParameter.cardLevel = item.value("cardLevel", 0);
            cardParameter.cardParameterType = mapEnum(EnumMap::cardParameterType, item.value("cardParameterType", ""));
            cardParameter.power = item.value("power", 0);
            cardParameters.push_back(cardParameter);
        }
        return cardParameters;
    }
};

struct Card {
    int id = 0;
    int seq = 0;
    int characterId = 0;
    int cardRarityType;
    int specialTrainingPower1BonusFixed = 0;
    int specialTrainingPower2BonusFixed = 0;
    int specialTrainingPower3BonusFixed = 0;
    int attr;
    int supportUnit;
    int skillId = 0;
    int specialTrainingSkillId = 0;
    std::vector<CardParameter> cardParameters;

    static inline std::vector<Card> fromJsonList(const json& jsonData) {
        std::vector<Card> cards;
        for (const auto& item : jsonData) {
            Card card;
            card.id = item.value("id", 0);
            card.seq = item.value("seq", 0);
            card.characterId = item.value("characterId", 0);
            card.cardRarityType = mapEnum(EnumMap::cardRarityType, item.value("cardRarityType", ""));
            card.specialTrainingPower1BonusFixed = item.value("specialTrainingPower1BonusFixed", 0);
            card.specialTrainingPower2BonusFixed = item.value("specialTrainingPower2BonusFixed", 0);
            card.specialTrainingPower3BonusFixed = item.value("specialTrainingPower3BonusFixed", 0);
            card.attr = mapEnum(EnumMap::attr, item.value("attr", ""));
            card.supportUnit = mapEnum(EnumMap::unit, item.value("supportUnit", ""));
            card.skillId = item.value("skillId", 0);
            card.specialTrainingSkillId = item.value("specialTrainingSkillId", 0);
            
            if (item["cardParameters"].is_array()) {
                // 日服格式 card param
                card.cardParameters = CardParameter::fromJsonList(item["cardParameters"]);
            } else {
                // 国服格式 card param 
                const std::vector<std::string> keys = { "param1", "param2", "param3" };
                for (const auto& key : keys) {
                    for (int i = 0; i < (int)item["cardParameters"][key].size(); i++) {
                        CardParameter cardParameter;
                        cardParameter.cardLevel = i + 1;
                        cardParameter.cardParameterType = mapEnum(EnumMap::cardParameterType, key);
                        cardParameter.power = item["cardParameters"][key][i];
                        card.cardParameters.push_back(cardParameter);
                    }
                }
            }
            cards.push_back(card);
        }
        return cards;
    }
};

#endif // CARD_H