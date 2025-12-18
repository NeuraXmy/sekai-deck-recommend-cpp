#include "static-data.h"

std::string staticDataDir = "./data";

void setStaticDataDir(const std::string &path)
{
    staticDataDir = path;
}

std::string getStaticDataDir()
{
    return staticDataDir;
}

std::string getStaticDataFilePath(const std::string &filename)
{
    return staticDataDir + "/" + filename;
}