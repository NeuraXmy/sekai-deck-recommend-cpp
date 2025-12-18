#include "deck-information/deck-calculator.h"
#include "common/timer.h"
#include "deck-calculator.h"


DeckBonusInfo DeckCalculator::getDeckBonus(
    const std::vector<const CardDetail *> &deckCards, 
    std::optional<int> eventType,
    std::optional<int> eventId
) 
{
    DeckBonusInfo ret{};

    // 如果没有预处理好活动加成，则返回空
    for (const auto &card : deckCards) 
        if (!card->maxEventBonus.has_value()) {
            ret.cardBonus = std::vector<double>(deckCards.size(), 0.0);
            return ret;
        }

    // 正常加成
    ret.cardBonus.reserve(deckCards.size());
    for (const auto &card : deckCards) {
        ret.cardBonus.push_back(card->maxEventBonus.value());
    }

    // 终章机制
    if (eventId.has_value() && eventId.value() == finalChapterEventId) {
        // 不是队长的角色扣掉1k牌加成和队长当期加成
        for (int i = 1; i < (int)deckCards.size(); i++) {
            ret.cardBonus[i] -= deckCards[i]->leaderHonorEventBonus.value_or(0.0);
            ret.cardBonus[i] -= deckCards[i]->leaderLimitEventBonus.value_or(0.0);
        }
        // 最多生效4个当期
        int limitedEventBonusNum = 0;
        for (int i = 0; i < (int)deckCards.size(); i++) {
            if (deckCards[i]->limitedEventBonus.value_or(0.) > 0) {
                if(++limitedEventBonusNum == 5) {
                    // 去掉最后一个当期加成
                    ret.cardBonus[i] -= deckCards[i]->limitedEventBonus.value();
                    break;
                }
            }
        }
    }

    // WL异色加成
    if (eventType == Enums::EventType::world_bloom) 
    {
        auto& worldBloomDifferentAttributeBonuses = this->dataProvider.masterData->worldBloomDifferentAttributeBonuses;
        bool attr_vis[10] = {};
        for (const auto &card : deckCards) 
            attr_vis[card->attr] = true;
        int attr_count = 0;
        for (int i = 0; i < 10; ++i) 
            attr_count += attr_vis[i];
        auto it = findOrThrow(worldBloomDifferentAttributeBonuses, [&](const auto &it) { 
            return it.attributeCount == attr_count; 
        }, [&]() { return "World bloom different attribute bonus not found for attributeCount=" + std::to_string(attr_count); });
        ret.diffAttrBonus = it.bonusRate;
    }

    ret.totalBonus = ret.diffAttrBonus + std::accumulate(ret.cardBonus.begin(), ret.cardBonus.end(), 0.0);
    return ret;
}

SupportDeckBonus DeckCalculator::getSupportDeckBonus(
    const std::vector<const CardDetail*> &deckCards, 
    const std::vector<SupportDeckCard>& supportCards, 
    int supportDeckCount
)
{
    double bonus = 0;
    int count = 0;
    
    std::vector<CardDetail> cards{};
    for (const auto &card : supportCards) {
        // 支援卡组的卡不能和主队伍重复，需要排除掉
        if (std::find_if(deckCards.begin(), deckCards.end(), [&](const auto &it) { 
            return it->cardId == card.cardId;
        }) != deckCards.end()) 
            continue;
        bonus += card.bonus;
        count++;
        if (count >= supportDeckCount) return { bonus, cards };
    }
    // 就算组不出完整的支援卡组也得返回
    return { bonus, cards };
}

