//
// Created by ShaudXiao on 2018/8/4.
//

#include "audio_encoder.h"

#define LOG_TAG "AudioEncoder"

AudioEncoder::AudioEncoder() {
    encode_frame = NULL;
    avCodecContext = NULL;
    audio_nex_pts = 0.0f;
}

AudioEncoder::~AudioEncoder() {

}

int AudioEncoder::init(int bitRate, int channels, int sampleRate, const char *codecName,
                       int (*fill_pcm_frame_callback)(int16_t *, int, int, double *, void *),
                       void *context) {
    this->publishBitRate = bitRate;
    this->audioChannels = channels;
    this->audioSampleRate = sampleRate;
    this->fillPcmFrameCallback = fill_pcm_frame_callback;
    this->fillPCMFrameContext = context;

    av_register_all();
//    avcodec_register_all();

    this->alloc_audio_stream(codecName);
    this->alloc_avframe();

    return 1;
}

int AudioEncoder::alloc_avframe() {
    int ret = 0;
    encode_frame = avcodec_alloc_frame();
    if (!encode_frame) {
        LOGI("Could not allocate audio frame\n");
        return -1;
    }
    encode_frame->nb_samples = avCodecContext->frame_size;
    encode_frame->format = avCodecContext->sample_fmt;
    encode_frame->channel_layout = avCodecContext->channel_layout;
    encode_frame->sample_rate = avCodecContext->sample_rate;

    audio_nb_samples = avCodecContext->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE ? 10240
                                                                                           : avCodecContext->frame_size;
    int src_samples_linesize;
    ret = av_samples_alloc_array_and_samples(&audio_sample_data, &src_samples_linesize,
                                             avCodecContext->channels, audio_nb_samples,
                                             avCodecContext->sample_fmt, 0);
    if (ret < 0) {
        LOGI("Could not allocate source samples\n");
        return -1;
    }
    audio_sample_size = av_samples_get_buffer_size(NULL, avCodecContext->channels,
                                                   audio_nb_samples, avCodecContext->sample_fmt, 0);
    return 0;
}

int AudioEncoder::alloc_audio_stream(const char *codec_name) {
    AVCodec *codec = avcodec_find_decoder_by_name(codec_name);
    if (!codec) {
        LOGI("Couldn't find a valid audio codec By Codec Name %s", codec_name);
        return -1;
    }

    avCodecContext = avcodec_alloc_context3(codec);
    avCodecContext->codec_type = AVMEDIA_TYPE_AUDIO;
    avCodecContext->sample_rate = this->audioSampleRate;
    avCodecContext->sample_fmt = AV_SAMPLE_FMT_S16;
    if (publishBitRate > 0) {
        avCodecContext->bit_rate = publishBitRate;
    } else {
        avCodecContext->bit_rate = PUBLISH_BITE_RATE;
    }

    LOGI("audioChannels is %d", audioChannels);
    LOGI("AV_SAMPLE_FMT_S16 is %d", AV_SAMPLE_FMT_S16);
    avCodecContext->channel_layout = this->audioChannels == 1 ?
                                     AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
    avCodecContext->channels = av_get_channel_layout_nb_channels(avCodecContext->channel_layout);
    avCodecContext->profile = FF_PROFILE_AAC_LOW;
    avCodecContext->flags != CODEC_FLAG_GLOBAL_HEADER;
    avCodecContext->codec_id = codec->id;
    if (avcodec_open2(avCodecContext, codec, NULL) < 0) {
        LOGI("Couldn't open codec");
        return -2;
    }

    return 0;
}

int AudioEncoder::encode(LiveAudioPacket **pAudioPacket) {
    /** 1、调用注册的回调方法来填充音频的PCM数据 **/
    double presentaitonTimeMills = -1;
    int actualFillSampleSize =
            fillPcmFrameCallback((int16_t *) audio_sample_data[0], audio_nb_samples, audioChannels,
                                 &presentaitonTimeMills, fillPCMFrameContext);
    if (actualFillSampleSize == -1) {
        LOGI("fillPCMFrameCallback failed return actualFillSampleSize is %d \n",
             actualFillSampleSize);
        return -1;
    }
    if (actualFillSampleSize == 0) {
        return -1;
    }

    int actualFIllFrameNum = actualFillSampleSize / audioChannels;
    int audioSampleSize = actualFIllFrameNum * audioChannels * sizeof(short);

    /** 2、将PCM数据按照编码器的格式编码到一个AVPacket中 **/
    AVRational time_base = {1, audioSampleSize};
    int ret;
    AVPacket pkt = {0};
    int got_packet;
    av_init_packet(&pkt);

    pkt.duration = (int) AV_NOPTS_VALUE;
    pkt.pts = pkt.dts = 0;
    avcodec_fill_audio_frame(encode_frame, avCodecContext->channels,
                             avCodecContext->sample_fmt, audio_sample_data[0], audioSampleSize, 0);
    encode_frame->pts = audio_nex_pts;
    audio_nex_pts += encode_frame->nb_samples;

    ret = avcodec_encode_audio2(avCodecContext, &pkt, encode_frame, &got_packet);
    if (ret < 0 || !got_packet) {
        LOGI("Error encoding audio frame: %s\n", av_err2str(ret));
        av_free_packet(&pkt);
        return ret;
    }
    if (got_packet) {
        pkt.pts = av_rescale_q(encode_frame->pts, avCodecContext->time_base,
                               time_base);
        (*pAudioPacket) = new LiveAudioPacket();
        (*pAudioPacket)->data = new byte[pkt.size];
        memcpy((*pAudioPacket), pkt.data, pkt.size);
        (*pAudioPacket)->size = pkt.size;
        (*pAudioPacket)->position = (float) (pkt.pts * av_q2d(time_base) * 1000.0f);

    }
    av_free_packet(&pkt);

    return 0;
}

void AudioEncoder::destroy() {
    LOGI("start destroy!!!");
    if (NULL != audio_sample_data[0]) {
        av_free(audio_sample_data[0]);
    }
    if (NULL != encode_frame) {
        av_free(encode_frame);
    }
    if (NULL != avCodecContext) {
        avcodec_close(avCodecContext);
        av_free(avCodecContext);
    }
    LOGI("end destroy!!!");
}
