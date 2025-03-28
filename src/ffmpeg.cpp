#include "ffmpeg.h"
    
// 获取视频编码格式并解码指定帧
uint8_t* getFrame(const char* filename, int frameIndex, size_t* size) {
    // 初始化 FFmpeg
    avformat_network_init();

    // 打开文件
    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, filename, nullptr, nullptr) < 0) {
        fprintf(stderr, "Error: Could not open file\n");
        return nullptr;
    }

    // 获取流信息
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        fprintf(stderr, "Error: Could not find stream info\n");
        avformat_close_input(&fmt_ctx);
        return nullptr;
    }

    // 查找视频流
    int video_stream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_stream < 0) {
        fprintf(stderr, "Error: No video stream found\n");
        avformat_close_input(&fmt_ctx);
        return nullptr;
    }

    AVStream* stream = fmt_ctx->streams[video_stream];
    AVCodecParameters* codec_par = stream->codecpar;

    // 强制指定 H.264 解码器
    const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Error: H.264 decoder not available\n");
        avformat_close_input(&fmt_ctx);
        return nullptr;
    }

    // 初始化解码器上下文
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        fprintf(stderr, "Error: Could not allocate codec context\n");
        avformat_close_input(&fmt_ctx);
        return nullptr;
    }

    // 手动设置参数（跳过 avcodec_parameters_to_context）
    codec_ctx->width = codec_par->width;    // 从 codec_par 复制
    codec_ctx->height = codec_par->height;
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    // 打开解码器
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        fprintf(stderr, "Error: Could not open H.264 decoder\n");
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return nullptr;
    }

    AVPacket pkt;
    AVFrame* frame = av_frame_alloc();
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_stream) {
            avcodec_send_packet(codec_ctx, &pkt);
            if (avcodec_receive_frame(codec_ctx, frame) == 0) {
                // 提取RGB数据（此处简化为直接返回Y平面）
                uint8_t* data = (uint8_t*)malloc(frame->width * frame->height);
                memcpy(data, frame->data[0], frame->width * frame->height);
                *size = frame->width * frame->height;
                av_packet_unref(&pkt);
                av_frame_free(&frame);
                avcodec_free_context(&codec_ctx);
                avformat_close_input(&fmt_ctx);
                return data;
            }
        }
        av_packet_unref(&pkt);
    }
    return nullptr;
}

// 调用示例
// size_t size;
// uint8_t* frameData = getFrame("video.mp4", 100, &size);
// 使用完毕后需 free(frameData);