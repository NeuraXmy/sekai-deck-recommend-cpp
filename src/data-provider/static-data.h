#ifndef STATIC_DATA_H
#define STATIC_DATA_H

#include <string>

void setStaticDataDir(const std::string& path);

std::string getStaticDataDir();

std::string getStaticDataFilePath(const std::string& filename);

#endif // STATIC_DATA_H