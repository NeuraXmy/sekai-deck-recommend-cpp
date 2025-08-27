#include "data-provider/user-data.h"

#include <fstream>
#include <iostream>


template<typename T>
T loadUserDataJson(const json& j, const std::string& key, bool required = true) {
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
std::vector<T> loadUserDataJsonList(const json& j, const std::string& key, bool required = true) {
    if (j.contains(key)) {
        return T::fromJsonList(j[key]);
    } 
    if (required) {
        throw std::runtime_error("user data key not found: " + key);
    }
    std::cerr << "[sekai-deck-recommend-cpp] warning: user data key not found: " + key << std::endl;
    return {};
}


void UserData::loadFromJson(const json& j) {
    this->userGamedata = loadUserDataJson<UserGameData>(j, "userGamedata");
    this->userAreas = loadUserDataJsonList<UserArea>(j, "userAreas");
    this->userCards = loadUserDataJsonList<UserCard>(j, "userCards");
    this->userCharacters = loadUserDataJsonList<UserCharacter>(j, "userCharacters");
    // this->userDecks = loadUserDataJsonList<UserDeck>(j, "userDecks");
    this->userHonors = loadUserDataJsonList<UserHonor>(j, "userHonors");

    this->userMysekaiCanvases = loadUserDataJsonList<UserMysekaiCanvas>(j, "userMysekaiCanvases", false);
    this->userMysekaiFixtureGameCharacterPerformanceBonuses = loadUserDataJsonList<UserMysekaiFixtureGameCharacterPerformanceBonus>(j, "userMysekaiFixtureGameCharacterPerformanceBonuses", false);
    this->userMysekaiGates = loadUserDataJsonList<UserMysekaiGate>(j, "userMysekaiGates", false);
}

void UserData::loadFromFile(const std::string& path) {
    json j;
    try {
        this->path = path;
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open user data file: " + path);
        }
        file >> j;
        file.close();
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load user data from file: " + path + ", error: " + e.what());
    }
    this->loadFromJson(j);
}

void UserData::loadFromString(const std::string& s) {
    json j;
    try {
        this->path.clear();
        j = json::parse(s);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load user data from bytes, error: " + std::string(e.what()));
    }
    this->loadFromJson(j);
}