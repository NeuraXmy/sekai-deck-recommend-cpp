#ifndef USER_MYSEKAI_CANVAS_H
#define USER_MYSEKAI_CANVAS_H

#include "common/collection-utils.h"

struct UserMysekaiCanvas {
    int mysekaiFixtureId = 0;
    int cardId = 0;
    bool isSpecialTraining = false;
    int quantity = 0;

    static inline std::vector<UserMysekaiCanvas> fromJsonList(const json& jsonData) {
        std::vector<UserMysekaiCanvas> userMysekaiCanvases;
        for (const auto& item : jsonData) {
            UserMysekaiCanvas userMysekaiCanvas;
            userMysekaiCanvas.mysekaiFixtureId = item.value("mysekaiFixtureId", 0);
            userMysekaiCanvas.cardId = item.value("cardId", 0);
            userMysekaiCanvas.isSpecialTraining = item.value("isSpecialTraining", false);
            userMysekaiCanvas.quantity = item.value("quantity", 0);
            userMysekaiCanvases.push_back(userMysekaiCanvas);
        }
        return userMysekaiCanvases;
    }
};

#endif  // USER_MYSEKAI_CANVAS_H