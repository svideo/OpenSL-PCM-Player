#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h> 

#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"

static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
	fprintf(stderr, "9992\n");
}

int main(int argc, char* const argv[])
{
	SLresult    result;
	SLObjectItf engineObject = NULL;
	result = slCreateEngine( &engineObject, 0, NULL, 0, NULL, NULL);
	result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);

	SLEngineItf engineEngine;
	result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, (void*)&engineEngine);

	SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                                    SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                    SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN };
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
	SLDataSource audioSrc = {&loc_bufq, &format_pcm};

	SLObjectItf outputMixObject = NULL;

	result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);
	fprintf(stderr, "result %d\n", result);
	result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
	fprintf(stderr, "result %d\n", result);

	SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
	SLDataSink audioSnk = {&loc_outmix, NULL};

	SLObjectItf bqPlayerObject = NULL;
	SLPlayItf bqPlayerPlay;
	SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
	const SLInterfaceID ids1[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
	const SLboolean req1[] = {SL_BOOLEAN_TRUE};
	fprintf(stderr, "result %d\n", result);
	result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk, 1, ids1, req1);
	fprintf(stderr, "result %d\n", result);
	result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    fprintf(stderr, "result %d\n", result);    
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    fprintf(stderr, "result %d\n", result);
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &bqPlayerBufferQueue);
    fprintf(stderr, "result %d\n", result);
	result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
	//result = (*bqPlayerObject)->RegisterCallback(bqPlayerObject, bqPlayerCallback, NULL);
    fprintf(stderr, "result %d\n", result);
	result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
	//bqPlayerBufferQueue->Enqueue(bqPlayerBufferQueue, NULL, );
    fprintf(stderr, "result %d\n", result);
	usleep(60000000);
	return 0;
}
