//
// Created by ShaudXiao on 2018/8/4.
//

#include "audio_process_encoder_adpater.h"

#define LOG_TAG "AudioProcessEnoderAdapter"

AudioProcessEnoderAdapter::AudioProcessEnoderAdapter() {
    musicMerger = NULL;
    accompanyPacketPool = NULL;
}

AudioProcessEnoderAdapter::~AudioProcessEnoderAdapter() {

}

void AudioProcessEnoderAdapter::init(LivePacketPool *pcmPacketpool, int audioSampleRate,
                                     int aduioChannles, int audioBitRate,
                                     const char *audio_codec_name) {
    this->accompanyPacketPool = LiveCommonPacketPool::GetInstance();
    musicMerger = new MusicMerger();
    AudioEncoderAdapter::init(pcmPacketpool, audioSampleRate, aduioChannles, audioBitRate, audio_codec_name);
    this->channelRatio = 2.0f;
}

void AudioProcessEnoderAdapter::destroy() {
    accompanyPacketPool->abortAccompanyPacketQueue();
    AudioEncoderAdapter::distroy();
    accompanyPacketPool->destoryAccompanyPacketQueue();
    if (NULL != musicMerger) {
        musicMerger->stop();
        delete musicMerger;
        musicMerger = NULL;
    }
}

void AudioProcessEnoderAdapter::discardAudioPacket() {
    AudioEncoderAdapter::discardAudioPacket();
    while(accompanyPacketPool->detectDiscardAudioPacket()) {
        if(!accompanyPacketPool->discardAudioPacket()) {
            break;
        }
    }
}

int AudioProcessEnoderAdapter::processAudio() {
    int ret = packetBufferSize;
    LiveAudioPacket *accompanyPacket = NULL;
    if(accompanyPacketPool->getAccompanyPacket(&accompanyPacket, true) < 0) {
        return -1;
    }

    if(NULL != accompanyPacket) {
        int accompanySampleSize = accompanyPacket->size;
        short *accompanyBuffer = accompanyPacket->buffer;
        long frameNum = accompanyPacket->frameNum;
        ret = musicMerger->mixtureMusicBuffer(frameNum, accompanyBuffer, accompanySampleSize, packetBuffer, packetBufferSize);
        delete  accompanyPacket;
        accompanyPacket = NULL;
    }

    return  ret;
}
