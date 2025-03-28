#include <thread>
#include <sstream>
#include "resource.h"
#include "decode.h"


namespace {
	std::string outFolder = "./output";
	std::string filePath = "data/h264.mp4";
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
	
	try {
		r.init();
		std::cout << is_h265("data/h264.mp4") << std::endl;

		thread = std::thread(ThreadFunc, r.getContext());
		std::ostringstream oss;
		oss << thread.get_id();
		uint64_t tid = std::stoull(oss.str());
		log("create thread successfully, threadId = " + std::to_string(tid));

		CheckAndCreateFolder(outFolder);

		// dvpp init
		decoder.Init(tid);

		const int inputWidth = 1920;
		const int inputHeight = 1080;
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
			decoder.SetInput(inBufferDev, inBufferSize, inputWidth, inputHeight);

			decoder.Process();

			++count;
			rest_len = rest_len - 1;
			log("success to execute aclvdecSendFrame, count = " + std::to_string(count));
		}
		decoder.DestroyResource();
	}
	catch(const std::exception& e) {
		decoder.DestroyResource();
		std::cerr << e.what() << '\n';
	}
}
