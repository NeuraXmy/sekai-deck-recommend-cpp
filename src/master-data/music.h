#ifndef MUSIC_H
#define MUSIC_H

#include "common/collection-utils.h"

struct Music {
    int id = 0;
    int seq = 0;
    int publishedAt = 0;
    std::string assetbundleName = "";

    static inline std::vector<Music> fromJsonList(const json& jsonData) {
        std::vector<Music> musics;
        for (const auto& item : jsonData) {
            Music music;
            music.id = item.value("id", 0);
            music.seq = item.value("seq", 0);
            music.publishedAt = item.value("publishedAt", 0);
            music.assetbundleName = item.value("assetbundleName", "");
            musics.push_back(music);
        }
        return musics;
    }
};


#endif  // MUSIC_H