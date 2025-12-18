#ifndef COMMON_ENUMS_H
#define COMMON_ENUMS_H

#include "common/enum-maps.h"

namespace Enums {
    namespace LiveType {
        const int multi_live = mapEnum(EnumMap::liveType, "multi");
        const int cheerful_live = mapEnum(EnumMap::liveType, "cheerful");
        const int solo_live = mapEnum(EnumMap::liveType, "solo");
        const int auto_live = mapEnum(EnumMap::liveType, "auto");
        const int challenge_live = mapEnum(EnumMap::liveType, "challenge");
        const int challenge_auto_live = mapEnum(EnumMap::liveType, "challenge_auto");

        inline bool isMulti(int liveType) {
            return liveType == multi_live || liveType == cheerful_live;
        }
        inline bool isAuto(int liveType) {
            return liveType == auto_live || liveType == challenge_auto_live;
        }
        inline bool isChallenge(int liveType) {
            return liveType == challenge_live || liveType == challenge_auto_live;
        }
    }

    namespace Unit {
        const int none = mapEnum(EnumMap::unit, "none");
        const int any = mapEnum(EnumMap::unit, "any");
        const int diff = mapEnum(EnumMap::unit, "diff");
        const int ref = mapEnum(EnumMap::unit, "ref");

        const int light_sound = mapEnum(EnumMap::unit, "light_sound");
        const int idol = mapEnum(EnumMap::unit, "idol");
        const int street = mapEnum(EnumMap::unit, "street");
        const int theme_park = mapEnum(EnumMap::unit, "theme_park");
        const int school_refusal = mapEnum(EnumMap::unit, "school_refusal");
        const int piapro = mapEnum(EnumMap::unit, "piapro");

        const std::array<int, 6> specificUnits = {
            light_sound,
            idol,
            street,
            theme_park,
            school_refusal,
            piapro
        };
    }

    namespace Attr {
        const int null = mapEnum(EnumMap::attr, "");
        const int any = mapEnum(EnumMap::attr, "any");

        const int mysterious = mapEnum(EnumMap::attr, "mysterious");
        const int cool = mapEnum(EnumMap::attr, "cool");
        const int pure = mapEnum(EnumMap::attr, "pure");
        const int cute = mapEnum(EnumMap::attr, "cute");
        const int happy = mapEnum(EnumMap::attr, "happy");

        const std::array<int, 5> specificAttrs = {
            mysterious,
            cool,
            pure,
            cute,
            happy
        };
    }

    namespace CardParameterType {
        const int param1 = mapEnum(EnumMap::cardParameterType, "param1");
        const int param2 = mapEnum(EnumMap::cardParameterType, "param2");
        const int param3 = mapEnum(EnumMap::cardParameterType, "param3");
    }

    namespace SpecialTrainingStatus {
        const int done = mapEnum(EnumMap::specialTrainingStatus, "done");
        const int not_doing = mapEnum(EnumMap::specialTrainingStatus, "not_doing");
    }

    namespace DefaultImage {
        const int original = mapEnum(EnumMap::defaultImage, "original");
        const int special_training = mapEnum(EnumMap::defaultImage, "special_training");
    }

    namespace ScenarioStatus {
        const int already_read = mapEnum(EnumMap::scenarioStatus, "already_read");
    }

    namespace SkillEffectType {
        const int score_up = mapEnum(EnumMap::skillEffectType, "score_up");
        const int score_up_condition_life = mapEnum(EnumMap::skillEffectType, "score_up_condition_life");
        const int score_up_keep = mapEnum(EnumMap::skillEffectType, "score_up_keep");
        const int life_recovery = mapEnum(EnumMap::skillEffectType, "life_recovery");
        const int score_up_character_rank = mapEnum(EnumMap::skillEffectType, "score_up_character_rank");
        const int other_member_score_up_reference_rate = mapEnum(EnumMap::skillEffectType, "other_member_score_up_reference_rate");
        const int score_up_unit_count = mapEnum(EnumMap::skillEffectType, "score_up_unit_count");
    }

    namespace EventType {
        const int marathon = mapEnum(EnumMap::eventType, "marathon");
        const int cheerful = mapEnum(EnumMap::eventType, "cheerful_carnival");
        const int world_bloom = mapEnum(EnumMap::eventType, "world_bloom");
    }   

    namespace CardRarityType {
        const int rarity_4 = mapEnum(EnumMap::cardRarityType, "rarity_4");
        const int rarity_3 = mapEnum(EnumMap::cardRarityType, "rarity_3");
        const int rarity_2 = mapEnum(EnumMap::cardRarityType, "rarity_2");
        const int rarity_1 = mapEnum(EnumMap::cardRarityType, "rarity_1");
        const int rarity_birthday = mapEnum(EnumMap::cardRarityType, "rarity_birthday");
    }

    namespace HonorRarity {
        const int high = mapEnum(EnumMap::honorRarity, "high");
        const int highest = mapEnum(EnumMap::honorRarity, "highest");
    }

    namespace WorldBloomSupportDeckCharacterType {
        const int specific = mapEnum(EnumMap::worldBloomSupportDeckCharacterType, "specific");
        const int others = mapEnum(EnumMap::worldBloomSupportDeckCharacterType, "others");
    }
}



#endif // COMMON_ENUMS_H