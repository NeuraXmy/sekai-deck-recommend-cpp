#include "data-provider/music-metas.h"
#include <fstream>

void MusicMetas::loadFromJson(const json &j)
{
    this->metas = MusicMeta::fromJsonList(j);
}

void MusicMetas::loadFromFile(const std::string &path)
{
    json j;
    try {
        this->path = path;
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + path);
        }
        file >> j;
        file.close();
    }
    catch (const std::exception &e) {
        throw std::runtime_error("Failed to load music metas from file: " + path + ", error: " + e.what());
    }
    this->loadFromJson(j);
}

void MusicMetas::loadFromString(const std::string& s)
{
    json j;
    try {
        this->path.clear();
        j = json::parse(s);
    } 
    catch (const std::exception &e) {
        throw std::runtime_error("Failed to load music metas from string, error: " + std::string(e.what()));
    }
    this->loadFromJson(j);
}