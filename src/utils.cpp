#include "utils.h"

bool is_h265(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;

    unsigned char buffer[4];
    file.read(reinterpret_cast<char*>(buffer), 4);

    // 检查H.265的起始头（简化版）
    if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0x00 && buffer[3] == 0x01) {
        file.read(reinterpret_cast<char*>(buffer), 1);
        return (buffer[0] >> 1) == 0x20; // VPS NALU类型
    }
    return false;
}

bool assert(aclError ret, const std::string &msg) {
    if (ACL_SUCCESS == ret) {
        return true;
    }
    throw std::runtime_error(msg + ", code = " + std::to_string(static_cast<int32_t>(ret)));
}

bool assert(bool v, const std::string &msg) {
    if (v) {
        return true;
    }
    throw std::runtime_error(msg);
}

void log(const std::string& msg) {
    std::cout << "[LOG] " << msg << std::endl;
}

void error(const std::string &msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
}

void error(const std::string &msg, aclError ret) {
    std::cerr << "[ERROR] " << msg << " code -> " << std::to_string(static_cast<int32_t>(ret))<< std::endl;
}

bool CheckAndCreateFolder(const std::string &folderName) {
    log( "start check result fold: " + folderName);
#if defined(_MSC_VER)
    DWORD ret = GetFileAttributes((LPCSTR)foldName);
    if (ret == INVALID_FILE_ATTRIBUTES) {
        BOOL flag = CreateDirectory((LPCSTR)foldName, nullptr);
        if (flag) {
            INFO_LOG("make dir successfully.");
        } else {
            INFO_LOG("make dir errorly.");
            return FAILED;
        }
    }
#else
    if (access(folderName.c_str(), 0) == -1) {
        int flag = mkdir(folderName.c_str() , 0777);
        if (flag == 0)
        {
            log("make dir successfully.");
        } else {
            error("make dir errorly.");
            return false;
        }
    }
#endif
    log("check result success, fold exist");
    return true;
}

bool ReadFileToDeviceMem(const std::string &fileName, void *&dataDev, uint32_t &dataSize, bool isDevice) {
    // read data from file.
    FILE *fp = fopen(fileName.c_str(), "rb");
    if (fp == nullptr) {
        error("open file " + fileName + " failed");
        return false;
    }

    fseek(fp, 0, SEEK_END);
    long fileLenLong = ftell(fp);
    if (fileLenLong <= 0) {
        error("file " + fileName + " len is invalid.");
        fclose(fp);
        return false;
    }
    fseek(fp, 0, SEEK_SET);

    auto fileLen = static_cast<uint32_t>(fileLenLong);
    dataSize = fileLen;
    size_t readSize;
    // Malloc input device memory
    auto aclRet = acldvppMalloc(&dataDev, dataSize);
    assert(aclRet, "acl malloc dvpp data failed, dataSize = " + std::to_string(dataSize));

    if (!isDevice) {
        void *dataHost = nullptr;
        auto aclRet = aclrtMallocHost(&dataHost, fileLen);
        assert(aclRet, "acl malloc host data buffer failed, dataSize = " + std::to_string(fileLen));
        if (dataHost == nullptr) {
            (void)acldvppFree(dataDev);
            dataDev = nullptr;
            fclose(fp);
            return false;
        }

        readSize = fread(dataHost, 1, fileLen, fp);
        if (readSize < fileLen) {
            error("need read file " + fileName + " " + std::to_string(fileLen) + " bytes, but only "+ std::to_string(readSize) + " read.");
            (void)aclrtFreeHost(dataHost);
            (void)acldvppFree(dataDev);
            dataDev = nullptr;
            fclose(fp);
            return false;
        }

        // copy input to device memory
        aclRet = aclrtMemcpy(dataDev, dataSize, dataHost, fileLen, ACL_MEMCPY_HOST_TO_DEVICE);
        assert(aclRet, "acl memcpy data to dev failed, fileLen = " + std::to_string(fileLen));
        if (aclRet != ACL_SUCCESS) {
            (void)aclrtFreeHost(dataHost);
            (void)acldvppFree(dataDev);
            dataDev = nullptr;
            fclose(fp);
            return false;
        }
        (void)aclrtFreeHost(dataHost);
    } else {
        readSize = fread(dataDev, 1, fileLen, fp);
        if (readSize < fileLen) {
            error("need read file " + fileName + " " + std::to_string(fileLen) + " bytes, but only "+ std::to_string(readSize) + " read.");
            (void)acldvppFree(dataDev);
            dataDev = nullptr;
            fclose(fp);
            return false;
        }
    }

    fclose(fp);
    return true;
}

bool WriteDeviceMemoryToFile(const std::string& fileName, void *dataDev, uint32_t dataSize, bool isDevice) {
    if (dataDev == nullptr) {
        error("dataDev is nullptr!");
        return false;
    }

    // copy output to host memory
    void *data = nullptr;
    aclError aclRet;
    if (!isDevice) {
        aclRet = aclrtMallocHost(&data, dataSize);
        if (data == nullptr) {
            assert(aclRet, "malloc host data buffer failed. dataSize = " + std::to_string(dataSize));
            return false;
        }
        aclRet = aclrtMemcpy(data, dataSize, dataDev, dataSize, ACL_MEMCPY_DEVICE_TO_HOST);
        if (aclRet != ACL_SUCCESS) {
            assert(aclRet, "acl memcpy data to host failed, dataSize = " + std::to_string(dataSize));
            (void)aclrtFreeHost(data);
            return false;
        }
    } else {
        data = dataDev;
    }

    FILE *outFileFp = fopen(fileName.c_str(), "wb+");
    if (outFileFp == nullptr) {
        error("fopen out file failed: " + fileName);
        (void)aclrtFreeHost(data);
        return false;
    }

    bool ret = true;
    size_t writeRet = fwrite(data, 1, dataSize, outFileFp);
    if (writeRet != dataSize) {
        error("need write " + std::to_string(dataSize) + " bytes to " + fileName + " , but only write " + std::to_string(writeRet) + " bytes\n");
        ret = false;
    }

    if (!isDevice) {
        (void)aclrtFreeHost(data);
    }
    fflush(outFileFp);
    fclose(outFileFp);
    return ret;
}
