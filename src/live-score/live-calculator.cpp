#include "live-score/live-calculator.h"


MusicMeta LiveCalculator::getMusicMeta(int musicId, int musicDif)
{
    auto& musicMetas = this->dataProvider.musicMetas->metas;
    return findOrThrow(musicMetas, [musicId, musicDif](const MusicMeta &meta) {
        return meta.music_id == musicId && meta.difficulty == musicDif;
    }, [&]() { return "Music meta not found for musicId=" + std::to_string(musicId) + " musicDif=" + std::to_string(musicDif); });
}

double LiveCalculator::getBaseScore(const MusicMeta &musicMeta, int liveType)
{
    if (Enums::LiveType::isAuto(liveType))
        return musicMeta.base_score_auto;
    if (Enums::LiveType::isMulti(liveType))
        return musicMeta.base_score + musicMeta.fever_score * 0.5;
    return musicMeta.base_score;
}

std::vector<double> LiveCalculator::getSkillScore(const MusicMeta &musicMeta, int liveType)
{
    if (Enums::LiveType::isAuto(liveType))
        return musicMeta.skill_score_auto;
    if (Enums::LiveType::isMulti(liveType))
        return musicMeta.skill_score_multi;
    return musicMeta.skill_score_solo;
}

SortedSkillDetails LiveCalculator::getSortedSkillDetails(
    const DeckDetail &deckDetail, 
    int liveType, 
    LiveSkillOrder liveSkillOrder,
    std::optional<std::vector<int>> specificSkillOrder,
    const std::optional<std::vector<DeckCardSkillDetail>> &skillDetails,
    std::optional<int> multiTeammateScoreUp
)
{
    // 如果已经给定合法有效的技能数据，按给定的技能数据执行
    if (skillDetails.has_value() && skillDetails->size() == 6 && skillDetails->at(5).scoreUp > 0) {
        return SortedSkillDetails{*skillDetails, false};
    }

    int cardNum = (int)deckDetail.cards.size();
    std::vector<DeckCardSkillDetail> skills{};

    if (Enums::LiveType::isMulti(liveType)) {
        // 多人live
        auto selfSkill = getMultiLiveSkill(deckDetail);
        auto otherSkill = multiTeammateScoreUp.has_value() 
            ? DeckCardSkillDetail{ .scoreUp = (double)multiTeammateScoreUp.value() }
            : selfSkill;
        // 放入1个自己的技能和4个队友技能
        skills.push_back(selfSkill);
        for (int i = 0; i < 4; ++i) 
            skills.push_back(otherSkill);
        // 最后一个固定为自己技能
        skills.push_back(selfSkill);
    }
    else {
        // 单人live
        for (const auto &card : deckDetail.cards)
            skills.push_back(card.skill);
        // 最后一个固定C位
        skills.push_back(deckDetail.cards.front().skill);
    }

    bool skillSorted = false;

    // 根据技能排序方式处理
    if (liveSkillOrder == LiveSkillOrder::specific) {
        // 指定顺序
        if (!specificSkillOrder.has_value()) 
            throw std::runtime_error("specificSkillOrder is required for specific LiveSkillOrder");
        if (specificSkillOrder->size() != skills.size() - 1)
            throw std::runtime_error("specificSkillOrder size does not match skills size");
        
        // 按照顺序放入技能
        std::vector<DeckCardSkillDetail> orderedSkills{};
        for (const auto &index : specificSkillOrder.value()) {
            if (index < 0 || index >= skills.size()) 
                throw std::runtime_error("specificSkillOrder index out of range: " + std::to_string(index));
            orderedSkills.push_back(skills[index]);
        }
        // 放入返场技能
        orderedSkills.push_back(skills.back());
        skills = std::move(orderedSkills);
    }
    else if (liveSkillOrder == LiveSkillOrder::best) {
        // 最佳技能，按效果正序排序技能
        std::sort(skills.begin(), skills.begin() + cardNum, [](const DeckCardSkillDetail &a, const DeckCardSkillDetail &b) {
            return a.scoreUp < b.scoreUp;
        });
        skillSorted = true;
    }
    else if (liveSkillOrder == LiveSkillOrder::worst) {
        // 最差技能，按效果反序排序技能
        std::sort(skills.begin(), skills.begin() + cardNum, [](const DeckCardSkillDetail &a, const DeckCardSkillDetail &b) {
            return a.scoreUp > b.scoreUp;
        });
        skillSorted = true;
    }
    else if (liveSkillOrder == LiveSkillOrder::average) {
        // 平均技能，进行平均
        double avgScoreUp = 0;
        for (int i = 0; i < cardNum; ++i)
            avgScoreUp += skills[i].scoreUp;
        avgScoreUp /= cardNum;
        for (int i = 0; i < cardNum; ++i)
            skills[i].scoreUp = avgScoreUp;
    }

    if (cardNum < 5) {
        // 如果卡牌数量不足5张，中间技能需要留空
        DeckCardSkillDetail emptySkill{};
        std::vector<DeckCardSkillDetail> emptySkills(5 - cardNum, emptySkill);
        // 将有效技能填充到前面、中间留空、第6个固定为C位
        skills.insert(skills.end() - 1, emptySkills.begin(), emptySkills.end());
    }
    return SortedSkillDetails{skills, skillSorted};
}


void LiveCalculator::sortSkillRate(bool sorted, int cardLength, std::vector<double> &skillScores)
{
    // 如果技能未排序，原样返回
    if (!sorted) return;
    // 按效果正序排序前cardLength个技能段、中间和后面不动
    std::sort(skillScores.begin(), skillScores.begin() + cardLength);
}

