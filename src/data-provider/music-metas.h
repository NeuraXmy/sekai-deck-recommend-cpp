#ifndef MUSIC_METAS_H
#define MUSIC_METAS_H

#include "common/music-meta.h"

class MusicMetas {
public:
    std::string path;

    std::vector<MusicMeta> metas;

    MusicMetas(const std::string& path);
};

#endif // MUSIC_METAS_H    