#ifndef EVENT_SERVICE_H
#define EVENT_SERVICE_H

#include "data-provider/data-provider.h"

/**
 * 活动信息设置
 */
struct EventConfig {
    int eventId = 0;           // 活动ID
    int eventType = 0;         // 活动类型
    int eventUnit = 0;         // 箱活团队
    int specialCharacterId = 0;   // 特殊角色ID，用于世界开花活动
};

class EventService {

    DataProvider dataProvider;

public:

    EventService(const DataProvider& dataProvider) : dataProvider(dataProvider) {}

    /**
     * 获取活动类型
     * @param eventId 活动ID
     */
    int getEventType(int eventId);

    /**
     * 获取活动设置
     * @`param eventId 活动ID
     * @param specialCharacterId 特别选择的角色ID（用于世界开花活动）
     */
    EventConfig getEventConfig(int eventId, int specialCharacterId = 0);

    /**
     * 获取箱活的加成团队
     * 如果非箱活，会返回0
     * @param eventId 活动ID
     */
    int getEventBonusUnit(int eventId);

};


#endif // EVENT_SERVICE_H