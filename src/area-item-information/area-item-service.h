#ifndef AREA_ITEM_SERVICE_H
#define AREA_ITEM_SERVICE_H

#include "data-provider/data-provider.h"

class AreaItemService {

    DataProvider dataProvider;

public:
    AreaItemService(const DataProvider& dataProvider) : dataProvider(dataProvider) {}

    /**
     * 获取用户纳入计算的区域道具
     */
    std::vector<AreaItemLevel> getAreaItemLevels();

    /**
     * 获取对应等级的区域道具
     * @param areaItemId 区域道具ID
     * @param level 等级
     */
    AreaItemLevel getAreaItemLevel(int areaItemId, int level);

    /**
     * 获取下一级区域道具
     * 目前只支持1～15级
     * @param areaItem 区域道具
     * @param areaItemLevel （可选）当前等级
     */
    AreaItemLevel getAreaItemNextLevel(const AreaItem& areaItem, std::optional<AreaItemLevel> areaItemLevel = std::nullopt);

    /**
     * 获取区域道具等级对应的ShopItem
     * 按理来说应该先去resourceBoxes中找到道具等级对应的ID，再通过resourceBoxId获取ShopItem
     * 但是为了这么简单的需求获取一个11MB的resourceBoxes纯属想不开，所以就自己找规律推一下了
     * 目前只支持1～15级
     * @param areaItemLevel 区域道具等级
     */
    ShopItem getShopItem(const AreaItemLevel& areaItemLevel);

};

#endif // AREA_ITEM_SERVICE_H