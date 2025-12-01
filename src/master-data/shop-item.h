#ifndef SHOP_ITEM_H
#define SHOP_ITEM_H

#include "common/collection-utils.h"

struct ShopItem {
    int id = 0;
    int shopId = 0;
    int seq = 0;
    int releaseConditionId = 0;
    int resourceBoxId = 0;

    static inline std::vector<ShopItem> fromJsonList(const json& jsonData) {
        std::vector<ShopItem> shopItems;
        for (const auto& item : jsonData) {
            ShopItem shopItem;
            shopItem.id = item.value("id", 0);
            shopItem.shopId = item.value("shopId", 0);
            shopItem.seq = item.value("seq", 0);
            shopItem.releaseConditionId = item.value("releaseConditionId", 0);
            shopItem.resourceBoxId = item.value("resourceBoxId", 0);
            shopItems.push_back(shopItem);
        }
        return shopItems;
    }
};

#endif