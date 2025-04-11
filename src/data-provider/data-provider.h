#ifndef DATA_PROVIDER_H
#define DATA_PROVIDER_H

#include <memory>

#include "data-provider/master-data.h"
#include "data-provider/user-data.h"
#include "data-provider/music-metas.h"

enum class Region { JP, EN, KR, TW, CN };

struct DataProvider {
    Region region;
    std::shared_ptr<MasterData> masterData;
    std::shared_ptr<UserData> userData;
    std::shared_ptr<MusicMetas> musicMetas;
};

#endif // DATA_PROVIDER_H