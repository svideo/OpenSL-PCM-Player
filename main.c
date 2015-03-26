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
	fprintf(stderr, "9992");
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
	result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

	SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
	SLDataSink audioSnk = {&loc_outmix, NULL};

	SLObjectItf bqPlayerObject = NULL;
	SLPlayItf bqPlayerPlay;
	SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
	const SLInterfaceID ids1[] = {SL_IID_ANDROIDBUFFERQUEUESOURCE};
	const SLboolean req1[] = {SL_BOOLEAN_TRUE};
	fprintf(stderr, "1\n");
	result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk, 1, ids1, req1);
	fprintf(stderr, "2\n");
	result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
	fprintf(stderr, "3\n");
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
	fprintf(stderr, "4\n");
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
	fprintf(stderr, "5\n");
	result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
	//result = (*bqPlayerObject)->RegisterCallback(bqPlayerObject, bqPlayerCallback, NULL);
	fprintf(stderr, "6\n");
	result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
	//bqPlayerBufferQueue->Enqueue(bqPlayerBufferQueue, NULL, );

	usleep(60000000);
	return 0;
}
