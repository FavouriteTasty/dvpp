#ifndef DVPP_DECODE_GUARD
#define DVPP_DECODE_GUARD
#include "acl/ops/acl_dvpp.h"
#include "utils.h"
#include "resource.h"
#include <string>

static aclrtRunMode g_runMode;

void callback(acldvppStreamDesc *input, acldvppPicDesc *output, void *userdata);

class Decoder {
private:
    Resource* resource;

    int count = 1;

    void *picOutBufferDev;
    uint32_t inBufferSize;
    void *inBufferDev;

    acldvppStreamDesc *streamInputDescription;
    aclvdecChannelDesc* vdecChannelDescription;
    acldvppPicDesc* picOutputDescription;

    acldvppStreamFormat enType;
    acldvppPixelFormat format;

    uint32_t inputWidth;
    uint32_t inputHeight;

public:
    Decoder(/* args */);
    void Init(uint64_t);
    void SetInput(void *inBufferDev, uint32_t inBufferSize, int inputWidth, int inputHeight);
    void CreatePicDescription(size_t size);
    void CreateStreamDescription();
    void DestroyPicDescription();
    void DestroyStreamDescription();

    void Process();
    void DestroyResource();
    ~Decoder();

};

#endif