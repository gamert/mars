###
### for libRakNet : 
### zz2015.2
###

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := RakNet_static

LOCAL_MODULE_FILENAME := libRakNet

#LOCAL_SRC_FILES := _FindFirst.cpp

FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES += $(FILE_LIST:$(LOCAL_PATH)/%=%) 


$(info $(LOCAL_PATH))

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)

LOCAL_LDLIBS := -llog \
                -lz

LOCAL_EXPORT_LDLIBS := -llog \
                       -lz
#LOCAL_WHOLE_STATIC_LIBRARIES := RakNet_static


# define the macro to compile through support/zip_support/ioapi.c
#LOCAL_CFLAGS := -Wno-psabi -DUSE_FILE32API
#LOCAL_EXPORT_CFLAGS := -Wno-psabi -DUSE_FILE32API

include $(BUILD_STATIC_LIBRARY)

