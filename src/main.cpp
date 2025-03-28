#include <thread>
#include <sstream>
#include "resource.h"
#include "decode.h"
#include "ffmpeg.h"


namespace {
	std::string outFolder = "./output";
	std::string filePath = "/home/dml/dvpp/data/output_0001.h264";
    bool runFlag = true;
}

void *ThreadFunc(aclrtContext sharedContext)
{
    if (sharedContext == nullptr) {
        error("sharedContext can not be nullptr");
        return ((void*)(-1));
    }
    log("use shared context for this thread");
    aclError ret = aclrtSetCurrentContext(sharedContext);
	assert(ret, "aclrtSetCurrentContext failed");

    log("thread start ");
    while (runFlag) {
        // Notice: timeout 1000ms
        (void)aclrtProcessReport(1000);
    }
    return (void*)0;
}

int main() {	
    aclrtStream stream;
    std::thread thread;

	Decoder decoder;
	Resource r(0);

	// size_t frameSize = 0;
	// getFrame(filePath.c_str(), 0, &frameSize);
	// std::cout << frameSize << std::endl;
	
	try {
		r.init();
		std::cout << is_h265(filePath) << std::endl;

		thread = std::thread(ThreadFunc, r.getContext());
		std::ostringstream oss;
		oss << thread.get_id();
		uint64_t tid = std::stoull(oss.str());
		log("create thread successfully, threadId = " + std::to_string(tid));

		CheckAndCreateFolder(outFolder);

		// dvpp init
		decoder.Init(tid);

		const int inputWidth = 1280;
		const int inputHeight = 720;
		int rest_len = 10;

		uint64_t count = 0;
		while (rest_len > 0) {
			void *inBufferDev = nullptr;
			uint32_t inBufferSize = 0;

			// read file to device memory
			if (!ReadFileToDeviceMem(filePath, inBufferDev, inBufferSize, r.getRunMode() == ACL_DEVICE)) {
				error("read file " + filePath +" to device mem failed.\n");
				decoder.DestroyResource();
			}
			std::cout << "buffer的大小：" << std::to_string(inBufferSize) << std::endl;

			decoder.SetInput(inBufferDev, inBufferSize, inputWidth, inputHeight);

			decoder.Process();

			++count;
			rest_len = rest_len - 1;
			log("success decode, count = " + std::to_string(count));
		}
		decoder.DestroyResource();
	}
	catch(const std::exception& e) {
		decoder.DestroyResource();
		std::cerr << e.what() << '\n';
	}
}
