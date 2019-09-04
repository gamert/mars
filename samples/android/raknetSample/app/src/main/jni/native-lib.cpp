#include <jni.h>
#include <string>

#include "xlogger/xloggerbase.h"
#include "xlogger/xlogger.h"

#include "xlogger/android_xlog.h"
#include "G:\xProject_dp\SDK\mars\mars\comm/socket/unix_socket.h"

//#include <jni.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <android/log.h>
//#include <sys/socket.h>
//#include <sys/types.h>
//#include <sys/un.h>
//#include <netinet/in.h>
//#include <stddef.h>
//#include <arpa/inet.h>
//#include <unistd.h>
//#include <in.h>

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


int udp_socket()
{
    int udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    if (udpSocket == -1) {
        return -1;
    }

    __android_log_print(ANDROID_LOG_ERROR, "socket", "udpSocket\n");

    sockaddr_in address;

    memset(&address, 0, sizeof(address));
    address.sin_family = PF_INET;

    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(12346);

    int bindState = bind(udpSocket, (const struct sockaddr*)&address, sizeof(address));

    if (bindState == -1) {
        return -1;
    }

    char cc[100];

    struct sockaddr_in recvfromAddress;
    memset(&recvfromAddress, 0, sizeof(recvfromAddress));
    socklen_t l = sizeof(struct sockaddr_in);

    ssize_t recvSize = recvfrom(udpSocket, cc, 8, 0, (struct sockaddr*)&recvfromAddress, &l);

    __android_log_print(ANDROID_LOG_ERROR, "socket", "recvSize\n");
    return 0;
}

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

//    __android_log_print(ANDROID_LOG_INFO, "test", "udp_socket:%d", s);
//    int s = udp_socket();
//    __android_log_print(ANDROID_LOG_INFO, "test", "udp_socket:%d", s);

	_Rak_Settings_t ss = {0,0};
	RkN_Initialize2(&ss,"","");
	if(id == -1)
    {
        LOGI("test", "111111111111");
        id = RkN_CreatePeer();
        __android_log_print(ANDROID_LOG_INFO, "test", "RkN_CreatePeer:%d", id);
        //	LOGI("test", "2222");

//        int ret;
//        unsigned short addressFamily = AF_INET;
//        int type = SOCK_STREAM;
//        int protocol = 0;
//        int rns2Socket = (int) ::socket( addressFamily, type, protocol );
//        __android_log_print(ANDROID_LOG_INFO, "test", "socket:%d", rns2Socket);


        int res = RkN_Startup(id,"0.0.0.0", 0, 1);
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
