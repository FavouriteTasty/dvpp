#include "decode.h"

void callback(acldvppStreamDesc *input, acldvppPicDesc *output, void *userdata)
{
    uint64_t frameIndex = 0;
    if (userdata != nullptr) {
        frameIndex = *((uint64_t *)userdata);
        log("start processing callback, frame index is " + std::to_string(frameIndex));
        free(userdata);
        userdata = nullptr;
    }
    if (input != nullptr) {
        void *vdecInBufferDev = acldvppGetStreamDescData(input);
        if (vdecInBufferDev != nullptr) {
            aclError ret = acldvppFree(vdecInBufferDev);
            assert(ret, "fail to free input stream desc data");
        }
        aclError ret = acldvppDestroyStreamDesc(input);
        assert(ret, "fail to destroy input stream desc");
    }

    if (output != nullptr) {
        void *vdecOutBufferDev = acldvppGetPicDescData(output);
        int retCode = acldvppGetPicDescRetCode(output);
        if (retCode != 0) {
            error("vdec decode frame failed, retCode = " + std::to_string(retCode));
            if (vdecOutBufferDev != nullptr) {
                aclError ret = acldvppFree(vdecOutBufferDev);
                assert(ret, "fail to free output pic desc data");
            }
            aclError ret = acldvppDestroyPicDesc(output);
            assert(ret, "fail to destroy output pic desc");
            return;
        }

        if (vdecOutBufferDev != nullptr) {
            // uint32_t size = acldvppGetPicDescSize(output);
            // std::string fileNameSave = "outdir/image" + std::to_string(frameIndex);
            // if (!Utils::WriteDeviceMemoryToFile(fileNameSave.c_str(), vdecOutBufferDev, size)) {
            //     ERROR_LOG("write file failed.");
            // }

            // aclError ret = acldvppFree(vdecOutBufferDev);
            // if (ret != ACL_SUCCESS) {
            //     ERROR_LOG("fail to free output pic desc data, errorCode = %d", static_cast<int32_t>(ret));
            // }
        }
        aclError ret = acldvppDestroyPicDesc(output);
        assert(ret, "fail to destroy output pic desc");
    }

    log("success to process vdec callback " + std::to_string(frameIndex));
}

Decoder::Decoder(): vdecChannelDescription(nullptr), streamInputDescription(nullptr),
picOutputDescription(nullptr), picOutBufferDev(nullptr),
inBufferDev(nullptr), inBufferSize(0), inputWidth(0),
inputHeight(0), enType(H264_MAIN_LEVEL), format(PIXEL_FORMAT_YUV_SEMIPLANAR_420)
{
}

void Decoder::Init(uint64_t threadId)
{
    // 创建视频码流处理通道时的通道描述信息，设置视频处理通道描述信息的属性
    vdecChannelDescription = aclvdecCreateChannelDesc();
    assert(vdecChannelDescription != nullptr, "fail to create vdec channel desc");
    aclError ret = aclvdecSetChannelDescChannelId(vdecChannelDescription, 10);
    assert(ret, "fail to set vdec ChannelId");
    ret = aclvdecSetChannelDescThreadId(vdecChannelDescription, threadId);
    assert(ret, "fail to create threadId");
    ret = aclvdecSetChannelDescCallback(vdecChannelDescription, callback);
    assert(ret, "fail to set vdec Callback");
    ret = aclvdecSetChannelDescEnType(vdecChannelDescription, static_cast<acldvppStreamFormat>(enType));
    assert(ret, "fail to set vdec EnType");
    ret = aclvdecSetChannelDescOutPicFormat(vdecChannelDescription, static_cast<acldvppPixelFormat>(format));
    assert(ret, "fail to set vdec OutPicFormat");
    // 创建视频码流处理的通道
    ret = aclvdecCreateChannel(vdecChannelDescription);
    assert(ret, "fail to create vdec channel");
    log("vdec init resource success");
}

void Decoder::SetInput(void *inBufferDev, uint32_t inBufferSize, int inputWidth, int inputHeight) {
    this->inBufferDev = inBufferDev;
    this->inBufferSize = inBufferSize;
    this->inputWidth = inputWidth;
    this->inputHeight = inputHeight;
}

void Decoder::CreatePicDescription(size_t size) {
    // Malloc output device memory
    aclError ret = acldvppMalloc(&picOutBufferDev, size);
    assert(ret, "aclrtMalloc failed");
    picOutputDescription = acldvppCreatePicDesc();
    assert(picOutputDescription != nullptr, "fail to create output pic desc");
    ret = acldvppSetPicDescData(picOutputDescription, picOutBufferDev);
    assert(ret, "fail to set PicDescData");
    ret = acldvppSetPicDescSize(picOutputDescription, size);
    assert(ret, "fail to set PicDescSize");
    ret = acldvppSetPicDescFormat(picOutputDescription, format);
    assert(ret, "fail to set PicDescHeight");
}

void Decoder::CreateStreamDescription() {
    // create input stream desc
    streamInputDescription = acldvppCreateStreamDesc();
    assert(streamInputDescription != nullptr, "fail to create input stream desc");

    aclError ret = acldvppSetStreamDescData(streamInputDescription, inBufferDev);
    assert(ret, "fail to set data for stream desc");
    // set size for dvpp stream desc
    ret = acldvppSetStreamDescSize(streamInputDescription, inBufferSize);
    assert(ret, "fail to set size for stream desc");
}

void Decoder::DestroyPicDescription() {
    if (picOutBufferDev != nullptr) {
        (void)acldvppFree(picOutBufferDev);
        picOutBufferDev = nullptr;
    }
    if (picOutputDescription != nullptr) {
        (void)acldvppDestroyPicDesc(picOutputDescription);
        picOutputDescription = nullptr;
    }
}

void Decoder::DestroyStreamDescription() {
    if (inBufferDev != nullptr) {
        (void)acldvppFree(inBufferDev);
        inBufferDev = nullptr;
    }
    if (streamInputDescription != nullptr) {
        (void)acldvppDestroyStreamDesc(streamInputDescription);
        streamInputDescription = nullptr;
    }
}

void Decoder::Process() {
    CreateStreamDescription();
    size_t DataSize = (inputWidth * inputHeight * 3) / 2; // yuv format size
    CreatePicDescription(DataSize);

    // set frame index, callback function can use it
    static uint64_t index = 0;
    uint64_t *frameIndex = (uint64_t *)malloc(sizeof(uint64_t));
    if (frameIndex != nullptr) {
        *frameIndex = index++;
    }

    std::cout << vdecChannelDescription << " " << streamInputDescription << " " << picOutputDescription << " " << frameIndex << std::endl;
    // send vdec frame
    aclError ret = aclvdecSendFrame(vdecChannelDescription, streamInputDescription,
                                    picOutputDescription, nullptr, static_cast<void *>(frameIndex));

    if (ret != ACL_SUCCESS) {
        error("fail to send frame", ret);
        DestroyStreamDescription();
        DestroyPicDescription();
        if (frameIndex != nullptr) {
            free(frameIndex);
            frameIndex = nullptr;
        }
    }
}

void Decoder::DestroyResource() {
    if (vdecChannelDescription != nullptr) {
        aclError ret = aclvdecDestroyChannel(vdecChannelDescription);
        assert(ret, "acldvppDestroyChannel failed");
        (void)aclvdecDestroyChannelDesc(vdecChannelDescription);
        vdecChannelDescription = nullptr;
    }
}

Decoder::~Decoder() {
    DestroyPicDescription();
    DestroyResource();
    DestroyStreamDescription();
}
