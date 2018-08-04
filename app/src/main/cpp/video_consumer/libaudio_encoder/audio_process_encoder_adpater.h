//
// Created by ShaudXiao on 2018/8/4.
//

#ifndef FFMPEPY_VIDEO_RECODER_AUDIO_PROCESS_ENCODER_ADPATER_H
#define FFMPEPY_VIDEO_RECODER_AUDIO_PROCESS_ENCODER_ADPATER_H


#include "audio_encoder/audio_encoder_adapter.h"
#include "libmusic_merger/music_merger.h"
#include "libvideo_consumer/live_common_packet_pool.h"

class AudioProcessEnoderAdapter : public AudioEncoderAdapter {
public:
    AudioProcessEnoderAdapter();

    ~AudioProcessEnoderAdapter();

    void init(LivePacketPool *pcmPacketpool, int audioSampleRate, int aduioChannles,
              int audioBitRate, const char *audio_codec_name);

    virtual void destroy();

private:
    LiveCommonPacketPool *accompanyPacketPool;
    MusicMerger *musicMerger;

    virtual void discardAudioPacket();
    virtual int processAudio();


};

#endif //FFMPEPY_VIDEO_RECODER_AUDIO_PROCESS_ENCODER_ADPATER_H
