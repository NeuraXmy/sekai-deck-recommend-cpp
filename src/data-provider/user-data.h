#ifndef USER_DATA_H
#define USER_DATA_H

#include "data-provider/user-data-types.h"

class UserData {

public:
    std::string path;

    UserGameData userGamedata;
    std::vector<UserArea> userAreas;
    std::vector<UserCard> userCards;
    std::vector<UserChallengeLiveSoloDeck> userChallengeLiveSoloDecks;
    std::vector<UserCharacter> userCharacters;
    std::vector<UserDeck> userDecks;
    std::vector<UserHonor> userHonors;
    std::vector<UserMysekaiCanvas> userMysekaiCanvases;
    std::vector<UserMysekaiFixtureGameCharacterPerformanceBonus> userMysekaiFixtureGameCharacterPerformanceBonuses;
    std::vector<UserMysekaiGate> userMysekaiGates;
    std::vector<UserWorldBloomSupportDeck> userWorldBloomSupportDecks;

    // 预处理终章用户哪些角色有称号活动加成，在dataProvider中计算
    std::map<int, double> userCharacterFinalChapterHonorEventBonusMap;     

    void loadFromJson(const json& j);

    void loadFromFile(const std::string& path);

    void loadFromString(const std::string& s);
};


#endif // USER_DATA_H