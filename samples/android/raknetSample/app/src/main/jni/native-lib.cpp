#include <jni.h>
#include <string>

#include "xlogger/xloggerbase.h"
#include "xlogger/xlogger.h"

#include "xlogger/android_xlog.h"

namespace RakNet
{
    typedef int64_t TimeMS;
}
struct _Rak_Settings_t
{
    uint32_t packetSwapPoolSize;
    uint32_t maxNumberOfPeers;
};

#define T_DLL
#define STDCALL
#include "G:\xProject_dp\SDK\mars\mars\raknet\src\RakNetConnectorDef.h"

int id = -1;
extern "C"
jstring
Java_com_tencent_mars_xlogsample_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {

	xinfo_function();


	xdebug2(TSF "test xlog, have level filter. line:%0, func:%1", __LINE__, __FUNCTION__);

	xdebug2("test xlog, have level filter. line:%d, func:%s", __LINE__, __FUNCTION__);

	xassert2(1<0, "assert false info");

	xassert2(false);
	xassert2(true);

	xverbose2(TSF "test xlog, have level filter");

//	LOGD("testxlog", "-------user define:%d--------", __LINE__);
	
//	__android_log_print(ANDROID_LOG_INFO, "test", "123");
//
//	__android_log_print(ANDROID_LOG_INFO, "test", "123:%d", 4);
//	__android_log_write(ANDROID_LOG_INFO, "test", "123");
//
//	__android_log_assert(1>0, "test", "%d", 3455);
//
//	__android_log_assert(1<0, "test", "%d", 3455);
//
//	__android_log_assert(1<0, "test", "3455dfdddddddddd");
	
//	LOGI("test", "0000");



	_Rak_Settings_t ss = {0,0};
	RkN_Initialize2(&ss,"","");
	if(id == -1)
    {
        LOGI("test", "111111111111");
        id = RkN_CreatePeer();
        __android_log_print(ANDROID_LOG_INFO, "test", "RkN_CreatePeer:%d", id);
        //	LOGI("test", "2222");
        int res = RkN_Startup(id,NULL,0,4);
        __android_log_print(ANDROID_LOG_INFO, "test", "RkN_Startup,id=%d, res=%d", id,res);
    }


    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


extern "C"
void
Java_com_tencent_mars_xlogsample_MainActivity_connect(
        JNIEnv* env,
        jobject /* this */)
{
    int res = RkN_Connect(id,"192.168.85.21", 9822);
    __android_log_print(ANDROID_LOG_INFO, "test", "RkN_Connect,id=%d, res=%d", id,res);
}
