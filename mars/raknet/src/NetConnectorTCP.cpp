
#include <stdlib.h>
#include <cstring>
#include <cstdio>
#include <time.h>


#include "BitStream.h"
#include "Rand.h"
#include "RakNetStatistics.h"
#include "MessageIdentifiers.h"
#include "Kbhit.h"
#include "GetTime.h"
#include "RakAssert.h"
#include "RakSleep.h"
#include "Gets.h"
#include "RakNetTypes.h"
#include "RakNetConnector.h"

#include "comm/xlogger/xlogger.h"
#include "mars/log/appender.h"
#include "mars/comm/verinfo.h"

#include "mars/comm/socket/local_ipstack.h"

////zzAdd: TCP support

#include "comm/autobuffer.h"
//#include "comm/xlogger/xlogger.h"
//#include "comm/bootrun.h"
//#include "comm/thread/mutex.h"
//#include "comm/thread/lock.h"
//#include "comm/thread/thread.h"
//#include "comm/time_utils.h"
//#include "comm/dns/dns.h"
#include "comm/socket/tcpclient.h"


#define MAX_SOCKETS       			1                //ͬʱ���ӵ��׽�����
#define MAXSERVER		  			1
#define SOCK_ERROR_CONNECTFAIL		0x0002
#define SOCK_ERROR_DISCONNECTED		0x0003





extern "C"
{
//
//	
//	//�����ʼ��:
//	//@objId: c#ָ��һ��ΨһID�����ڿ���c��
//	//@pszSvrUrl: tcp://192.168.11.23:8888
//	T_DLL int netty_Initialize(OBJ_T objId, kPlatform platform, uint permission, const char *pszSvrUrl)
//	{
//		return s_NetSystem.Initialize(objId,  platform,  permission, pszSvrUrl);
//	}
//
//	//���ð�ȫ��Ϣ:
//	T_DLL kNetResult netty_setSecurityInfo(OBJ_T objId, kEncryptMethod encyptMethod, kKeyMaking keyMakingMethod, const char *pszDHP)
//	{
//		return kNetResult::Success;
//	}
//
//	//����·����Ϣ
//	T_DLL kNetResult netty_setRouteInfo(OBJ_T objId, byte *buff, int size)
//	{
//		return kNetResult::Success;
//	}
//
//	//����:
//	T_DLL kNetResult netty_connect(OBJ_T objId, uint timeout)
//	{
//		return s_NetSystem.connect(objId, timeout);
//	}
//
//	//����:
//	T_DLL kNetResult netty_reconnect(OBJ_T objId, uint timeout)
//	{
//		return kNetResult::Success;
//	}
//
//	//�����Ͽ�
//	T_DLL kNetResult netty_disconnect(OBJ_T objId)
//	{
//		return kNetResult::Success;
//	}
//
//	//дTCP����
//	T_DLL kNetResult netty_writeData(OBJ_T objId, byte *buff, int size)
//	{
//		return s_NetSystem.writeData(objId, buff, size);
//	}
//
//	//дUDP����
//	T_DLL kNetResult netty_writeUdpData(OBJ_T objId, byte *buff, int size)
//	{
//		return kNetResult::Success;
//	}
//
//	//��·��д
//	T_DLL kNetResult netty_writeData_with_route_info(OBJ_T objId, byte *buff, int size, byte *routeData, int routeDataLen, bool allowLost)
//	{
//		return kNetResult::Success;
//	}
//
//	//��TCP����
//	T_DLL kNetResult netty_readData(OBJ_T objId, byte *buff, int *size)
//	{
//		return kNetResult::Success;
//	}
//
//	//��UDP����
//	T_DLL kNetResult netty_readUdpData(OBJ_T objId, byte *buff, int *size)
//	{
//		return kNetResult::Success;
//	}
//
//	//
//	T_DLL kNetResult netty_getstopreason(OBJ_T objId, int *result, int *reason, int *excode)
//	{
//		return kNetResult::Success;
//	}
//
//	
//	T_DLL kNetResult netty_report_accesstoken(OBJ_T objId, const char *atk, uint expire)
//	{
//		return kNetResult::Success;
//	}
//
//	//
//	T_DLL kNetResult netty_set_clientType(OBJ_T objId, kClientType type)
//	{
//		return kNetResult::Success;
//	}
//
//	//����Э��汾:
//	T_DLL kNetResult netty_set_protocol_version(OBJ_T objId, int headVersion, int bodyVersion)
//	{
//		return kNetResult::Success;
//	}
//
//
//	//
//	T_DLL void addNettyObject(OBJ_T objectId, const char *objName)
//	{
//		printf("addNettyObject:objectId=%d,objName=%s\n", objectId, objName);
//	};
//
//	T_DLL void removeNettyObject(OBJ_T objectId)
//	{
//		printf("removeNettyObject:objectId=%d\n", objectId);
//	};
//
//
//	T_DLL void setNettySendMessageCallback( NettySendMessageDelegate callback)
//	{
//		printf("setNettySendMessageCallback:callback=%x\n", (uint32_t)callback);
//		m_NettySendMessageDelegate = callback;
//	};
//	T_DLL void setNettySendStructCallback( NettySendStructDelegate callback)
//	{
//		printf("setNettySendStructCallback:callback=%x\n", (uint32_t)callback);
//		m_NettySendStructDelegate = callback;
//	};
//
//	T_DLL void setNettySendResultCallback( NettySendResultDelegate callback)
//	{
//		printf("setNettySendResultCallback:callback=%x\n", (uint32_t)callback);
//	};
//
//	T_DLL void setNettySendBufferCallback( NettySendBufferDelegate callback)
//	{
//		printf("setNettySendBufferCallback:callback=%x\n", (uint32_t)callback);
//	};
//
//	T_DLL void setNettySendResultBufferCallback( NettySendResultBufferDelegate callback)
//	{
//		printf("setNettySendResultBufferCallback:callback=%x\n", (uint32_t)callback);
//	};
//
//	T_DLL void Netty_quit()
//	{
//		printf("Netty_quit:\n");
//	};
}

