//
// Created by ShaudXiao on 2018/8/3.
//

#include "live_packet_pool.h"


#define LOG_TAG "LivePacketPool"

LivePacketPool::LivePacketPool() {
    audioPacketQueue = NULL;
    recordingVideoPacketQueue = NULL;
    statistics = NULL;
    buffer = NULL;

    pthread_rwlock_init(&mRwlock, NULL);
}

LivePacketPool::~LivePacketPool() {
    pthread_rwlock_destroy(&mRwlock);
}

//初始化静态成员
LivePacketPool *LivePacketPool::instance = new LivePacketPool();

LivePacketPool *LivePacketPool::GetInstance() {
    return instance;
}

/**************************start audio packet queue process**********************************************/
void LivePacketPool::initAudioPacketQueue(int audioSampleRate) {
    audioPacketQueue = new LiveAudioPacketQueue("audioPacket queue");
    this->audioSampleRate = audioSampleRate;
    this->channels = INPUT_CHANNEL_4_ANDROID;
    bufferSize = audioSampleRate * channels * AUDIO_PACKET_QUEUE_THRRESHOLD;
    buffer = new short[bufferSize];
    bufferCursor = 0;
}

void LivePacketPool::abortAudioPacketQueue() {
    if (audioPacketQueue != NULL) {
        this->audioPacketQueue->abort();
    }
}

void LivePacketPool::destoryAudioPacketQueue() {

    if (audioPacketQueue != NULL) {
        delete audioPacketQueue;
        audioPacketQueue = NULL;
    }

    if (buffer != NULL) {
        delete[] buffer;
        buffer = NULL;
    }
}

int LivePacketPool::getAudioPacket(LiveAudioPacket **audioPacket, bool block) {
    int result = -1;
    if (audioPacketQueue != NULL) {
        result = this->audioPacketQueue->get(audioPacket, block);
    }

    return result;
}

int LivePacketPool::getAudioPacketQueueSize() {
    return audioPacketQueue->size();
}

bool LivePacketPool::discardAudioPacket() {
    bool ret = false;
    LiveAudioPacket *tmpAudioPacket = NULL;
    int resultCode = this->audioPacketQueue->get(&tmpAudioPacket, true);
    if (tmpAudioPacket != NULL) {
        delete tmpAudioPacket;
        tmpAudioPacket = NULL;
        pthread_rwlock_wrlock(&mRwlock);
        totalDiscardVideoPacketDuration -= (AUDIO_PACKET_DURATION_IN_SECS *1000.0f);
        pthread_rwlock_unlock(&mRwlock);
        ret = true;
    }

    return ret;

}

void LivePacketPool::setStatistics(PublisherStatistics *statistics) {
    this->statistics = statistics;
}

bool LivePacketPool::detectDiscardAudioPacket() {
    bool ret = false;
    pthread_rwlock_wrlock(&mRwlock);
    ret = totalDiscardVideoPacketDuration >= (AUDIO_PACKET_DURATION_IN_SECS *1000.0f);
    pthread_rwlock_unlock(&mRwlock);
    return ret;
}

void LivePacketPool::pushAudioPacketToQueue(LiveAudioPacket *audioPacket) {
    if (this->audioPacketQueue != NULL) {
        int audioPacketBufferCursor = 0;
        while (audioPacket->size > 0) {
            int audioBufferLength = bufferSize - bufferCursor;
            int length = MIN(audioPacket->size, audioBufferLength);
            memcpy(buffer + bufferCursor, audioPacket->buffer + audioPacketBufferCursor, length * sizeof(short));
            audioPacket->size -= length;
            bufferCursor += length;
            audioPacketBufferCursor += length;

            if (bufferCursor == bufferSize) {
                LiveAudioPacket *targetAudioPacket = new LiveAudioPacket();
                targetAudioPacket->size = bufferSize;
                short *audioBuffer = new short[bufferSize];
                memcpy(audioBuffer, buffer, bufferSize * sizeof(short));
                targetAudioPacket->buffer = audioBuffer;

                audioPacketQueue->put(targetAudioPacket);
                bufferCursor = 0;
            }
        }

        delete audioPacket;
    }
}

/**************************end audio packet queue process**********************************************/

/**************************start decoder original song packet queue process*******************************************/

void LivePacketPool::initRecordingVideoPacketQueue() {
    if (NULL == recordingVideoPacketQueue) {
        const char *name = "recording video yuv frame packet queue";
        recordingVideoPacketQueue = new LiveVideoPacketQueue(name);
        totalDiscardVideoPacketDuration = 0;
        statistics = NULL;
        tempVideoPacket = NULL;
        tempVideoPacketRefCnt = 0;
    }
}

void LivePacketPool::abortRecordingVideoPacketQueue() {
    if (NULL != recordingVideoPacketQueue) {
        recordingVideoPacketQueue->abort();
    }
}

void LivePacketPool::destoryRecordingVideoPacketQueue() {
    if (NULL != recordingVideoPacketQueue) {
        delete recordingVideoPacketQueue;
        recordingVideoPacketQueue = NULL;
        if (tempVideoPacketRefCnt > 0) {
            delete tempVideoPacket;
            tempVideoPacket = NULL;
        }
    }
}

int LivePacketPool::getRecordingVideoPacket(LiveVideoPacket **videoPacket,
                                            bool block) {
    int result = -1;
    if (NULL != recordingVideoPacketQueue) {
        result = recordingVideoPacketQueue->get(videoPacket, block);
    }
    return result;
}

bool LivePacketPool::detectDiscardVideoPacket() {
    return recordingVideoPacketQueue->size() > VIDEO_PACKET_QUEUE_THRRESHOLD;
}

bool LivePacketPool::pushRecordingVideoPacketToQueue(LiveVideoPacket *videoPacket) {
    bool dropFrame = false;
    if (NULL != recordingVideoPacketQueue) {
        while (detectDiscardVideoPacket()) {
            dropFrame = true;
            int discardVideoFrameCnt = 0;
            int didcardVideoFrmeDuration =
                    recordingVideoPacketQueue->discardGOP(&discardVideoFrameCnt);
            if (didcardVideoFrmeDuration < 0) {
                break;
            }

            if (NULL != statistics) {
                statistics->discardVideoFrame(discardVideoFrameCnt);
            }
            this->recordDropVideoFrame(didcardVideoFrmeDuration);
        }
        //为了计算当前帧的Duration, 所以延迟一帧放入Queue中
        if (NULL != tempVideoPacket) {
            int packetDuration = videoPacket->timeMills - tempVideoPacket->timeMills;
            tempVideoPacket->duration = packetDuration;
            recordingVideoPacketQueue->put(tempVideoPacket);
            tempVideoPacketRefCnt = 0;
        }
        tempVideoPacket = videoPacket;
        tempVideoPacketRefCnt = 1;
        if (NULL != statistics) {
            statistics->pushVideoFrame();
        }
    }

    return dropFrame;
}

void LivePacketPool::recordDropVideoFrame(int discardVideoPacketDuration){
    pthread_rwlock_wrlock(&mRwlock);
    totalDiscardVideoPacketDuration+=discardVideoPacketDuration;
    pthread_rwlock_unlock(&mRwlock);
}

int LivePacketPool::getRecordingVideoPacketQueueSize() {
    if (NULL != recordingVideoPacketQueue) {
        return recordingVideoPacketQueue->size();
    }
    return 0;
}

void LivePacketPool::clearRecordingVideoPacketToQueue() {
    if (NULL != recordingVideoPacketQueue) {
        return recordingVideoPacketQueue->flush();
    }
}
/**************************end decoder original song packet queue process*******************************************/
