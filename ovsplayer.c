#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "com_example_chengang_myapplication_MainActivity.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

#define  LOG_TAG    "ovsplayer"
#define  LOG(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLEffectSendItf bqPlayerEffectSend;
static SLMuteSoloItf bqPlayerMuteSolo;
static SLVolumeItf bqPlayerVolume;

// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings =
    SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

// file descriptor player interfaces
static SLObjectItf fdPlayerObject = NULL;
static SLPlayItf fdPlayerPlay;
static SLSeekItf fdPlayerSeek;
static SLMuteSoloItf fdPlayerMuteSolo;
static SLVolumeItf fdPlayerVolume;

// pointer and size of the next player buffer to enqueue, and number of remaining buffers
static short *nextBuffer;
static unsigned nextSize;
static int nextCount;

// Ffmpeg vars
AVFrame * aFrame; //audio frame
AVFrame * pFrame; //video frame
AVCodecContext * aCodecCtx;
AVCodecContext * pCodecCtx;
AVCodec * aCodec;
AVCodec * pCodec;
AVFormatContext * input_context;
AVPacket packet;
int videoStream = -1;
int audioStream = -1;

// packet queue vars
typedef struct PacketQueue
{
    AVPacketList * first_pkt, *last_pkt;
    int nb_packets;
    int size;
} PacketQueue;

void packet_queue_init(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
    LOG("put Q");
    AVPacketList *pkt1;
    if(av_dup_packet(pkt) < 0)
        return -1;
    pkt1 = av_malloc(sizeof(AVPacketList));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;



    if (!q->last_pkt)
        q->first_pkt = pkt1;
    else
        q->last_pkt->next = pkt1;
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size;

    return 0;
}

int quit = 0;
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
    AVPacketList *pkt1;
    int ret;
        if(quit)
        {
            ret = -1;
          return -1;
        }

        pkt1 = q->first_pkt;
        if (pkt1)
        {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt)
                q->last_pkt = NULL;
            q->nb_packets--;
            q->size -= pkt1->pkt.size;
            *pkt = pkt1->pkt;
            av_free(pkt1);
            ret = 1;
            return ret;
        } else if (!block) {
            ret = 0;
            return ret;
        }
    return ret;
}

PacketQueue audio_queue;

void decode_audio()
{
//    LOG("[%s]", "audioDecodec enter");
//    int isAudioDecodeSucc = 0;
//    avcodec_decode_audio4(aCodecCtx, aFrame, &isAudioDecodeSucc, &packet);
//    LOG("[%s]", "audioDecodec exit");
//    int data_size = av_samples_get_buffer_size(
//            aFrame->linesize,aCodecCtx->channels,
//            aFrame->nb_samples,aCodecCtx->sample_fmt, 0);
//    LOG("audioDecodec  :%d",data_size);
//    (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, aFrame->data[0], data_size);
}

int decode_frame(uint8_t *stream){
    LOG("[%s]", "audioDecodec enter");
      int len, data_size, got_frame;
        for(;;){
        while(packet.size>0){
//             if(!aFrame)
//              {
//                  if (!(aFrame = av_frame_alloc()))
//                    return AVERROR(ENOMEM);
//               }
//               else
//               {
//                   av_frame_unref(aFrame);
//                }
    LOG("[%s]", "audioDecodec 1");
              len = avcodec_decode_audio4(aCodecCtx, aFrame, &got_frame, &packet);
    LOG("[%s]", "audioDecodec 2");

              if(len < 0)
              {
    LOG("[%s]", "audioDecodec 3");
                  /* if error, skip frame */
                  packet.size = 0;
                  break;
              }
    LOG("[%s]", "audioDecodec 4");
              packet.data += len;
              packet.size -= len;

              if(got_frame <= 0) /* No data yet, get more frames */
                  continue;

    LOG("[%s]", "audioDecodec 5");
     data_size = av_samples_get_buffer_size(
            aFrame->linesize,aCodecCtx->channels,
            aFrame->nb_samples,aCodecCtx->sample_fmt, 0);
    LOG("[%s]", "audioDecodec 6");
                memcpy(stream, aFrame->data[0], data_size);
    LOG("[%s]", "audioDecodec 7");

    LOG("[%s]", "audioDecodec exit");
              return data_size;
        }
        packet_queue_get(&audio_queue,&packet,1);
    }
}

