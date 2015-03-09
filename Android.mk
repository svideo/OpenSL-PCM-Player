LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := audio-test
LOCAL_SRC_FILES := main.c
LOCAL_LDLIBS    += -lOpenSLES -llog
include $(BUILD_EXECUTABLE)
