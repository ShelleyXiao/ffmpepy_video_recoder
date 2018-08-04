//
// Created by ShaudXiao on 2018/8/4.
//

#ifndef FFMPEPY_VIDEO_RECODER_AUDIO_ENCODER_ADAPTER_H
#define FFMPEPY_VIDEO_RECODER_AUDIO_ENCODER_ADAPTER_H

#include <pthread.h>
#include "audio_encoder.h"
#include "../common/live_packet_pool.h"
#include "../common/live_audio_packet_pool.h"

/**
 * 1:负责组织PCM的Queue
 * 2:负责处理PCM
 * 3:负责打上时间戳输出到AAC的Queue
 *
 */

class AudioEncoderAdapter {
public:
    AudioEncoderAdapter();

    ~AudioEncoderAdapter();


    void init(LivePacketPool *pcmPacketpool, int audioSampleRate, int aduioChannles,
              int audioBitRate, const char *audio_codec_name);

    virtual void distroy();

protected:
    bool isEncoding;
    AudioEncoder *audioEncoder;

    pthread_t audioEncoderThread;


    /** 负责从pcmPacketPool中取数据, 调用编码器编码之后放入aacPacketPool中 **/
    static void *startEncodeThread(void *ptr);

    void startEncode();

    LiveAudioPacketPool *aacPacketPool;
    LivePacketPool *pcmPacketPool;

    /** 由于音频的buffer大小和每一帧的大小不一样，所以我们利用缓存数据的方式来 分次得到正确的音频数据 **/
    int packetBufferSize;
    short *packetBuffer;
    int packetBufferCursor;

    int audioSampleRate;
    int audioChannels;
    int audioBitRate;

    char *codecName;
    double packetBufferPresentationTimeMills;

    /** 安卓和iOS平台的输入声道是不一样的, 所以设置channelRatio这个参数 **/
    float channelRatio;

    int cpyToSamples(int16_t *samples, int samplesInShortCursor, int cpyPacketBufferSize,
                     double *presentationTimeMills);

    int getAudioPacket();

    virtual int processAudio() {
        return packetBufferSize;
    }

    virtual void discardAudioPacket();

public:
    int
    getAudioFrame(int16_t *samples, int frame_size, int nb_channels, double *presentationTimeMills);

};

#endif //FFMPEPY_VIDEO_RECODER_AUDIO_ENCODER_ADAPTER_H
