//
// Created by ShaudXiao on 2018/8/2.
//

#include "live_audio_packet_queue.h"

LiveAudioPacketQueue::LiveAudioPacketQueue() {
    init();
}

LiveAudioPacketQueue::LiveAudioPacketQueue(const char *name) {
    init();
    queueName = name;
}

LiveAudioPacketQueue::~LiveAudioPacketQueue() {
    flush();
    pthread_mutex_destroy(&mLock);
    pthread_cond_destroy(&mCondition);
}

void LiveAudioPacketQueue::init() {
    pthread_mutex_init(&mLock, NULL);
    pthread_cond_init(&mCondition, NULL);
    mNbPackets = 0;
    mFirst = NULL;
    mLast = NULL;
    mAbortRequest = false;
}

void LiveAudioPacketQueue::flush() {
    LiveAudioPacketList *pkt, *pkt1;
    LiveAudioPacket *audioPacket;
    pthread_mutex_lock(&mLock);
    for (pkt = mFirst; pkt != NULL; pkt = pkt1) {
        pkt1 = pkt->next;
        audioPacket = pkt->audioPacket;
        if (NULL != audioPacket) {
            delete audioPacket;
            audioPacket = NULL;
        }
        delete pkt;
        pkt = NULL;
    }

    mLast = NULL;
    mFirst = NULL;
    mNbPackets = 0;


    pthread_mutex_unlock(&mLock);
}

int LiveAudioPacketQueue::put(LiveAudioPacket *audioPacket) {
    if (mAbortRequest) {
        delete audioPacket;
        return -1;
    }

    LiveAudioPacketList *pkt = new LiveAudioPacketList();
    if (!pkt) {
        return -1;
    }

    pkt->audioPacket = audioPacket;
    pkt->next = NULL;
    pthread_mutex_lock(&mLock);
    if (mLast == NULL) {
        mFirst = pkt;
    } else {
        mLast->next = pkt;
    }

    mLast = pkt;
    mNbPackets++;
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);

    return 0;
}

int LiveAudioPacketQueue::get(LiveAudioPacket **audioPacket, bool block) {
    LiveAudioPacketList *pkt;
    int ret;

    pthread_mutex_lock(&mLock);
    for (;;) {
        if (mAbortRequest) {
            ret = -1;
            break;
        }

        pkt = mFirst;
        if (NULL != pkt) {
            mFirst = mFirst->next;
            if (!mFirst) {
                mLast = NULL;
            }
            mNbPackets--;
            *audioPacket = pkt->audioPacket;
            delete pkt;
            pkt = NULL;
            ret = 1;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            pthread_cond_wait(&mCondition, &mLock);
        }
    }
    pthread_mutex_unlock(&mLock);


    return ret;
}

int LiveAudioPacketQueue::size() {
    pthread_mutex_lock(&mLock);
    int size = mNbPackets;
    pthread_mutex_unlock(&mLock);

    return size;
}

void LiveAudioPacketQueue::abort() {
    pthread_mutex_lock(&mLock);
    mAbortRequest = true;
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
}
