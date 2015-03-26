all:
	ndk-build NDK_PROJECT_PATH=null APP_BUILD_SCRIPT=Android.mk APP_PLATFORM=android-21 NDK_OUT=. NDK_LIBS_OUT=. APP_ABI=armeabi
playmp3:
	ndk-build NDK_PROJECT_PATH=null APP_BUILD_SCRIPT=Android_mp3.mk APP_PLATFORM=android-21 NDK_OUT=. NDK_LIBS_OUT=. APP_ABI=armeabi
mp3topcm:
	gcc mp3topcm.c  -lavformat -lavcodec -lavutil
clean:
	rm -rf armeabi/ local/