int DeckCalculator::getHonorBonusPower()
{
    auto& honors = this->dataProvider.masterData->honors;
    auto& userHonors = this->dataProvider.userData->userHonors;
    int bonus = 0;
    for (const auto &userHonor : userHonors) {
        auto it = findOrThrow(honors, [&](const auto &it) { 
            return it.id == userHonor.honorId; 
        }, [&]() { return "Honor not found for honorId=" + std::to_string(userHonor.honorId); });
        auto levelIt = findOrThrow(it.levels, [&](const auto &it) { 
            return it.level == userHonor.level; 
        }, [&]() { return "Honor level not found for honorId=" + std::to_string(userHonor.honorId) + " level=" + std::to_string(userHonor.level); });
        bonus += levelIt.bonus;
    }
    return bonus;
}


std::vector<DeckDetail> DeckCalculator::getDeckDetailByCards(
    const std::vector<const CardDetail*> &cardDetails, 
    std::map<int, std::vector<SupportDeckCard>>& supportCards,
    int honorBonus, 
    std::optional<int> eventType,
    std::optional<int> eventId,
    SkillReferenceChooseStrategy skillReferenceChooseStrategy,
    bool keepAfterTrainingState,
    bool bestSkillAsLeader
)
{   
    // 活动加成
    auto eventBonusInfo = getDeckBonus(cardDetails, eventType, eventId);
    
    // 支援加成
    SupportDeckBonus supportDeckBonus{};
    if (supportCards.size()) {
        std::vector<SupportDeckCard>* pSupportCards = nullptr;
        if (eventId.value_or(0) == finalChapterEventId) 
            pSupportCards = &supportCards[cardDetails[0]->characterId]; // 终章支援角色为队长角色
        else
            pSupportCards = &(supportCards.begin()->second);    // 普通wl只会处理出一组支援卡牌列表
        supportDeckBonus = this->getSupportDeckBonus(
            cardDetails, *pSupportCards, 
            this->getWorldBloomSupportDeckCount(eventId.value_or(0))
        );
    }

    // 预处理队伍和属性，存储每个队伍或属性出现的次数
    int card_num = int(cardDetails.size());
    int attr_map[16] = {};
    int unit_map[16] = {};
    for (auto p : cardDetails) {
        auto& cardDetail = *p;
        attr_map[cardDetail.attr]++;
        for (const auto &key : cardDetail.units) {
            unit_map[key]++;
        }
    }
    int unit_num = 0;
    for (int i = 0; i < 16; ++i) 
        unit_num += bool(unit_map[i]);

    // 计算当前卡组的综合力，要加上称号的固定加成
    std::array<DeckCardPowerDetail, 5> cardPower{};
    for (int i = 0; i < card_num; ++i) {
        auto& cardDetail = *cardDetails[i];
        DeckCardPowerDetail powerDetail = {};
        for (const auto &unit : cardDetail.units) {
            auto current = cardDetail.power.get(unit, unit_map[unit], attr_map[cardDetail.attr]);
            // 有多个组合时，取最高加成组合
            powerDetail = current.total > powerDetail.total ? current : powerDetail;
        }
        cardPower[i] = powerDetail;
    }
    DeckPowerDetail power{};
    for (int i = 0; i < card_num; ++i) {
        auto& p = cardPower[i];
        power.base += p.base;
        power.areaItemBonus += p.areaItemBonus;
        power.characterBonus += p.characterBonus;
        power.fixtureBonus += p.fixtureBonus;
        power.gateBonus += p.gateBonus;
        power.total += p.total;
    }
    power.total += honorBonus;

    // 计算当前卡组每个卡牌的花前/花后固定技能效果（进Live之前）
    std::array<std::array<DeckCardSkillDetail, 2>, 5> prepareSkills{};
    int doubleSkillMask = 0;
    int needEnumerateStatusMask = 0;  
    int needEnumerateCount = 0;

    for (int i = 0; i < card_num; ++i) {
        auto& cardDetail = *cardDetails[i];
        // 获取普通技能效果（所有普通技能&bf花后）
        DeckCardSkillDetail s2 = {};
        // 组分技能效果（对vs有多个组合取最大）或 固定技能效果
        for (const auto &unit : cardDetail.units) {
            auto current = cardDetail.skill.get(unit, unit_map[unit], 1);
            if (current.scoreUp > s2.scoreUp) s2 = current;
        }

        // 获取双技能的花前技能效果，以及判断是否需要枚举技能状态
        DeckCardSkillDetail s1 = {};
        bool needEnumerate = false;

        // 吸分技能效果(max)
        auto current = cardDetail.skill.get(Enums::Unit::ref, 1, 1);
        current.scoreUp += current.scoreUpReferenceMax;   
        if (current.skillId != s2.skillId && current.scoreUp > s1.scoreUp) {
            s1 = current;
            needEnumerate = true;   // 吸分技能需要枚举
        }
        // 异组技能效果
        current = cardDetail.skill.get(Enums::Unit::diff, unit_num - 1, 1);
        if (current.skillId != s2.skillId && current.scoreUp > s1.scoreUp) {
            s1 = current;
            needEnumerate = false;  // 异组技能不需要枚举
        }

        // 记录有双技能的位置
        if(s1.skillId) doubleSkillMask |= (1 << i); 

        if (keepAfterTrainingState) {
            // 如果指定不改变状态，则无论如何都不枚举，并且根据用户选择的状态设置
            if(cardDetail.defaultImage != Enums::DefaultImage::special_training && s2.isAfterTraining)
                s2 = s1; // 用户设置花前技能
        } else {
            if (needEnumerate) {
                // 需要枚举则记录需要枚举的位置
                needEnumerateStatusMask |= (1 << i);   
                needEnumerateCount++;
            } else {
                // 不需要枚举则花后设置为两个技能的最大 
                s2 = (s2.scoreUp >= s1.scoreUp ? s2 : s1); 
            }
        }

        prepareSkills[i] = { s1, s2 };
    }

    // 枚举技能状态，计算当前卡组的实际技能效果（包括选择花前/花后技能），并归纳卡牌在队伍中的详情信息
    std::array<DeckCardSkillDetail, 5> skills{};
    std::array<int, 5> order{};
    std::vector<double> memberSkillMaxs{};
    std::vector<std::pair<int, int>> scoreUps{};
    scoreUps.reserve(1 << needEnumerateCount);
    std::vector<DeckDetail> ret{};
    for (int mask = needEnumerateStatusMask; mask >= 0; mask = mask ? (mask - 1) & needEnumerateStatusMask : -1) {
        // 根据mask枚举花前/花后技能状态，计算实际技能
        for (int i = 0; i < card_num; ++i) {
            auto& s1 = prepareSkills[i][0]; // 花前技能
            auto& s2 = prepareSkills[i][1]; // 花后技能（或者已经被花前技能替换的技能）
            auto& s = (mask & (1 << i)) ? s1 : s2; // 实际技能，0为花后技能，1为花前技能
            s.scoreUpToReference = s.scoreUp; // 此时的值为吸分技能能吸取的值
            skills[i] = s;
        } 

        // 计算枚举状态的技能的实际值
        for (int i = 0; i < card_num; ++i) {
            auto& s = skills[i];

            // 吸分
            if (s.hasScoreUpReference) {
                s.scoreUp -= s.scoreUpReferenceMax; // 从max回到还没吸的基础值
                memberSkillMaxs.clear();
                // 收集其他成员的技能最大值
                for (int j = 0; j < card_num; ++j) if (i != j) {
                    double m = skills[j].scoreUpToReference;
                    m = std::min(std::floor(m * s.scoreUpReferenceRate / 100.), s.scoreUpReferenceMax);
                    memberSkillMaxs.push_back(m);
                }
                // 不同选择策略
                double chosenSkillMax = 0;
                if (skillReferenceChooseStrategy == SkillReferenceChooseStrategy::Max) 
                    chosenSkillMax = *std::max_element(memberSkillMaxs.begin(), memberSkillMaxs.end());
                else if (skillReferenceChooseStrategy == SkillReferenceChooseStrategy::Min)
                    chosenSkillMax = *std::min_element(memberSkillMaxs.begin(), memberSkillMaxs.end());
                else if (skillReferenceChooseStrategy == SkillReferenceChooseStrategy::Average)
                    chosenSkillMax = std::accumulate(memberSkillMaxs.begin(), memberSkillMaxs.end(), 0.0) / memberSkillMaxs.size();
                s.scoreUp += chosenSkillMax; 
            } 
        }

        std::iota(order.begin(), order.begin() + card_num, 0);
        if (bestSkillAsLeader) {
            // 如果需要，调整最大技能的卡为队长
            int bestIndex = std::max_element(order.begin(), order.begin() + card_num, [&skills, &cardDetails](int x, int y) {
                return std::tuple(skills[x].scoreUp, -cardDetails[x]->cardId) < std::tuple(skills[y].scoreUp, -cardDetails[y]->cardId);
            }) - order.begin();
            if (bestIndex != 0) std::swap(order[0], order[bestIndex]);
        } else {
            // 否则只需要队长之后的按卡牌ID排序
            std::sort(order.begin() + 1, order.begin() + card_num, [&cardDetails](int x, int y) {
                return cardDetails[x]->cardId < cardDetails[y]->cardId;
            });
        }

        // 检查当前队长技能/其他成员技能的总和，如果都劣于或等于之前某组，则不用考虑该组
        // 分开考虑队长和其他成员，是考虑到协力和单人live技能机制不同
        double leaderScoreUp = 0, otherScoreUpSum = 0;
        for (auto i : order) {
            if (i == 0) leaderScoreUp = skills[i].scoreUp;
            else otherScoreUpSum += skills[i].scoreUp;
        }
        bool skip = false;
        for (const auto& scoreUp : scoreUps) {
            if (scoreUp.first >= leaderScoreUp && scoreUp.second >= otherScoreUpSum) {
                skip = true;
                break;
            }
        }
        if (skip) continue;
        scoreUps.push_back({ leaderScoreUp, otherScoreUpSum });

        // 归纳卡牌在队伍中的详情信息
        std::vector<DeckCardDetail> cards{};
        cards.reserve(card_num);
        for (auto i : order) {
            auto& cardDetail = *cardDetails[i];

            // 如果确实是双技能，根据技能调整卡面状态
            int defaultImage = cardDetail.defaultImage;
            if (doubleSkillMask & (1 << i)) {
                defaultImage = skills[i].isAfterTraining ? Enums::DefaultImage::special_training : Enums::DefaultImage::original;
            }

            cards.push_back(DeckCardDetail{ 
                cardDetail.cardId, 
                cardDetail.level, 
                cardDetail.skillLevel, 
                cardDetail.masterRank, 
                cardPower[i],
                eventBonusInfo.cardBonus[i],
                skills[i],
                cardDetail.episode1Read,
                cardDetail.episode2Read,
                cardDetail.afterTraining,
                defaultImage,
                cardDetail.hasCanvasBonus,
            });
        }

        // 计算多人live的技能实效
        double multiLiveScoreUp = 0;
        multiLiveScoreUp += skills[order[0]].scoreUp;
        for (int i = 1; i < card_num; ++i) 
            multiLiveScoreUp += skills[order[i]].scoreUp * 0.2;

        ret.push_back(DeckDetail{ 
            .power = power, 
            .eventBonus = eventBonusInfo.totalBonus,
            .supportDeckBonus = supportDeckBonus.bonus,
            .supportDeckCards = std::nullopt, // supportDeckBonus.cards
            .cards = std::move(cards),
            .multiLiveScoreUp = multiLiveScoreUp
        });
    }

    return ret;
}


int DeckCalculator::getWorldBloomSupportDeckCount(int eventId) const
{
    int turn = this->dataProvider.masterData->getWorldBloomEventTurn(eventId);
    // wl1 12 wl2 20
    return turn == 1 ? 12 : 20;
}
