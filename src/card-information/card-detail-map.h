#ifndef CARD_DETAIL_MAP_H
#define CARD_DETAIL_MAP_H

#include <optional>
#include <string>
#include <map>

#include "common/json-utils.h"

inline int any_unit_enum = mapEnum(EnumMap::unit, "any");

constexpr int MAX_UNIT_NUM = 8;
constexpr int MAX_UNIT_MEMBER_NUM = 5;
constexpr int MAX_ATTR_MEMBER_NUM = 2;

/**
 * 用于记录在不同的同组合、同属性加成的情况下的综合力或加分技能
 */
template <typename T>
class CardDetailMap {

    inline const std::optional<T>& getInterval(int unit, int unitMember, int attrMember) const {
        return this->values[getKey(unit, unitMember, attrMember)];
    }

public:
    int min = std::numeric_limits<int>::max();
    int max = std::numeric_limits<int>::min();
    std::optional<T> values[MAX_UNIT_NUM * MAX_UNIT_MEMBER_NUM * MAX_ATTR_MEMBER_NUM] = {};

    /**
     * 设定给定情况下的值
     * 为了减少内存消耗，人数并非在所有情况下均为实际值，可能会用1代表混组或无影响
     * @param unit 特定卡牌组合（虚拟歌手卡牌可能存在两个组合）
     * @param unitMember 该组合对应的人数（用于受组合影响的技能时，1-5、其他情况，5人为同组、1人为混组或无影响）
     * @param attrMember 卡牌属性对应的人数（5人为同色、1人为混色或无影响）
     * @param cmpValue
     * @param value 设定的值
     */
    inline void set(int unit, int unitMember, int attrMember, int cmpValue, const T& value) {
        this->min = std::min(this->min, cmpValue);
        this->max = std::max(this->max, cmpValue);
        this->values[getKey(unit, unitMember, attrMember)] = value;
    }

    /**
     * 获取给定情况下的值
     * 会返回最合适的值，如果给定的条件与卡牌完全不符会给出异常
     * @param unit 特定卡牌组合（虚拟歌手卡牌可能存在两个组合）
     * @param unitMember 该组合对应的人数（真实值）
     * @param attrMember 卡牌属性对应的人数（真实值）
     */
    inline T get(int unit, int unitMember, int attrMember) const {
       // 因为实际上的attrMember取值只能是5和1，直接优化掉
        int attrMember0 = attrMember == 5 ? 5 : 1;
        auto best = this->getInterval(unit, unitMember, attrMember0);
        if (best.has_value()) return best.value();
        // 有可能unitMember在混组的时候优化成1了
        best = this->getInterval(unit, unitMember == 5 ? 5 : 1, attrMember0);
        if (best.has_value()) return best.value();
        // 有可能因为技能是固定数值，attrMember、unitMember都优化成1了，组合直接为any
        best = this->getInterval(any_unit_enum, 1, 1);
        if (best.has_value()) return best.value();
        // 如果这还找不到，说明给的情况就不对
        throw std::runtime_error("case not found");
    }

    /**
     * 实际用于Map的key，用于内部调用避免创建对象的开销
     * @param unit 组合
     * @param unitMember 组合人数
     * @param attrMember 属性人数
     * @private
     */
    inline int getKey(int unit, int unitMember, int attrMember) const {
        assert(unit >= 0 && unit < MAX_UNIT_NUM);
        assert(unitMember >= 1 && unitMember <= MAX_UNIT_MEMBER_NUM);
        assert(attrMember == 1 || attrMember == 5);
        return (attrMember == 1 ? 0 : 1) * MAX_UNIT_NUM * MAX_UNIT_MEMBER_NUM
             + (unitMember - 1) * MAX_UNIT_NUM
             + unit;
    }

    /**
     * 是否肯定比另一个范围小
     * 如果几个维度都比其他小，这张卡可以在自动组卡时舍去
     * @param another 另一个范围
     */
    inline bool isCertainlyLessThan(const CardDetailMap<T>& another) const {
        return this->max < another.min;
    }
};

#endif // CARD_DETAIL_MAP_H

