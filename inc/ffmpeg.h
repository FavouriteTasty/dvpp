#ifndef DVPP_FFMPEG_GUARD
#define DVPP_FFMPEG_GUARD

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}

uint8_t* getFrame(const char* filename, int frameIndex, size_t* size);

#endif