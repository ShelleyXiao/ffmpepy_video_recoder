//
// Created by ShaudXiao on 2018/8/4.
//

#ifndef FFMPEPY_VIDEO_RECODER_RECORD_PROCESSOR_H
#define FFMPEPY_VIDEO_RECODER_RECORD_PROCESSOR_H

#include "CommonTools.h"
#include "libaudio_encoder/audio_process_encoder_adpater.h"
#include "libvideo_consumer/live_common_packet_pool.h"
#include "record_corrector.h"

class RecordProcessor {
private:
    int audioSampleRate;

    /** 当前这里面存储的audioSamples **/
    short *audioSamples;
    float audioSamplesTimeMills;
    int64_t dataAccumulateTimeMills;
    /** 当前的audioSamples的size大小 **/
    int audioSamplesCursor;

    /** 每一个要填入queue的buffer的大小 **/
    int audioBufferSize;
    int audioBufferTimeMills;
    LiveCommonPacketPool *packetPool;

    AudioProcessEnoderAdapter *audioEncoder;

    RecordCorrector *corrector;

    /** 纪录开始从Mic采集出来的时间 **/
    bool						isRecordingFlag;
    long long				startTimeMills;
    void cpyToAudioSamples(short* sourceBuffer, int cpyLength);

    LiveAudioPacket* getSilentDataPacket(int audioBufferSize);

    int correctRecordBuffer(int correctTimeMills);
public:
    RecordProcessor();
    ~RecordProcessor();

    //关于录音器录制出来的人声的处理
    void initAudioBufferSize(int sampleRate, int audioBufferSizeParam);
    int pushAudioBufferToQueue(short* samplesParam, int size);

    void flushAudioBufferToQueue();

    void destroy();
};


#endif //FFMPEPY_VIDEO_RECODER_RECORD_PROCESSOR_H
