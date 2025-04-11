#include "live-score/live-calculator.h"

static int enum_solo = mapEnum(EnumMap::liveType, "solo");
static int enum_challenge = mapEnum(EnumMap::liveType, "challenge");
static int enum_multi = mapEnum(EnumMap::liveType, "multi");
static int enum_cheerful = mapEnum(EnumMap::liveType, "cheerful");
static int enum_auto = mapEnum(EnumMap::liveType, "auto");

MusicMeta LiveCalculator::getMusicMeta(int musicId, int musicDif)
{
    auto& musicMetas = this->dataProvider.musicMetas->metas;
    return findOrThrow(musicMetas, [musicId, musicDif](const MusicMeta &meta) {
        return meta.music_id == musicId && meta.difficulty == musicDif;
    });
}

double LiveCalculator::getBaseScore(const MusicMeta &musicMeta, int liveType)
{
    if (liveType == enum_solo || liveType == enum_challenge) {
        return musicMeta.base_score;
    } else if (liveType == enum_multi || liveType == enum_cheerful) {
        return musicMeta.base_score + musicMeta.fever_score;
    } else if (liveType == enum_auto) {
        return musicMeta.base_score_auto;
    } else {
        throw std::runtime_error("Invalid live type");
    }
}

std::vector<double> LiveCalculator::getSkillScore(const MusicMeta &musicMeta, int liveType)
{
    if (liveType == enum_solo || liveType == enum_challenge) {
        return musicMeta.skill_score_solo;
    } else if (liveType == enum_multi || liveType == enum_cheerful) {
        return musicMeta.skill_score_multi;
    } else if (liveType == enum_auto) {
        return musicMeta.skill_score_auto;
    } else {
        throw std::runtime_error("Invalid live type");
    }
}

SortedSkillDetails LiveCalculator::getSortedSkillDetails(const DeckDetail &deckDetail, int liveType, const std::optional<std::vector<DeckCardSkillDetail>> &skillDetails)
{
    // 如果已经给定合法有效的技能数据，按给定的技能数据执行
    if (skillDetails.has_value() && skillDetails->size() == 6 && skillDetails->at(5).scoreUp > 0) {
        return SortedSkillDetails{*skillDetails, false};
    }
    // 如果是多人联机，复制6次当前卡组的效果
    if (liveType == enum_multi) {
        return SortedSkillDetails{ 
            std::vector<DeckCardSkillDetail>(6, getMultiLiveSkill(deckDetail)), 
            false 
        };
    }

    // 单人，按效果正序排序技能
    std::vector<DeckCardSkillDetail> sortedSkill{};
    for (const auto &card : deckDetail.cards) {
        sortedSkill.push_back(card.skill);
    }
    std::sort(sortedSkill.begin(), sortedSkill.end(), [](const DeckCardSkillDetail &a, const DeckCardSkillDetail &b) {
        return a.scoreUp < b.scoreUp;
    });
    // 如果卡牌数量不足5张，中间技能需要留空
    DeckCardSkillDetail emptySkill{0, 0};
    std::vector<DeckCardSkillDetail> emptySkills(5 - sortedSkill.size(), emptySkill);
    // 将有效技能填充到前面、中间留空、第6个固定为C位
    sortedSkill.insert(sortedSkill.end(), emptySkills.begin(), emptySkills.end());
    sortedSkill.push_back(deckDetail.cards[0].skill);
    return SortedSkillDetails{sortedSkill, true};
}


void LiveCalculator::sortSkillRate(bool sorted, int cardLength, std::vector<double> &skillScores)
{
    // 如果技能未排序，原样返回
    if (!sorted) {
        return;
    }
    // 按效果正序排序前cardLength个技能、中间和后面不动
    std::sort(skillScores.begin(), skillScores.begin() + cardLength);
}

LiveDetail LiveCalculator::getLiveDetailByDeck(const DeckDetail &deckDetail, const MusicMeta &musicMeta, int liveType, const std::optional<std::vector<DeckCardSkillDetail>> &skillDetails, int multiPowerSum)
{
    // 确定技能发动顺序，未指定则直接按效果排序或多人重复当前技能
    auto skills = this->getSortedSkillDetails(deckDetail, liveType, skillDetails);
    // 与技能无关的分数比例
    auto baseRate = this->getBaseScore(musicMeta, liveType);
    // 技能分数比例，如果是最佳技能计算则按加成排序（复制一下防止影响原数组顺序）
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
    double powerSum = multiPowerSum == 0 ? 5 * deckDetail.power.total : multiPowerSum;
    double activeBonus = liveType == enum_multi ? 5 * 0.015 * powerSum : 0;
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
        scoreUp,
        lifeRecovery
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
    ret = std::vector<DeckCardSkillDetail>(6, DeckCardSkillDetail{0, 0});
    // 将C位重复技能以外的技能分配到合适的位置
    for (size_t i = 0; i < skills.size() - 1; ++i) {
        ret[i] = skills[i];
    }
    // 将C位重复技能固定放在最后
    ret[5] = skills[skills.size() - 1];
    return ret;
}

LiveDetail LiveCalculator::getLiveDetail(const std::vector<UserCard> &deckCards, const MusicMeta &musicMeta, int liveType, const std::optional<std::vector<LiveSkill>> &liveSkills)
{
    auto deckDetail = this->deckCalculator.getDeckDetail(deckCards, deckCards);
    auto skills = liveType == enum_multi ? std::nullopt : this->getSoloLiveSkill(liveSkills.value(), deckDetail.cards);
    auto ret = this->getLiveDetailByDeck(deckDetail, musicMeta, liveType, skills);
    ret.deck = deckDetail; // 附加上卡组信息，方便debug
    return ret;
}

int LiveCalculator::getLiveScoreByDeck(const DeckDetail &deckDetail, const MusicMeta &musicMeta, int liveType)
{
    return this->getLiveDetailByDeck(deckDetail, musicMeta, liveType).score;
}

ScoreFunction LiveCalculator::getLiveScoreFunction(int liveType)
{
    return [this, liveType](const MusicMeta &musicMeta, const DeckDetail &deckDetail) {
        return this->getLiveScoreByDeck(deckDetail, musicMeta, liveType);
    };
}
