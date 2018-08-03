//
// Created by ShaudXiao on 2018/8/3.
//

#ifndef ANDROIDFFMPEGRECODER_LIVE_AUDIO_PACKET_POOL_H
#define ANDROIDFFMPEGRECODER_LIVE_AUDIO_PACKET_POOL_H

#include "./live_audio_packet_queue.h"

class LiveAudioPacketPool {

protected:
    LiveAudioPacketPool();

    static LiveAudioPacketPool *instance;

    LiveAudioPacketQueue *audioPacketQueue;

public:
    static LiveAudioPacketPool *GetInstance();

    virtual  ~LiveAudioPacketPool();

    /** 人声的packet queue的所有操作 **/
    virtual void initAudioPacketQueue();

    virtual void abortAudioPacketQueue();

    virtual void destoryAudioPacketQueue();

    virtual int getAudioPacket(LiveAudioPacket **audioPacket, bool block);

    virtual void pushAudioPacketToQueue(LiveAudioPacket *audioPacket);

    virtual int getAudioPacketQueueSize();
};

#endif //ANDROIDFFMPEGRECODER_LIVE_AUDIO_PACKET_POOL_H
