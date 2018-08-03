//
// Created by ShaudXiao on 2018/8/3.
//

#include "live_audio_packet_pool.h"

#define LOG_TAG "LiveAudioPacketPool"

LiveAudioPacketPool::LiveAudioPacketPool() {
    audioPacketQueue = NULL;
}

LiveAudioPacketPool *LiveAudioPacketPool::instance = new LiveAudioPacketPool();

LiveAudioPacketPool *LiveAudioPacketPool::GetInstance() {
    return instance;
}

LiveAudioPacketPool::~LiveAudioPacketPool() {

}

void LiveAudioPacketPool::initAudioPacketQueue() {
    const char *name = "audio packet aac data queue";
    audioPacketQueue = new LiveAudioPacketQueue(name);
}

void LiveAudioPacketPool::abortAudioPacketQueue() {
    if (NULL != audioPacketQueue) {
        this->audioPacketQueue->abort();
    }

}

void LiveAudioPacketPool::destoryAudioPacketQueue() {
    if (NULL != audioPacketQueue) {
        delete audioPacketQueue;
        audioPacketQueue = NULL;
    }
}

int LiveAudioPacketPool::getAudioPacket(LiveAudioPacket **audioPacket, bool block) {
    int result = -1;

    if (NULL != audioPacketQueue) {
        result = this->audioPacketQueue->get(audioPacket, block);
    }

    return result;
}

void LiveAudioPacketPool::pushAudioPacketToQueue(LiveAudioPacket *audioPacket) {
    if (NULL != audioPacketQueue) {
        this->audioPacketQueue->put(audioPacket);
    }
}

int LiveAudioPacketPool::getAudioPacketQueueSize() {
    return audioPacketQueue->size();
}
