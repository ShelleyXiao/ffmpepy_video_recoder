//
// Created by ShaudXiao on 2018/8/3.
//

#include "live_common_packet_pool.h"

LiveCommonPacketPool::LiveCommonPacketPool() {
    accompanyBuffer = NULL;
    pthread_rwlock_init(&accompanyDropFrameRwlock, NULL);
}

void LiveCommonPacketPool::recordDropVideoFrame(int discardVideoPacketDuration) {
    LivePacketPool::recordDropVideoFrame(discardVideoPacketDuration);
    pthread_rwlock_wrlock(&accompanyDropFrameRwlock);
    totalDiscardVideoPacketDurationCopy += discardVideoPacketDuration;
    pthread_rwlock_unlock(&accompanyDropFrameRwlock);
}

LiveCommonPacketPool *LiveCommonPacketPool::instance = new LiveCommonPacketPool();

LiveCommonPacketPool *LiveCommonPacketPool::GetInstance() {
    return instance;
}

LiveCommonPacketPool::~LiveCommonPacketPool() {
    pthread_rwlock_destroy(&accompanyDropFrameRwlock);
}

/**************************start decoder accompany packet queue process*******************************************/
void LiveCommonPacketPool::initDecoderAccompanyPacketQueue() {
    const char *name = "decoder accompany packet queue";
    decoderAccompanyPacketQueue = new LiveAudioPacketQueue(name);
}

void LiveCommonPacketPool::abortDecoderAccompanyPacketQueue() {
    if (NULL != decoderAccompanyPacketQueue) {
        decoderAccompanyPacketQueue->abort();
    }
}

void LiveCommonPacketPool::destoryDecoderAccompanyPacketQueue() {
    if(NULL != decoderAccompanyPacketQueue){
        delete decoderAccompanyPacketQueue;
        decoderAccompanyPacketQueue = NULL;
    }
}

int LiveCommonPacketPool::getDecoderAccompanyPacket(LiveAudioPacket **audioPacket, bool block) {
    int result = -1;
    if(NULL != decoderAccompanyPacketQueue){
        result = decoderAccompanyPacketQueue->get(audioPacket, block);
    }
    return result;
}

void LiveCommonPacketPool::pushDecoderAccompanyPacketToQueue(LiveAudioPacket *audioPacket) {
    decoderAccompanyPacketQueue->put(audioPacket);
}

void LiveCommonPacketPool::clearDecoderAccompanyPacketToQueue() {
    decoderAccompanyPacketQueue->flush();
}

int LiveCommonPacketPool::geDecoderAccompanyPacketQueueSize() {
    return decoderAccompanyPacketQueue->size();
}


/**************************start accompany packet queue process****************************************/
void LiveCommonPacketPool::initAccompanyPacketQueue(int sampleRate, int channels) {
    const char* name = "accompanyPacket queue";
    accompanyPacketQueue = new LiveAudioPacketQueue(name);
    /** 初始化 Accompany 缓冲 Buffer **/
    accompanyBufferSize = sampleRate * channels * AUDIO_PACKET_DURATION_IN_SECS;
    accompanyBuffer = new short[accompanyBufferSize];
    accompanyBufferCursor = 0;
    totalDiscardVideoPacketDurationCopy = 0;
}


void LiveCommonPacketPool::abortAccompanyPacketQueue() {
    if (NULL != accompanyPacketQueue) {
        accompanyPacketQueue->abort();
    }
}

void LiveCommonPacketPool::destoryAccompanyPacketQueue() {
    if (NULL != accompanyPacketQueue) {
        delete accompanyPacketQueue;
        accompanyPacketQueue = NULL;
    }
    if (accompanyBuffer) {
        delete[] accompanyBuffer;
        accompanyBuffer = NULL;
    }
}

int LiveCommonPacketPool::getAccompanyPacket(LiveAudioPacket **accompanyPacket, bool block) {
    int result = -1;
    if (NULL != accompanyPacketQueue) {
        result = accompanyPacketQueue->get(accompanyPacket, block);
    }
}

void LiveCommonPacketPool::pushAccompanyPacketToQueue(LiveAudioPacket *accompanyPacket) {
    int audioPacketBufferCursor = 0;
    while (accompanyPacket->size > 0) {
        int audioBufferLength = accompanyBufferSize - accompanyBufferCursor;
        int length = MIN(audioBufferLength, accompanyPacket->size);
        memcpy(accompanyBuffer + accompanyBufferCursor,
               accompanyPacket->buffer + audioPacketBufferCursor,
               length * sizeof(short));
        accompanyPacket->size -= length;
        accompanyBufferCursor += length;
        audioPacketBufferCursor += length;
        if (accompanyBufferCursor == accompanyBufferSize) {
            LiveAudioPacket *targetAudioPacket = new LiveAudioPacket();
            targetAudioPacket->size = accompanyBufferSize;
            short *audioBuffer = new short[accompanyBufferSize];
            memcpy(audioBuffer, accompanyBuffer, accompanyBufferSize * sizeof(short));
            targetAudioPacket->buffer = audioBuffer;
            targetAudioPacket->position = accompanyPacket->position;
            targetAudioPacket->frameNum = accompanyPacket->frameNum;
            accompanyPacketQueue->put(targetAudioPacket);
            accompanyBufferCursor = 0;
        }
    }

    delete accompanyPacket;
    accompanyPacket = NULL;

}

int LiveCommonPacketPool::getAccompanyPacketQueueSize() {
    return accompanyPacketQueue->size();
}

bool LiveCommonPacketPool::discardAccompanyPacket() {
    bool ret = false;
    LiveAudioPacket *tempAudioPacket = NULL;
    int resultCoce = accompanyPacketQueue->get(&tempAudioPacket, true);
    if (resultCoce > 0) {
        delete tempAudioPacket;
        tempAudioPacket = NULL;
        pthread_rwlock_wrlock(&accompanyDropFrameRwlock);
        totalDiscardVideoPacketDurationCopy += (AUDIO_PACKET_DURATION_IN_SECS *1000.0F);
        pthread_rwlock_unlock(&accompanyDropFrameRwlock);
        ret = true;
    }

    return ret;
}


bool LiveCommonPacketPool::detectDiscardAccompanyPacket() {
    bool ret = false;
    pthread_rwlock_wrlock(&accompanyDropFrameRwlock);
    ret = totalDiscardVideoPacketDurationCopy >= (AUDIO_PACKET_DURATION_IN_SECS *1000.0f);
    pthread_rwlock_unlock(&accompanyDropFrameRwlock);
    return ret;
}
