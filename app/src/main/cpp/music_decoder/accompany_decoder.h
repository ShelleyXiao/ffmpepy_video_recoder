//
// Created by ShaudXiao on 2018/8/3.
//

#ifndef ANDROIDFFMPEGRECODER_ACCOMPANY_DECODER_H
#define ANDROIDFFMPEGRECODER_ACCOMPANY_DECODER_H

#include <livecore/common/live_audio_packet_queue.h>
#include "CommonTools.h"
#include "libvideo_consumer/live_common_packet_pool.h"

#define LOG_TAG "AACAccompanyDecoder"

class AccompanyDecoder {
protected:
    bool seek_req;
    bool seek_resp;
    float seek_seconds;

    float actualSeekPosition;

public:

    AccompanyDecoder() {
        this->seek_req = false;
        this->seek_resp = false;
        this->seek_seconds = 0.0f;
    }

    ~AccompanyDecoder()();

    virtual int getMusicMeta(const char *fileString, int *metaData) = 0;

    //初始化这个decoder，即打开指定的mp3文件
    virtual void init(const char *fileString, int packetBufferSizeParam) = 0;

    virtual int init(const char *fileString) = 0;

    virtual void setPacketBufferSize(int packetBufferSizeParam) = 0;

    virtual LiveAudioPacket *decodePacket() = 0;

    //销毁这个decoder
    virtual void destroy() = 0;

    virtual int getSampleRate() = 0;

    void setSeekReq(bool seekReqParam) {
        seek_req = seekReqParam;
        if (seek_req) {
            seek_resp = false;
        }
    };

    bool hasSeekReq() {
        return seek_req;
    };

    bool hasSeekResp() {
        return seek_resp;
    };

    /** 设置到播放到什么位置，单位是秒，但是后边3位小数，其实是精确到毫秒 **/
    void setPosition(float seconds) {
        actualSeekPosition = -1;
        this->seek_seconds = seconds;
        this->seek_req = true;
        this->seek_resp = false;
    };

    float getActualSeekPosition() {
        float ret = actualSeekPosition;
        if (ret != -1) {
            actualSeekPosition = -1;
        }
        return ret;
    };

    virtual void seek_frame() = 0;
};

#endif //ANDROIDFFMPEGRECODER_ACCOMPANY_DECODER_H
