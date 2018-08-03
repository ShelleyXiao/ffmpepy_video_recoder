//
// Created by ShaudXiao on 2018/8/3.
//

#ifndef ANDROIDFFMPEGRECODER_LIVE_PACKET_POOL_H
#define ANDROIDFFMPEGRECODER_LIVE_PACKET_POOL_H

#include "./live_audio_packet_queue.h"
#include "./live_video_packet_queue.h"
#include "./publisher_statistics.h"

#define AUDIO_PACKET_QUEUE_THRRESHOLD                                        15
#define VIDEO_PACKET_QUEUE_THRRESHOLD                                        60

#define    INPUT_CHANNEL_4_ANDROID                                                1
#define    INPUT_CHANNEL_4_IOS                                                    2

#define AUDIO_PACKET_DURATION_IN_SECS                   0.04f

class LivePacketPool {
protected:
    LivePacketPool();

    static LivePacketPool *instance;

    /** 边录边合---人声的packet queue **/
    LiveAudioPacketQueue *audioPacketQueue;
    int audioSampleRate;
    int channels;

    /** 边录边合成---视频的YUV数据帧的queue **/
    LiveVideoPacketQueue *recordingVideoPacketQueue;

private:

    int totalDiscardVideoPacketDuration;
    pthread_rwlock_t mRwlock;

    int bufferSize;
    short *buffer;
    int bufferCursor;

    bool detectDiscardVideoPacket();

    /** 为了计算每一帧的时间长度 **/
    LiveVideoPacket *tempVideoPacket;
    int tempVideoPacketRefCnt;

protected:
    PublisherStatistics *statistics;

    virtual void recordDropVideoFrame(int discardVideoPacketSize);

public:
    static LivePacketPool *GetInstance(); //工厂方法(用来获得实例)
    virtual ~LivePacketPool();

    /** 由于丢帧会有丢帧的统计 **/
    void setStatistics(PublisherStatistics *statistics);

    /** 人声的packet queue的所有操作 **/
    virtual void initAudioPacketQueue(int audioSampleRate);

    virtual void abortAudioPacketQueue();

    virtual void destoryAudioPacketQueue();

    virtual int getAudioPacket(LiveAudioPacket **audioPacket, bool block);

    virtual void pushAudioPacketToQueue(LiveAudioPacket *audioPacket);

    virtual int getAudioPacketQueueSize();

    bool discardAudioPacket();

    bool detectDiscardAudioPacket();

    /** 解码出来的伴奏的queue的所有操作 **/
    void initRecordingVideoPacketQueue();

    void abortRecordingVideoPacketQueue();

    void destoryRecordingVideoPacketQueue();

    int getRecordingVideoPacket(LiveVideoPacket **videoPacket, bool block);

    bool pushRecordingVideoPacketToQueue(LiveVideoPacket *videoPacket);

    int getRecordingVideoPacketQueueSize();

    void clearRecordingVideoPacketToQueue();

};

#endif //ANDROIDFFMPEGRECODER_LIVE_PACKET_POOL_H