// this callback handler is called every time a buffer finishes playing
uint8_t audio_buf[(192000 * 3) / 2];
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    LOG("[%s]", "callback enter");
      int len=decode_frame(audio_buf);
        LOG("[%s]", "callback 1");
        (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, audio_buf, 2*len);
        LOG("[%s]", "callback 2");
    assert(bq == bqPlayerBufferQueue);
        LOG("[%s]", "callback 3");
    assert(NULL == context);
        LOG("[%s]", "callback 4");

//    // for streaming playback, replace this test by logic to find and fill the next buffer
////    if (--nextCount > 0 && NULL != nextBuffer && 0 != nextSize) {
//        SLresult result;
//        // enqueue another buffer
//        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer, nextSize);
//        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
//        // which for this code example would indicate a programming error
//        assert(SL_RESULT_SUCCESS == result);
//    }
        LOG("[%s]", "callback exit");
}


void createEngine()
{
    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
//    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
            &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
    }
    // ignore unsuccessful result codes for environmental reverb, as it is optional for this example
}

void createBufferQueueAudioPlayer(JNIEnv* env, int rate, int channel,int bitsPerSample)
{
    SLresult result;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    LOG("rate1[%d]", rate);
    LOG("rate2[%d]", SL_SAMPLINGRATE_44_1);
    LOG("channel[%d]", channel);
    LOG("bitsPerSample1[%d]", bitsPerSample);
    LOG("bitsPerSample2[%d]", SL_PCMSAMPLEFORMAT_FIXED_16);

//    SLDataFormat_PCM format_pcm;
//    format_pcm.formatType = SL_DATAFORMAT_PCM;
//    format_pcm.numChannels = channel;
//    format_pcm.samplesPerSec = SL_SAMPLINGRATE_44_1;
//    format_pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
//    format_pcm.containerSize = bitsPerSample;
//    if(channel == 2)
//        format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
//    else
//        format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
//    format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN };

    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

//    SLDataSource audioSrc;
//    audioSrc.pFormat      = (void *)&format_pcm;
//    audioSrc.pLocator     = (void *)&loc_bufq;

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
            3, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
            &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);

    // get the effect send interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
            &bqPlayerEffectSend);
    assert(SL_RESULT_SUCCESS == result);


    // get the volume interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    assert(SL_RESULT_SUCCESS == result);

    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);

}

void AudioWrite(const void*buffer, int size)
{
    (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, buffer, size);
}

long get_time() {
    clock_t t = clock();
    return t;
}

static void fill_bitmap(AndroidBitmapInfo*  info, void *pixels, AVFrame *pFrame)
{
    uint8_t *frameLine;

    int  yy;
    for (yy = 0; yy < info->height; yy++) {
        uint8_t*  line = (uint8_t*)pixels;
        frameLine = (uint8_t *)pFrame->data[0] + (yy * pFrame->linesize[0]);

        int xx;
        for (xx = 0; xx < info->width; xx++) {
            int out_offset = xx * 4;
            int in_offset = xx * 3;

            line[out_offset] = frameLine[in_offset];
            line[out_offset+1] = frameLine[in_offset+1];
            line[out_offset+2] = frameLine[in_offset+2];
            line[out_offset+3] = 254;
        }
        pixels = (char*)pixels + info->stride;
    }
}


