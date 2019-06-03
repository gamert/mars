/*
	
*/
#include "KcpClientImp.h"
#include "kcp_net.h"

//#include "comm/xlogger/xlogger.h"
#include "mars/log/appender.h"
#include "mars/comm/verinfo.h"
#include "mars/comm/socket/local_ipstack.h"


//BOOST_NO_EXCEPTIONS
//namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
//{
//#ifdef _WIN32	
//	//void throw_exception(std::exception const & e) // user defined
//	//{
//	//	printf("throw_exception: ");
//	//}
//#endif
//}

#ifdef __ANDROID__
#include <jni.h>
#include <string>
#endif


#ifdef _WIN32
#include "mars/comm/windows/zlib/zlib.h"

#include <Wininet.h>  
#include <Sensapi.h>  
#include <iostream>
#pragma comment(lib, "Sensapi.lib") 
//#pragma comment(lib,"boost.lib")
//#pragma comment(lib,"comm.lib")
//#pragma comment(lib,"log.lib")


void Fun_IsNetworkAlive()
{
	DWORD   flags;//上网方式  
	BOOL   m_bOnline = TRUE;//是否在线    
	m_bOnline = IsNetworkAlive(&flags);
	if (m_bOnline)//在线    
	{
		if ((flags & NETWORK_ALIVE_LAN) == NETWORK_ALIVE_LAN)
		{
			std::cout << "在线：NETWORK_ALIVE_LAN\n";
		}

		if ((flags & NETWORK_ALIVE_WAN) == NETWORK_ALIVE_WAN)
		{
			std::cout << "在线：NETWORK_ALIVE_WAN\n";
		}

		if ((flags & NETWORK_ALIVE_AOL) == NETWORK_ALIVE_AOL)
		{
			std::cout << "在线：NETWORK_ALIVE_AOL\n";
		}
	}
	else
	{
		std::cout << "不在线\n";
	}
}

#else
	#include <zlib.h>
#endif


std::string g_dataPath;
std::string g_persistentDataPath;

int g_log = 0;


int CCLOG(int level, const char * _Format, ...)
{
	const int bufsize = 1024 * 2;
	char buffer[bufsize];

	va_list argptr;
	int cnt;
	va_start(argptr, _Format);
	cnt = vsnprintf(buffer, bufsize, _Format, argptr);
	va_end(argptr);
	if (level == 0)
	{
#ifdef _WIN32
		::OutputDebugStringA(buffer);
#else
		LOGD("%s", buffer);
#endif
	}
	else
	{
		//写日志文件..
		//LOGE("%s",buffer);
		//appender_flush();
		xinfo2(TSF"%_", buffer);
	}
	return(cnt);
}


#if RAKNET_SUPPORT_IPV6 == 1
TLocalIPStack GetLocalIPStack(bool bForce);
TLocalIPStack g_TLocalIPStack = GetLocalIPStack(true);
TLocalIPStack GetLocalIPStack(bool bForce)
{
	if (!bForce)return g_TLocalIPStack;
	std::string _log;
	g_TLocalIPStack = local_ipstack_detect_log(_log);
	xinfo2(TSF"%_", _log);
	return g_TLocalIPStack;
}
void FlushLocalIPStack()
{
	GetLocalIPStack(true);
}

#endif




bool g_initialized = false;

