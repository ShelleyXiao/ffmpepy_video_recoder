//
// Created by ShaudXiao on 2018/8/4.
//

#include "record_processor.h"

#define LOG_TAG "RecordProcessor "

RecordProcessor::RecordProcessor() {
    audioSamples = NULL;
    audioEncoder = NULL;
    isRecordingFlag = false;
    dataAccumulateTimeMills = 0;
}

RecordProcessor::~RecordProcessor() {

}

void RecordProcessor::cpyToAudioSamples(short *sourceBuffer, int cpyLength) {
    if (audioSamplesCursor == 0) {
        audioBufferTimeMills = currentTimeMills() - startTimeMills;
    }
    memcpy(audioSamples + audioSamplesCursor, sourceBuffer, cpyLength * sizeof(short));
}

LiveAudioPacket *RecordProcessor::getSilentDataPacket(int audioBufferSize) {
    LiveAudioPacket *audioPacket = new LiveAudioPacket();
    audioPacket->buffer = new short[audioBufferSize];
    memset(audioPacket->buffer, 0, audioBufferSize * sizeof(short));
    audioPacket->size = audioBufferSize;
    return audioPacket;
}

int RecordProcessor::correctRecordBuffer(int correctTimeMills) {
    int correctBufferSize = ((float) correctTimeMills / 1000.0f) * audioSampleRate;
    LiveAudioPacket *audioPacket = this->getSilentDataPacket(correctBufferSize);
    packetPool->pushAudioPacketToQueue(audioPacket);
    LiveAudioPacket *accompanyPacket = getSilentDataPacket(correctBufferSize);
    packetPool->pushAccompanyPacketToQueue(accompanyPacket);
    dataAccumulateTimeMills += correctTimeMills;

    return 0;
}

void RecordProcessor::initAudioBufferSize(int sampleRate, int audioBufferSizeParam) {
    audioSamplesCursor = 0;
    audioBufferSize = audioBufferSizeParam;
    audioSampleRate = sampleRate;
    audioSamples = new short[audioBufferSize];
    packetPool = LiveCommonPacketPool::GetInstance();
    corrector = new RecordCorrector();
    audioBufferTimeMills = (float) audioBufferSize * 1000.0f / (float) audioSampleRate;
}

int RecordProcessor::pushAudioBufferToQueue(short *samplesParam, int size) {
    if (size < 0) {
        return size;
    }

    if (!isRecordingFlag) {
        isRecordingFlag = true;
        startTimeMills = currentTimeMills();
    }

    int sampleCursor = 0;
    int sampleCnt = size;
    while (sampleCnt > 0) {
        if ((audioSamplesCursor + sampleCnt) < audioBufferSize) {
            this->cpyToAudioSamples(samplesParam + sampleCursor, sampleCnt);
            audioSamplesCursor += sampleCnt;
            sampleCursor += sampleCnt;
            sampleCnt = 0;

        } else {
            int subFullSize = audioBufferSize - audioSamplesCursor;
            this->cpyToAudioSamples(samplesParam + sampleCursor, subFullSize);
            audioSamplesCursor += subFullSize;
            sampleCursor += subFullSize;
            sampleCnt -= subFullSize;
            flushAudioBufferToQueue();
        }
    }

    return size;
}

void RecordProcessor::flushAudioBufferToQueue() {
    if (audioSamplesCursor > 0) {
        if (NULL == audioEncoder) {
            audioEncoder = new AudioProcessEnoderAdapter();
            int audioChannels = 2;
            int audioBitRate = 64 * 1024;
            const char *audioCodecName = "libfdk_aac";
            audioEncoder->init(packetPool, audioSampleRate, audioChannels, audioBitRate,
                               audioCodecName);

        }
        short *packetBuffer = new short[audioSamplesCursor];
        if (NULL == packetBuffer) {
            return;
        }

        memcpy(packetBuffer, audioSamples, audioSamplesCursor * sizeof(short));
        LiveAudioPacket *packet = new LiveAudioPacket();
        packet->buffer = packetBuffer;
        packet->size = audioSamplesCursor;
        packetPool->pushAudioPacketToQueue(packet);
        audioSamplesCursor = 0;
        dataAccumulateTimeMills += audioBufferTimeMills;
        int correctDurationInTimeMills = 0;
        if (corrector->detectNeedCorrect(dataAccumulateTimeMills, audioSamplesTimeMills,
                                         &correctDurationInTimeMills)) {
            //检测到有问题了, 需要进行修复
            this->correctRecordBuffer(correctDurationInTimeMills);
        }

    }
}

void RecordProcessor::destroy() {
    if (NULL != audioSamples) {
        delete[] audioSamples;
        audioSamples = NULL;
    }
    if (NULL != audioEncoder) {
        audioEncoder->destroy();
        delete audioEncoder;
        audioEncoder = NULL;
    }
    if (NULL != corrector) {
        delete corrector;
        corrector = NULL;
    }
}
