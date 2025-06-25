#ifndef MUSIC_METAS_H
#define MUSIC_METAS_H

#include "common/music-meta.h"

class MusicMetas {
public:
    std::string path;

    std::vector<MusicMeta> metas;

    void loadFromJson(const json& j);

    void loadFromFile(const std::string& path);

    void loadFromString(const std::string& s);
};

#endif // MUSIC_METAS_H    