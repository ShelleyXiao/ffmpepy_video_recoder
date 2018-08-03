//
// Created by ShaudXiao on 2018/8/2.
//

#ifndef ANDROIDFFMPEGRECODER_LIVE_AUDIO_PACKET_QUEUE_H
#define ANDROIDFFMPEGRECODER_LIVE_AUDIO_PACKET_QUEUE_H

#include <pthread.h>
#include "livecore/platform_dependent/platform_4_live_common.h"


typedef struct LiveAudioPacket {
    short *buffer;
    byte *data;
    int size;
    float position;
    long frameNum;

    LiveAudioPacket() {
        buffer = NULL;
        data = NULL;
        size = 0;
        position = -1;
    }

    ~LiveAudioPacket() {
        if (NULL != buffer) {
            //对于基本数据类型，这里delete delete[] 效果一样
            delete[] buffer;
            buffer = NULL;
        }
        if (NULL != data) {
            delete data;
            data = NULL;
        }
    }

} LiveAudioPacket;

typedef struct LiveAudioPacketList {
    LiveAudioPacket *audioPacket;
    LiveAudioPacketList *next;

    LiveAudioPacketList() {
        audioPacket = NULL;
        next = NULL;
    }

} LiveAudioPacketList;


inline void buildPacketFromBuffer(LiveAudioPacket *audioPacket, short *buffer, int sampleSize) {
    short *packetBuffer = new short[sampleSize];
    if (NULL != packetBuffer) {
        memcpy(packetBuffer, buffer, sampleSize * 2);
        audioPacket->buffer = packetBuffer;
        audioPacket->size = sampleSize;
    } else {
        audioPacket->size = -1;
    }
}

class LiveAudioPacketQueue {
public:
    LiveAudioPacketQueue();
    LiveAudioPacketQueue(const char *name);

    ~LiveAudioPacketQueue();

    void init();
    void flush();

    int put(LiveAudioPacket *audioPacket);

    int get(LiveAudioPacket **audioPacket, bool block);

    int size();

    void abort();

private:
    LiveAudioPacketList *mFirst;
    LiveAudioPacketList *mLast;
    int mNbPackets;
    bool mAbortRequest;
    pthread_mutex_t mLock;
    pthread_cond_t mCondition;
    const char *queueName;
};

#endif //ANDROIDFFMPEGRECODER_LIVE_AUDIO_PACKET_QUEUE_H
