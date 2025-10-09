#ifndef DECK_SERVICE_H
#define DECK_SERVICE_H

#include "data-provider/data-provider.h"
#include "card-information/card-calculator.h"
#include "card-information/card-service.h"

#include <optional>


struct DeckPowerDetail {
    int base;
    int areaItemBonus;
    int characterBonus;
    int honorBonus;
    int fixtureBonus;
    int gateBonus;
    int total;
};

struct DeckCardDetail {
    int cardId;
    int level;
    int skillLevel;
    int masterRank;
    DeckCardPowerDetail power;
    std::optional<double> eventBonus;
    DeckCardSkillDetail skill;
    bool episode1Read;
    bool episode2Read;
    bool afterTraining;
    int defaultImage;
    bool hasCanvasBonus;
};

struct DeckDetail {
    DeckPowerDetail power;
    std::optional<double> eventBonus;
    std::optional<double> supportDeckBonus;
    std::optional<std::vector<CardDetail>> supportDeckCards;    // for debug
    std::vector<DeckCardDetail> cards;
    double multiLiveScoreUp;
};


class DeckService {

    DataProvider dataProvider;

public:
    DeckService(const DataProvider& dataProvider) : dataProvider(dataProvider) {}
    
    /**
     * 获取用户卡牌
     * @param cardId 卡牌ID
     */
    UserCard getUserCard(int cardId);

    /**
     * 通过卡组ID获取用户卡组
     * @param deckId 用户卡组ID
     */
    UserDeck getDeck(int deckId);

    /**
     * 获得用户卡组中的用户卡牌
     * @param userDeck 用户卡组
     */
    std::vector<UserCard> getDeckCards(const UserDeck& userDeck);

    /**
     * 给定卡牌组建新的用户卡组
     * @param userCards 卡牌（5张）
     * @param userId 玩家ID
     * @param deckId 卡组ID
     * @param name 卡组名称
     */
    UserDeck toUserDeck(
        const std::vector<DeckCardDetail>& userCards, 
        long long userId = 1145141919810,
        int deckId = 1
    );

    /**
     * 通过角色ID获取挑战Live卡组
     * @param characterId 角色ID
     */
    UserChallengeLiveSoloDeck getChallengeLiveSoloDeck(int characterId);

    /**
     * 获取用户挑战Live卡组中的卡牌
     * @param deck 挑战Live卡组
     */
    std::vector<UserCard> getChallengeLiveSoloDeckCards(const UserChallengeLiveSoloDeck& deck);

    /**
     * 给定卡牌组建新的用户挑战卡组
     * @param userCards 卡牌（2～5张）
     * @param characterId 角色ID
     */
    UserChallengeLiveSoloDeck toUserChallengeLiveSoloDeck(const std::vector<DeckCardDetail>& userCards, int characterId);

    /**
     * 给定卡牌组建新的用户世界连接应援卡组
     * @param userCards 卡牌（0～12张）
     * @param gameCharacterId 角色ID
     */
    UserWorldBloomSupportDeck toUserWorldBloomSupportDeck(const std::vector<CardDetail>& userCards, int gameCharacterId);

};

#endif  // DECK_SERVICE_H