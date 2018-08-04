//
// Created by ShaudXiao on 2018/8/4.
//

#ifndef FFMPEPY_VIDEO_RECODER_AUDIO_ENCODER_H
#define FFMPEPY_VIDEO_RECODER_AUDIO_ENCODER_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../common/live_audio_packet_queue.h"

#ifndef UINT64_C
#define UINT64_C(value)__CONCAT(value,ULL)
#endif

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/samplefmt.h"
#include "libavutil/common.h"
#include "libavutil/channel_layout.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
};

#ifndef PUBLISH_BITE_RATE
#define PUBLISH_BITE_RATE 64000
#endif


class AudioEncoder {

private:
    AVCodecContext *avCodecContext;
    AVFrame *encode_frame;
    int64_t audio_nex_pts;

    uint8_t **audio_sample_data;
    int audio_nb_samples;
    int audio_sample_size;

    int publishBitRate;
    int audioChannels;
    int audioSampleRate;

    int alloc_avframe();

    int alloc_audio_stream(const char *codec_name);

    typedef int (*fill_pcm_frame_callback)(int16_t *, int, int, double *, void *context);


    fill_pcm_frame_callback fillPcmFrameCallback;

    void *fillPCMFrameContext;


public:
    AudioEncoder();

    virtual ~AudioEncoder();

    int init(int bitRate, int channels, int sampleRate, const char *codecName,
             int (*fill_pcm_frame_callback)(int16_t *, int, int, double *, void *context),
    void *context);

    int encode(LiveAudioPacket **pAudioPacket);

    void destroy();
};

#endif //FFMPEPY_VIDEO_RECODER_AUDIO_ENCODER_H
