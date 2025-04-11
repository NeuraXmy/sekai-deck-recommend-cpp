#include "data-provider/user-data.h"

#include <fstream>
#include <iostream>


template<typename T>
T loadUserDataFromJson(const json& j, const std::string& key, bool required = true) {
    if (j.contains(key)) {
        return T::fromJson(j[key]);
    } 
    if (required) {
        throw std::runtime_error("user data key not found: " + key);
    }
    std::cerr << "[sekai-deck-recommend-cpp] warning: user data key not found: " + key << std::endl;
    return {};
}

template<typename T>
std::vector<T> loadUserDataFromJsonList(const json& j, const std::string& key, bool required = true) {
    if (j.contains(key)) {
        return T::fromJsonList(j[key]);
    } 
    if (required) {
        throw std::runtime_error("user data key not found: " + key);
    }
    std::cerr << "[sekai-deck-recommend-cpp] warning: user data key not found: " + key << std::endl;
    return {};
}


UserData::UserData(const std::string& path) {
    this->path = path;
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open user data file: " + path);
    }
    json j;
    file >> j;
    file.close();

    this->userGamedata = loadUserDataFromJson<UserGameData>(j, "userGamedata");
    this->userAreas = loadUserDataFromJsonList<UserArea>(j, "userAreas");
    this->userCards = loadUserDataFromJsonList<UserCard>(j, "userCards");
    this->userChallengeLiveSoloDecks = loadUserDataFromJsonList<UserChallengeLiveSoloDeck>(j, "userChallengeLiveSoloDecks");
    this->userCharacters = loadUserDataFromJsonList<UserCharacter>(j, "userCharacters");
    this->userDecks = loadUserDataFromJsonList<UserDeck>(j, "userDecks");
    this->userHonors = loadUserDataFromJsonList<UserHonor>(j, "userHonors");

    this->userMysekaiCanvases = loadUserDataFromJsonList<UserMysekaiCanvas>(j, "userMysekaiCanvases", false);
    this->userMysekaiFixtureGameCharacterPerformanceBonuses = loadUserDataFromJsonList<UserMysekaiFixtureGameCharacterPerformanceBonus>(j, "userMysekaiFixtureGameCharacterPerformanceBonuses", false);
    this->userMysekaiGates = loadUserDataFromJsonList<UserMysekaiGate>(j, "userMysekaiGates", false);
    this->userWorldBloomSupportDecks = loadUserDataFromJsonList<UserWorldBloomSupportDeck>(j, "userWorldBloomSupportDecks", false);
}