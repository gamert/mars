////����㣺�������ʹ�õ�����2����������1�����ݻ�������2�����ܻ�����
////���ܻ�����Ϊָ����С�Ļ��壬���ڴ�SOCKET���ն������ݣ����SOCKET�и������ݣ���ô������ѭ����ȡ��
///*
//	1-- if SOCKET ������ then
//	2-- ��ȡ���и��ࣿ -- ���趨���
//	3-- �������ݻ��壬�Ƿ��б�ǣ�����ص�2
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
