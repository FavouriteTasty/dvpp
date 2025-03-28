#ifndef DVPP_RESOURCE_GUARD
#define DVPP_RESOURCE_GUARD
#include "acl/acl.h"
#include "utils.h"

class Resource {
private:
    int32_t deviceId = -1;
    aclrtStream stream = nullptr;
    aclrtRunMode runMode;
	aclrtContext context = nullptr;


public:
    Resource(int32_t did): deviceId(did){};
    void init();
    int32_t getDeviceId() const;
    aclrtRunMode getRunMode() const;
    aclrtContext getContext() const;
    ~Resource();
};

#endif
