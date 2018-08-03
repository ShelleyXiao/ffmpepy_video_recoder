//
// Created by ShaudXiao on 2018/8/3.
//

#include "aac_accompany_deocder.h"

int AACAccompanyDecoder::readSamples(short *samples, int size) {
    if (seek_req) {
        this->audioBufferCursor = audioBufferSize;
    }

    int sampleSize = size;
    while (size > 0) {
        if (audioBufferCursor < audioBufferSize) {
            //copy
            int audioBufferDataSize = audioBufferSize - audioBufferCursor;
            int cpySize = MIN(audioBufferDataSize, size);
            copy(samples + (sampleSize - size),
                 audioBuffer + audioBufferCursor, cpySize * 2);
            size -= cpySize;
            audioBufferCursor += cpySize;
        } else {
            //read
            if (readFrame() < 0) {
                break;
            }
        }
    }
    int fillSize = sampleSize - size;
    if (fillSize == 0) {
        return -1;
    }

    return fillSize;
}

int AACAccompanyDecoder::readFrame() {
    if (this->seek_req) {
        this->seek_frame();
    }

    int ret = 1;
    av_init_packet(&packet);
    int gotFrame = 0;
    int readFrameCode = -1;
    while (true) {
        readFrameCode = av_read_frame(avFormatContext, &packet);
        if (readFrameCode >= 0) {
            if (packet.stream_index == stream_index) {
                int len = avcodec_decode_audio4(avCodecContext, pAudioFrame, &gotFrame, &packet);
                if (len < 0) {
                    LOGI("decode audio error, skip packet");
                }
                if (gotFrame) {
                    int numChannels = OUT_PUT_CHANNELS;
                    int numFrames = 0;
                    void *audioData;
                    //重采样
                    if (swrContext) {
                        const int ratio = 2;
                        const int bufSize = av_samples_get_buffer_size(
                                NULL, numChannels, pAudioFrame->nb_samples * ratio,
                                AV_SAMPLE_FMT_S16, 1
                        );

                        if (!swrBuffer || swrBufferSize < bufSize) {
                            swrBufferSize = bufSize;
                            swrBuffer = realloc(swrBuffer, swrBufferSize);
                        }

                        byte *outBuf[2] = {(byte *) swrBuffer, NULL};
                        numFrames = swr_convert(swrContext, outBuf,
                                                pAudioFrame->nb_samples * ratio,
                                                (const uint8_t **) pAudioFrame->data,
                                                pAudioFrame->nb_samples);
                        if (numFrames < 0) {
                            LOGI("fail resample audio");
                            ret = -1;
                            break;
                        }
                        audioData = swrBuffer;
                    } else {
                        if (avCodecContext->sample_fmt != AV_SAMPLE_FMT_S16) {
                            LOGI("bucheck, audio format is invalid");
                            ret = -1;
                            break;
                        }
                        audioData = pAudioFrame->data[0];
                        numFrames = pAudioFrame->nb_samples;
                    }
                    if (isNeedFirstFrameCorrectFlag && position >= 0) {
                        float expectedPosition = position + duration;
                        float actualPosition =
                                av_frame_get_best_effort_timestamp(pAudioFrame) * timeBase;
                        firstFrameCorrectInSecs = actualPosition - expectedPosition;
                        isNeedFirstFrameCorrectFlag = false;
                    }
                    duration = av_frame_get_pkt_duration(pAudioFrame) * timeBase -
                               firstFrameCorrectInSecs;
                    if (!seek_success_read_fream_success) {
                        LOGI("position is %.6f", position);
                        actualSeekPosition = position;
                        seek_success_read_fream_success = true;
                    }
                    audioBufferSize = numFrames * numChannels;
                    audioBuffer = (short *) audioData;
                    audioBufferCursor = 0;
                    break;


                }

            }
        } else {
            ret = -1;
            break;
        }
    }
    av_free_packet(&packet);
    return ret;
}

bool AACAccompanyDecoder::audioCodecIsSupported() {
    if (avCodecContext->sample_fmt == AV_SAMPLE_FMT_S16) {
        return true;
    }
    return false;
}

AACAccompanyDecoder::AACAccompanyDecoder() {
    avFormatContext = NULL;
    avCodecContext = NULL;
    swrContext = NULL;
    swrBuffer = NULL;
    accompanyFilePath = NULL;
}

AACAccompanyDecoder::~AACAccompanyDecoder() {
    if (accompanyFilePath != NULL) {
        delete accompanyFilePath;
        accompanyFilePath = NULL;
    }
}

int AACAccompanyDecoder::getMusicMeta(const char *fileString, int *metaData) {
    init(fileString);
    int sampleRate = avCodecContext->sample_rate;
    int bitRate = avCodecContext->bit_rate;
    destroy();
    metaData[0] = sampleRate;
    metaData[1] = bitRate;
    return 0;

}

void AACAccompanyDecoder::init(const char *fileString, int packetBufferSizeParam) {
    init(fileString);
    packetBufferSize = packetBufferSizeParam;
}

