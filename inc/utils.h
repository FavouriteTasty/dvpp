#ifndef DVPP_UTILS_GUARD
#define DVPP_UTILS_GUARD

#include "acl/acl.h"
#include "acl/ops/acl_dvpp.h"
#include <string>
#include <iostream>
#include <fstream>
#include <iostream>
#if defined(_MSC_VER)
    #include <windows.h>
    #include <io.h>
#else
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
#endif


bool is_h265(const std::string& filename);

bool assert(aclError ret, const std::string& msg);
bool assert(bool v, const std::string &msg);

void log(const std::string& msg);
void error(const std::string& msg);
void error(const std::string& msg, aclError ret);

bool CheckAndCreateFolder(const std::string& folderName);
bool ReadFileToDeviceMem(const std::string& fileName, void *&dataDev, uint32_t &dataSize, bool isDevice);
bool WriteDeviceMemoryToFile(const std::string& fileName, void *dataDev, uint32_t dataSize, bool isDevice);

#endif