extern "C"
{
	//
	T_DLL void STDCALL _std_initialize(const char *dataPath, const char *persistentDataPath)
	{
		if (!g_initialized)
		{

#ifdef _WIN32
			WSADATA wsaData;
			WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

			if (dataPath)
				g_dataPath = dataPath;
			if (persistentDataPath)
				g_persistentDataPath = persistentDataPath;

			std::string dir = g_persistentDataPath + "/xLog";
			CCLOG(0, "Net_Initialize2(%s) MARS_BUILD_TIME: " MARS_BUILD_TIME, dir.c_str());
			appender_open(kAppednerAsync, dir.c_str(), "zz", NULL);
			xlogger_SetLevel(kLevelAll);

			xinfo2(TSF"start log:%_", 0);

			xinfo2(TSF"[w]Net_Initialize2:%_;%_\n", g_dataPath.c_str(), g_persistentDataPath.c_str());
			g_initialized = true;

			appender_flush();
		}
	}

	T_DLL void STDCALL _std_uninitialize()
	{
		if (g_initialized)
		{
			g_initialized = false;
		}
		appender_flush();
	}

	T_DLL IntPtr STDCALL _std_create_session(const char *type)
	{
		if (stricmp(type, "kcp") == 0)
		{
			return new CKcpClientImp();
		}
		else if (stricmp(type, "tcp") == 0)
		{
		}
		return NULL;
	}

	T_DLL void STDCALL _std_release_session(IntPtr handle)
	{
		CKcpClientImp *p = (CKcpClientImp *)handle;
		delete p;
	};
	

	//
	T_DLL int STDCALL _std_connect(IntPtr handle, const char* ip, unsigned short port, unsigned uuid)
	{
		// auto ping/pong
		//peer->SetOccasionalPing(true);
		int ipv6 = 0;
#if RAKNET_SUPPORT_IPV6 == 1
		if (ELocalIPStack_IPv6 == g_TLocalIPStack)
			ipv6 = true;
		//            char address[INET6_ADDRSTRLEN];
		//            ipv6 = IOS_GetIp(address, NULL);
#endif
		if (ipv6)
			ip = "";

		CKcpClientImp *p = (CKcpClientImp *)handle;
		bool bres = p->connect(ip, port, uuid);
		return bres == true ? 0 : -1;
	}

	T_DLL void STDCALL _std_close_connect(IntPtr handle)
	{
		CKcpClientImp *p = (CKcpClientImp *)handle;
		p->CloseConnect();
	}


	//返回一个整数，高16位表示，剩余报文，低16位表示已经处理的报文
	T_DLL void* STDCALL _std_receive_cycle(IntPtr handle, int *result)
	{

#if EDITOR_CONSOLE==1
		consoleServer.Update();
#endif

		//if (_Rak_Client_Info_t *pClient = handle2Client(handle))
		//{
		//	if (RakPeerInterface* peer = pClient->peer)
		//	{
		//		int limit = g_Settings.packetSwapPoolSize;

		//		//检查
		//		//如果上一次循环没有处理完？
		//		if (pClient->count > pClient->lastIndex)
		//		{
		//			//Net_Dump(pClient);                    
		//			if (pClient->guard == nGuad && pClient->lastIndex < limit && pClient->count <= limit)
		//			{
		//				//返回0，则c#退出循环
		//				*result = _std_receive_cycle(pClient, g_rbuf, RBUF_SIZE);	//+104 （4	DW-Normal _std_receive_cycle (RakNetConnector.cpp:590）
		//				return g_rbuf;
		//			}
		//			else
		//			{
		//				xerror2(TSF"_std_receive_cycle: check guard(%_/%_) lastIndex(%_/%_) count(%_/%_) pClient(%_) peer(%_)\n", pClient->guard, nGuad, pClient->lastIndex, limit, pClient->count, limit, pClient, peer);
		//			}
		//		}
		//		//一轮新的开始...
		//		//pClient->count = 0;
		//		pClient->lastIndex = 0;
		//		pClient->count = peer->Receive(pClient->receiveSwapPool, limit);
		//		if (pClient->count>0)
		//		{
		//			if (g_max_count<pClient->count)
		//			{
		//				xinfo2(TSF"_std_receive_cycle: g_max_count(%_)<pClient->count(%_) \n", g_max_count, pClient->count);
		//				g_max_count = pClient->count;
		//			}
		//			*result = _std_receive_cycle(pClient, g_rbuf, RBUF_SIZE);	//
		//		}
		//		else
		//		{
		//			*result = 0;
		//		}
		//		return g_rbuf;
		//	}
		//	*result = -1;
		//	return 0;
		//}
		*result = -2;
		return 0;
	}

	T_DLL int STDCALL _std_send(IntPtr handle, const char *data, const int length, int sendType)
	{
		CKcpClientImp *p = (CKcpClientImp *)handle;
		return p->Send(data, length, sendType);
	}

	T_DLL int STDCALL _std_get_connection_state(IntPtr handle)
	{
		CKcpClientImp *p = (CKcpClientImp *)handle;
		return p->GetConnectionState();
	}

	T_DLL int STDCALL _std_get_average_ping(IntPtr handle)
	{
		CKcpClientImp *p = (CKcpClientImp *)handle;
		return p->GetAveragePing();
	}

	T_DLL int STDCALL _std_send_ping(IntPtr handle, int index, bool bTcp = true)
	{
		//printf("send_ping \n");
		//在 windows 下 和 linux 下 取到的 时间精度 很不一样啊, windows下 居然 位数都不对, 比linux 下 少两位数
		KTime t2 = GetKTime();
		char buf[64] = { 0 };
		buf[0] = TF_TYPE_PING;
		memcpy(buf + 1, &index, sizeof(index));
		memcpy(buf + 5, &t2, sizeof(t2));

		_std_send(handle, buf, 1 + 4 + 8, bTcp ? 0 : 1);
		return 0;
	}


	T_DLL uint64_t STDCALL _std_get_timeUs()
	{
		KTime t1 = GetKTime();
		uint64_t t2 = *(uint64_t*)&t1;
		return t2;
	}

	//新加的。。。
	T_DLL void STDCALL MemCopy(void *dst, void *src, int size)
	{
		memcpy(dst, src, size);
	}

	T_DLL int STDCALL Compress(Bytef *dest, uLongf *destLen, const Bytef *source, uint64_t sourceLen)
	{
		//uLongf destLen = *destLen0;
		int res = compress2(dest, destLen, source, sourceLen, Z_BEST_SPEED);
		//*destLen0 = destLen;
		return res;
	}

	T_DLL int STDCALL UnCompress(Bytef *dest, uLongf *destLen, const Bytef *source, uint64_t sourceLen)
	{
		//uLongf destLen = *destLen0;
		int res = uncompress(dest, destLen, source, sourceLen);
		//*destLen0 = destLen;
		return res;
	}

	//typedef enum {
	//	kLevelAll = 0,
	//	kLevelVerbose = 0,
	//	kLevelDebug,    // Detailed information on the flow through the system.
	//	kLevelInfo,     // Interesting runtime events (startup/shutdown), should be conservative and keep to a minimum.
	//	kLevelWarn,     // Other runtime situations that are undesirable or unexpected, but not necessarily "wrong".
	//	kLevelError,    // Other runtime errors or unexpected conditions.
	//	kLevelFatal,    // Severe errors that cause premature termination.
	//	kLevelNone,     // Special level used to disable all log messages.
	//} TLogLevel;
	//0722add: log
	T_DLL void STDCALL _std_xlog(int level, const char* host)
	{
		xlog2((TLogLevel)level, TSF"%_\n", host);
	}
	//1012add: flush log
	T_DLL void STDCALL _std_xlog_flush(int param)
	{
		appender_flush_sync();
	}

	//2019/1/18: 增加网络事件通知，用于更新IPStack
	T_DLL int STDCALL _std_event(int param)
	{
#if RAKNET_SUPPORT_IPV6 == 1
		FlushLocalIPStack();
		//必须异步...
		//std::string GetDetailNetInfo()
		return g_TLocalIPStack;
#endif
		return 0;
	}

	//T_DLL int STDCALL Net_GetIpStack(Bytef *dest, uLongf *destLen, const Bytef *source, uint64_t sourceLen)
	//{
	//}
}
#ifdef __ANDROID__
extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_test360pay_MainActivity_testNewString(
	JNIEnv* env,
	jobject /* this */) {

	int use = Net_GetNativeMemoryUsage();
	const char *hello = "Hello from raknet";
	return env->NewStringUTF(hello);
}
#endif



