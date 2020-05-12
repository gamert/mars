////网络层：和网络层使用的是有2个缓冲区：1。数据缓冲区，2。接受缓冲区
////接受缓冲区为指定大小的缓冲，用于从SOCKET接收定量数据，如果SOCKET有更多数据，那么将进行循环读取，
///*
//	1-- if SOCKET 有数据 then
//	2-- 读取，有更多？ -- 〉设定标记
//	3-- 处理数据缓冲，是否有标记，有则回到2
//*/
//
////Net.cpp

//#include "comm/thread/mutex.h"
//#include "comm/thread/lock.h"
//#include "comm/thread/thread.h"

//Net.h

#include "NetChannel.h"

#include "NetRptApi.h"
#include "Net.h"
#include <assert.h>
#ifdef USE_AntiBot
#include "AntiBot.h"
#endif

//#ifdef _WIN32
//#pragma comment(lib, "ws2_32.lib")
//#endif



//#define NET_LOG
//#ifdef NET_LOG
//#include "SimpleLog.h"
//static myLog sLog("NetLog.txt",true,false);
//static void LogChangeCallback( ConVar *var, char const *pOldString )
//{
//	sLog.SetEnable(var->GetInt() > 0);
//};
//ConVar  log_net( "log_net","0",0,"net log",LogChangeCallback);
//#endif


//#ifdef _DEBUG
//#define NETWORK_HOOK
//#endif

class CCNet :public INet
{
public:
	CCNet()
	{

	}
	virtual void Shutdown()
	{
	};

	virtual INetChannel *Create()
	{
		return new Channel_t();
	};
	virtual void Delete(INetChannel *p)
	{
		assert(p);
		delete p;
	};
};

static CCNet __CCNet;
//NET_INTERFACE INet	*GetINet()
//{
//	return &__CCNet;
//}
