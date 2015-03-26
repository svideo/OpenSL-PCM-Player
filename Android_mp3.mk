LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := audio-test-mp3
LOCAL_SRC_FILES := main_mp3.c
LOCAL_LDLIBS    += -lOpenSLES -llog
include $(BUILD_EXECUTABLE)
