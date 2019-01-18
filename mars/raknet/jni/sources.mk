

TEMP_LOCAL_PATH :=$(call my-dir)/..

include $(TEMP_LOCAL_PATH)/../mk_template/flags.mk
include $(TEMP_LOCAL_PATH)/../mk_template/util.mk

LOCAL_CFLAGS += -Wno-error=maybe-uninitialized -Wno-error -Wno-error=sign-compare -Wno-unused # x86 compile: ini.h
#LOCAL_CFLAGS := $(LOCAL_CFLAGS:-Wconversion=)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/src/*.cpp)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

$(info 'garry=$(LOCAL_SRC_FILES)')

LOCAL_C_INCLUDES += $(TEMP_LOCAL_PATH)/ $(TEMP_LOCAL_PATH)/src $(TEMP_LOCAL_PATH)/../../
LOCAL_LDFLAGS += -Wl,--version-script=$(TEMP_LOCAL_PATH)/jni/export.exp
