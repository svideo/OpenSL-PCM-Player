#include <stdio.h>
#include <SLES/OpenSLES.h>
#include <android/log.h>
#include <assert.h>
enum _bool {
	false = 0,
	true
};
typedef enum _bool bool;
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
static const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
static SLObjectItf uriPlayerObject = NULL;
static SLPlayItf uriPlayerPlay;
static SLSeekItf uriPlayerSeek;

void createEngine()
{
	SLresult result;
	result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
	assert(SL_RESULT_SUCCESS == result);
	result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);
	result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
	assert(SL_RESULT_SUCCESS === result);
	const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
	const SLboolean req[1] = {SL_BOOLEAN_FALSE};
	result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
	assert(SL_RESULT_SUCCESS == result);
	result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);
}

bool createUriAudioPlayer(char* uri)
{
	SLresult result;
	SLDataLocator_URI loc_uri = {SL_DATALOCATOR_URI, (SLchar *) uri};
	SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
	SLDataSource audioSrc = {&loc_uri, &format_mime};
	SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
	SLDataSink audioSnk = {&loc_outmix, NULL};
	const SLInterfaceID ids[1] = {SL_IID_SEEK};
	const SLboolean req[1] = {SL_BOOLEAN_TRUE};
	result = (*engineEngine)->CreateAudioPlayer(engineEngine, &uriPlayerObject, &audioSrc, &audioSnk, 1, ids, req);
	assert(SL_RESULT_SUCCESS == result);
	result = (*uriPlayerObject)->Realize(uriPlayerObject, SL_BOOLEAN_FALSE);
	if (SL_RESULT_SUCCESS != result) {
		(*uriPlayerObject)->Destroy(uriPlayerObject);
		uriPlayerObject = NULL;
		return false;
	}
	result = (*uriPlayerObject)->GetInterface(uriPlayerObject, SL_IID_PLAY, &uriPlayerPlay);
	assert(SL_RESULT_SUCCESS == result);
	result = (*uriPlayerObject)->GetInterface(uriPlayerObject, SL_IID_SEEK, &uriPlayerSeek);
	assert(SL_RESULT_SUCCESS == result);
	result = (*uriPlayerSeek)->SetLoop(uriPlayerSeek, SL_BOOLEAN_TRUE, 0, SL_TIME_UNKNOWN);
	assert(SL_RESULT_SUCCESS == result);
	return true;
}

setPlayingUriAudioPlayer(bool played)
{
	SLresult result;
	if (uriPlayerPlay != NULL) {
		result = (*uriPlayerPlay)->SetPlayState(uriPlayerPlay, played ?
				SL_PLAYSTATE_PLAYING : SL_PLAYSTATE_PAUSED);
		assert(SL_RESULT_SUCCESS == result);
	}
}

int main(int argc, char** argv)
{
	createEngine();
	createUriAudioPlayer(argv[1]);
	printf("Playing...");
	setPlayingUriAudioPlayer(true);
	sleep(20);
	printf("Pause...");  
	setPlayingUriAudioPlayer(false);
	sleep(20);

	printf("Playing...");    
	setPlayingUriAudioPlayer(true);

	sleep(1000);
}