JNIEXPORT jstring JNICALL Java_com_example_chengang_myapplication_MainActivity_getStringFromNative
  (JNIEnv * env , jobject obj, jstring url, jstring bitmap)
  {
        char wd[512];
        LOG("[%s]", "hello terminal123");

        const char * filename = (*env)->GetStringUTFChars(env, url, 0);

        // char filename[512];
        // sprintf(filename, "%s", "/sdcard/1.flv");

        // FILE * fd;
        // fd = fopen(filename, "r");
        // fseek(fd, 0L, 2);
        // long filesize = ftell(fd);

        // 打开媒体文件
        av_register_all();
        input_context = NULL;
        int ret = avformat_open_input(&input_context, filename, NULL, NULL);

        // 分别找出一路视频流和一路音频流
        ret = avformat_find_stream_info(input_context, NULL);
        // int streamNum = input_context->nb_streams;
        int i;
        for (i=0; i<input_context->nb_streams; i++) {
            if(input_context->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
                videoStream = i;
            }
            else if(input_context->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
                audioStream = i;
            }
        }

        // 初始化视频解码器
        pCodecCtx = input_context->streams[videoStream]->codec;
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        avcodec_open2(pCodecCtx, pCodec, NULL);
        // LOG("Video size is [%d x %d]", pCodecCtx->width, pCodecCtx->height);

        // 初始化音频解码器
        aCodecCtx = input_context->streams[audioStream]->codec;
        aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
        avcodec_open2(aCodecCtx, aCodec, NULL);

        // 初始化OpenSL
        createEngine();
        createBufferQueueAudioPlayer(env, aCodecCtx->sample_rate, aCodecCtx->channels, SL_PCMSAMPLEFORMAT_FIXED_16);


        // 初始化一个YUV帧内存空间
        pFrame = av_frame_alloc();

        // 初始化一个音频帧内存空间
        aFrame = av_frame_alloc();

        // 初始化一个RGB帧内存空间
        int target_width = 512;
        int target_height = 288;
        AVFrame * pFrameRGB = av_frame_alloc();
        int buffBytes = avpicture_get_size(AV_PIX_FMT_RGB24, target_width, target_height);
        uint8_t * buffer = (uint8_t *) av_malloc( buffBytes * sizeof(uint8_t) );
        avpicture_fill( (AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24, target_width, target_height);

        // 拿到Java中Bitmap的引用
        AndroidBitmapInfo  bitMapInfo;
        AndroidBitmap_getInfo(env, bitmap, &bitMapInfo);

        // Lock Bitmap
        void * pixels_lock;
        AndroidBitmap_lockPixels(env, bitmap, &pixels_lock);

        // 把视频往前seek一点
        av_seek_frame(input_context, videoStream, 5000, 0);

        // 读取一个视频帧
        static struct SwsContext *img_convert_ctx;
        int readVideoFrameNum = 0;
        int isVideoDecodeSucc = 0;

        // LOG("0[%d]\n", CLOCKS_PER_SEC);
        packet_queue_init(&audio_queue);
        int tmp=100;
        while(tmp--){
            if(av_read_frame(input_context,&packet)<0){
                break;
            }

            if(packet.stream_index==audioStream){
                packet_queue_put(&audio_queue,&packet);
            }else {
                av_free_packet(&packet);
            }
        }
        LOG("1[%d]\n", CLOCKS_PER_SEC);
        bqPlayerCallback(NULL,NULL);
//        while (readVideoFrameNum == 0)
//        {
//            av_read_frame(input_context, &packet);
//            if(packet.stream_index==videoStream)
//            {
//                LOG("1[%ld]\n", get_time());
//                avcodec_decode_video2(pCodecCtx, pFrame, &isVideoDecodeSucc, &packet);
//                if (isVideoDecodeSucc)
//                {
//                    img_convert_ctx = sws_getContext(
//                        pCodecCtx->width,   pCodecCtx->height,  pCodecCtx->pix_fmt,
//                        target_width,       target_height,      AV_PIX_FMT_RGB24,
//                        SWS_BICUBIC, NULL, NULL, NULL);
//                    if(img_convert_ctx == NULL) {
//                        LOG("could not initialize conversion context\n");
//                        return;
//                    }
//                    int x = sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
//
//                    fill_bitmap(&bitMapInfo, pixels_lock, pFrameRGB);
//                    readVideoFrameNum++;
//                }
//                else
//                {
//                }
//            }
//            else if(packet.stream_index==audioStream) {
//                decode_audio();
//                readVideoFrameNum++;
//            }
//        }

        // Unlock Bitmap
        AndroidBitmap_unlockPixels(env, bitmap);

        sprintf(wd, "AVCODEC VERSION %u\n, videoStream[%d], audioStream[%d]" , avcodec_version(), videoStream, audioStream);
        av_free(pFrame);
        av_free(aFrame);
        avcodec_close(pCodecCtx);
        avcodec_close(aCodecCtx);
        avformat_close_input(&input_context);

        return (*env)->NewStringUTF(env, wd);
  }