int AACAccompanyDecoder::init(const char *audioFile) {
    LOGI("enter AACAccompanyDecoder::init");
    audioBuffer = NULL;
    position = -1.0f;
    audioBufferCursor = 0;
    audioBufferSize = 0;
    swrContext = NULL;
    swrBuffer = NULL;
    swrBufferSize = 0;

    seek_success_read_fream_success = true;
    isNeedFirstFrameCorrectFlag = true;
    firstFrameCorrectInSecs = 0.0f;

    avcodec_register_all();
    av_register_all();

    avFormatContext = avformat_alloc_context();
    // 打开输入文件
    LOGI("open accompany file %s....", audioFile);
    if (NULL == accompanyFilePath) {
        int length = strlen(audioFile);
        accompanyFilePath = new char[length + 1];
        memset(accompanyFilePath, 0, length + 1);
        memcpy(accompanyFilePath, audioFile, length + 1);
    }

    int result =
            avformat_open_input(&avFormatContext, audioFile, NULL, NULL);
    if (result != 0) {
        LOGI("can't open file %s result is %d", audioFile, result);
        avFormatContext = NULL;
        return -1;
    } else {
        LOGI("open file %s success and result is %d", audioFile, result);
    }

    avFormatContext->max_analyze_duration = 50000;
    result = avformat_find_stream_info(avFormatContext, NULL);
    if (result < 0) {
        LOGI("fail avformat_find_stream_info result is %d", result);
        return -1;
    } else {
        LOGI("sucess avformat_find_stream_info result is %d", result);
    }

    stream_index = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (stream_index == -1) {
        LOGI("NO AUDIO STREAM");
        return -1;
    }

    AVStream *audioStream = avFormatContext->streams[stream_index];
    if (audioStream->time_base.den && audioStream->time_base.num) {
        timeBase = av_q2d(audioStream->time_base);
    } else if (audioStream->codec->time_base.den && audioStream->codec->time_base.num) {
        timeBase = av_q2d(audioStream->codec->time_base);
    }

    avCodecContext = audioStream->codec;
    // 根据解码器上下文找到解码器
    LOGI("avCodecContext->codec_id is %d AV_CODEC_ID_AAC is %d",
         avCodecContext->codec_id, AV_CODEC_ID_AAC);
    AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);
    if (avCodec == NULL) {
        LOGI("Unsupported codec ");
        return -1;
    }

    result = avcodec_open2(avCodecContext, avCodec, NULL);
    if (result < 0) {
        LOGI("fail avformat_find_stream_info result is %d", result);
        return -1;
    } else {
        LOGI("sucess avformat_find_stream_info result is %d", result);
    }

    //4、判断是否需要resampler
    if (!audioCodecIsSupported()) {
        LOGI("because of audio Codec Is Not Supported so we will init swresampler...");
        /**
         * 初始化resampler
         * @param s               Swr context, can be NULL
         * @param out_ch_layout   output channel layout (AV_CH_LAYOUT_*)
         * @param out_sample_fmt  output sample format (AV_SAMPLE_FMT_*).
         * @param out_sample_rate output sample rate (frequency in Hz)
         * @param in_ch_layout    input channel layout (AV_CH_LAYOUT_*)
         * @param in_sample_fmt   input sample format (AV_SAMPLE_FMT_*).
         * @param in_sample_rate  input sample rate (frequency in Hz)
         * @param log_offset      logging level offset
         * @param log_ctx         parent logging context, can be NULL
         */
        swrContext = swr_alloc_set_opts(NULL, av_get_default_channel_layout(OUT_PUT_CHANNELS),
                                        AV_SAMPLE_FMT_S16, avCodecContext->sample_rate,
                                        av_get_default_channel_layout(avCodecContext->channels),
                                        avCodecContext->sample_fmt, avCodecContext->sample_rate, 0,
                                        NULL);
        if (!swrContext || swr_init(swrContext)) {
            if (swrContext)
                swr_free(&swrContext);
            avcodec_close(avCodecContext);
            LOGI("init resampler failed...");
            return -1;
        }
    }
    LOGI("channels is %d sampleRate is %d",
         avCodecContext->channels, avCodecContext->sample_rate);
    pAudioFrame = avcodec_alloc_frame();

    return 0;
}

void AACAccompanyDecoder::setPacketBufferSize(int packetBufferSizeParam) {
    packetBufferSize = packetBufferSizeParam;
}

LiveAudioPacket *AACAccompanyDecoder::decodePacket() {
    short *samples = new short[packetBufferSize];
    int stereoSampleSize = readSamples(samples, packetBufferSize);
    LiveAudioPacket *packet = new LiveAudioPacket();
    if (stereoSampleSize > 0) {
        packet->buffer = samples;
        packet->size = stereoSampleSize;
        packet->position = position;
    } else {
        packet->size = -1;
    }

    return packet;
}

void AACAccompanyDecoder::destroy() {
    if (NULL != swrBuffer) {
        free(swrBuffer);
        swrBuffer = NULL;
        swrBufferSize = 0;
    }
    if (NULL != swrContext) {
        swr_free(&swrContext);
        swrContext = NULL;
    }
    if (NULL != pAudioFrame) {
        av_free (pAudioFrame);
        pAudioFrame = NULL;
    }
    if (NULL != avCodecContext) {
        avcodec_close(avCodecContext);
        avCodecContext = NULL;
    }
    if (NULL != avFormatContext) {
        LOGI("leave LiveReceiver::destory");
        avformat_close_input(&avFormatContext);
        avFormatContext = NULL;
    }
}

void AACAccompanyDecoder::seek_frame() {
    LOGI("\n seek frame firstFrameCorrectionInSecs is %.6f, seek_seconds=%f, position=%f \n",
         firstFrameCorrectInSecs,
         seek_seconds, position);
    float targetPosition = seek_seconds;
    float currentPosition = position;
    float frameDuration = duration;
    if (targetPosition < currentPosition) {
        this->destroy();
        this->init(accompanyFilePath);
        currentPosition = 0.0;
    }
    int readFrameCode = -1;
    while (true) {
        av_init_packet(&packet);
        readFrameCode = av_read_frame(avFormatContext, &packet);
        if(readFrameCode >= 0) {
            currentPosition += frameDuration;
            if(currentPosition >= targetPosition) {
                break;
            }
        }
        av_free_packet(&packet);
    }
    seek_resp = true;
    seek_req = false;
    seek_success_read_fream_success = false;
}