LiveDetail LiveCalculator::getLiveDetailByDeck(
    const DeckDetail &deckDetail, 
    const MusicMeta &musicMeta, 
    int liveType, 
    LiveSkillOrder liveSkillOrder,
    std::optional<std::vector<int>> specificSkillOrder,
    const std::optional<std::vector<DeckCardSkillDetail>> &skillDetails, 
    int multiPowerSum,
    std::optional<int> multiTeammateScoreUp,
    std::optional<int> multiTeammatePower
)
{
    // 确定技能发动顺序，未指定则直接按效果排序或多人重复当前技能
    auto skills = this->getSortedSkillDetails(
        deckDetail, liveType, 
        liveSkillOrder, specificSkillOrder, 
        skillDetails, multiTeammateScoreUp
    );
    // 与技能无关的分数比例
    auto baseRate = this->getBaseScore(musicMeta, liveType);
    // 技能分数比例，如果是最佳/最差技能计算则按加成排序
    auto skillScores = this->getSkillScore(musicMeta, liveType);
    this->sortSkillRate(skills.sorted, deckDetail.cards.size(), skillScores);
    auto& skillRate = skillScores;
    // 计算总的分数比例
    double rate = baseRate;
    for (size_t i = 0; i < skills.details.size(); ++i) {
        rate += skills.details[i].scoreUp * skillRate[i] / 100.;
    }
    int life = 0;
    for (const auto &it : skills.details) {
        life += it.lifeRecovery;
    }
    // 活跃加分
    double powerSum = 5 * deckDetail.power.total;   // 默认复制5份自己
    if (multiPowerSum)
        powerSum = multiPowerSum;   // 指定总和
    if (multiTeammatePower.has_value())
        powerSum = deckDetail.power.total + multiTeammatePower.value() * 4; // 指定队友综合力 自己+4*队友
    double activeBonus = liveType == Enums::LiveType::isMulti(liveType) ? 5 * 0.015 * powerSum : 0;
    return LiveDetail{
        int(rate * deckDetail.power.total * 4 + activeBonus),
        musicMeta.music_time,
        std::min(2000, life + 1000),
        musicMeta.tap_count
    };
}

DeckCardSkillDetail LiveCalculator::getMultiLiveSkill(const DeckDetail &deckDetail)
{
    // 多人技能加分效果计算规则：C位100%发动、其他位置20%发动
    double scoreUp = 0;
    for (size_t i = 0; i < deckDetail.cards.size(); ++i) {
        scoreUp += (i == 0 ? deckDetail.cards[i].skill.scoreUp : (deckDetail.cards[i].skill.scoreUp / 5.));
    }
    // 奶判只看C位
    double lifeRecovery = deckDetail.cards[0].skill.lifeRecovery;
    return DeckCardSkillDetail{
        .scoreUp=scoreUp,
        .lifeRecovery=lifeRecovery,
    };
}

std::optional<std::vector<DeckCardSkillDetail>> LiveCalculator::getSoloLiveSkill(const std::vector<LiveSkill> &liveSkills, const std::vector<DeckCardDetail> &skillDetails)
{
    if (liveSkills.empty()) return std::nullopt;
    std::vector<DeckCardSkillDetail> skills = {};
    for (const auto &liveSkill : liveSkills) {
        skills.push_back(findOrThrow(skillDetails, [&](const DeckCardDetail &it) {
            return it.cardId == liveSkill.cardId;
        }).skill);
    }

    std::vector<DeckCardSkillDetail> ret{};
    // 因为可能会有技能空缺，先将无任何效果的技能放入6个位置
    ret = std::vector<DeckCardSkillDetail>(6, DeckCardSkillDetail{});
    // 将C位重复技能以外的技能分配到合适的位置
    for (size_t i = 0; i < skills.size() - 1; ++i) {
        ret[i] = skills[i];
    }
    // 将C位重复技能固定放在最后
    ret[5] = skills[skills.size() - 1];
    return ret;
}

int LiveCalculator::getLiveScoreByDeck(
    const DeckDetail &deckDetail, 
    const MusicMeta &musicMeta, 
    int liveType,
    LiveSkillOrder liveSkillOrder,
    std::optional<std::vector<int>> specificSkillOrder,
    std::optional<int> multiTeammateScoreUp,
    std::optional<int> multiTeammatePower
)
{
    return this->getLiveDetailByDeck(
        deckDetail, musicMeta, liveType, 
        liveSkillOrder, specificSkillOrder, 
        std::nullopt, 0, multiTeammateScoreUp, multiTeammatePower
    ).score;
}

ScoreFunction LiveCalculator::getLiveScoreFunction(
    int liveType,
    LiveSkillOrder liveSkillOrder,
    std::optional<std::vector<int>> specificSkillOrder,
    std::optional<int> multiTeammateScoreUp,
    std::optional<int> multiTeammatePower
)
{
    return [this, liveType, liveSkillOrder, specificSkillOrder, multiTeammateScoreUp, multiTeammatePower](const MusicMeta &musicMeta, const DeckDetail &deckDetail) {
        int liveScore = this->getLiveScoreByDeck(
            deckDetail, musicMeta, liveType, 
            liveSkillOrder, specificSkillOrder,
            multiTeammateScoreUp, multiTeammatePower
        );
        Score ret{};
        ret.score = liveScore;
        ret.liveScore = liveScore;
        return ret;
    };
}
