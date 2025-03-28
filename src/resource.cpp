#include "resource.h"

void Resource::init() {
    aclError ret = aclInit(nullptr);
    assert(ret, "acl init");
	ret = aclrtSetDevice(deviceId);
    assert(ret, "set device");
    ret = aclrtCreateStream(&stream);
    assert(ret, "create steam");
    ret = aclrtGetRunMode(&runMode);
    assert(ret, "get run mode");
    std::string mode = (runMode == aclrtRunMode::ACL_DEVICE) ? "device" : "host";
    log("Run mode: " + mode);
    ret = aclrtCreateContext(&context, deviceId);
    assert(ret, "acl create context failed, deviceId = " + std::to_string(deviceId));
}

int32_t Resource::getDeviceId() const {
    return deviceId;
}

aclrtRunMode Resource::getRunMode() const {
    return runMode;
}

aclrtContext Resource::getContext() const {
    return context;
}

Resource::~Resource() {
    if (deviceId < 0 || stream == nullptr) return; 
    aclError ret = aclrtDestroyContext(context);
    assert(ret, "destroy context");
    ret = aclrtDestroyStream(stream);
    assert(ret, "destroy stream");
    ret = aclrtResetDevice(deviceId);
    assert(ret, "reset device");


	aclFinalize();
}

