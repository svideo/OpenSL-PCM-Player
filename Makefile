all:
	ndk-build NDK_PROJECT_PATH=null APP_BUILD_SCRIPT=Android.mk APP_PLATFORM=android-21 NDK_OUT=. NDK_LIBS_OUT=. APP_ABI=armeabi
clean:
	rm -rf armeabi/ local/
