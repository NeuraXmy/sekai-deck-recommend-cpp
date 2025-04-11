#include "data-provider/music-metas.h"
#include <fstream>

MusicMetas::MusicMetas(const std::string &path)
{
    this->path = path;
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    json j;
    file >> j;
    file.close();

    this->metas = MusicMeta::fromJsonList(j);
}