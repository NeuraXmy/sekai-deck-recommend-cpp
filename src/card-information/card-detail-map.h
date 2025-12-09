#ifndef CARD_DETAIL_MAP_H
#define CARD_DETAIL_MAP_H

#include <optional>
#include <string>
#include <map>

#include "common/collection-utils.h"

constexpr int UNIT_MAX = 12;
constexpr int UNIT_MEMBER_MAX = 6;
constexpr int ATTR_MEMBER_MAX = 2;

/**
 * 用于记录在不同的同组合、同属性加成的情况下的综合力或加分技能
 */
template <typename T>
class CardDetailMap {

    inline const std::optional<T> getValue(int unit, int unitMember, int attrMember) const {
        int key = getKey(unit, unitMember, attrMember);
        if (this->values[key].has_value())
            return this->values[key];
        return std::nullopt;
    }

public:
    std::array<std::optional<T>, UNIT_MAX * UNIT_MEMBER_MAX * ATTR_MEMBER_MAX> values = {};
    int min = std::numeric_limits<int>::max();
    int max = std::numeric_limits<int>::min();
    
    /**
     * 设定给定情况下的值
     * 为了减少内存消耗，人数并非在所有情况下均为实际值，可能会用1代表混组或无影响
     * @param unit 该map的类别; 组合不影响实际值的情况:any 组分:对应组合(vs可能有2种) vsbf花前:diff ocbf花前:ref 
     * @param unitMember 组合对应的人数（组分技能代表相同组数1-5; vsbf花前代表为不同组数0-2; 其他情况5为同组1为混组或无影响）
     * @param attrMember 卡牌属性对应的人数（5人为同色、1人为混色或无影响）
     * @param cmpValue 设置最小值、最大值的用于剪枝的可比较值
     * @param value 实际值
     */
    inline void set(int unit, int unitMember, int attrMember, int cmpValue, const T& value) {
        this->min = std::min(this->min, cmpValue);
        this->max = std::max(this->max, cmpValue);
        this->values[getKey(unit, unitMember, attrMember)] = value;
    }

    /**
     * 获取给定情况下的值
     * 会返回最合适的值，如果给定的条件与卡牌完全不符会给出异常
     * @param unit 组合
     * @param unitMember 该组合对应的人数（真实值）
     * @param attrMember 卡牌属性对应的人数（真实值）
     */
    inline T get(int unit, int unitMember, int attrMember) const {
        // 所有情况下，属性实际只有混不混的区别
        attrMember = (attrMember == 5 ? 5 : 1);
        std::optional<T> best{};

        // (vsbf花前) 不同组数只能是0-2
        if (unit == Enums::Unit::diff) {
            best = this->getValue(Enums::Unit::diff, std::min(2, unitMember), 1);
            if (best.has_value()) return best.value();
        }

        // (ocbf花前) 这边unit其实是当作技能tag用
        if (unit == Enums::Unit::ref) {
            best = this->getValue(Enums::Unit::ref, 1, 1);
            if (best.has_value()) return best.value();
        }

        // (组分) 受指定组合人数影响的情况 
        best = this->getValue(unit, unitMember, attrMember);  
        if (best.has_value()) return best.value();

        // (综合力计算) 只考虑混不混组的情况
        best = this->getValue(unit, unitMember == 5 ? 5 : 1, attrMember);
        if (best.has_value()) return best.value();

        // 技能为固定数值的情况（能够变化但取到保底固定数值的也会落到这里）
        best = this->getValue(Enums::Unit::any, 1, 1);
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
        assert(unit >= 0 && unit < 12);
        assert(unitMember >= 0 && unitMember <= 5);
        assert(attrMember == 1 || attrMember == 5);
        return (unit * UNIT_MEMBER_MAX + unitMember) * ATTR_MEMBER_MAX + (attrMember == 5 ? 1 : 0);
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

