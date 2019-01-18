#ifndef _RAKNET_CONNECTOR_H_
#define _RAKNET_CONNECTOR_H_

#include "NetConnector.h"



#define DEFAULT_MAX_PEERS 16
#define DEFAULT_PACKET_SWAP_POOL_SIZE 1024


#pragma pack(8)

#include "RakPeerInterface.h"
#include "PacketLogger.h"

//typedef struct tagPacket
//{
//	// guid
//	uint64_t guid;
//
//	// system address
//	char *source_ip;
//	unsigned short source_port;
//
//	// control command
//	unsigned char control_cmd;
//	unsigned int length;
//
//	// user data
//	unsigned char* data;
//} _Packet_t;

typedef struct tagSystemAddress
{
	char* ip;
	unsigned short port;
} _SystemAddress_t;

//typedef struct tagNode
//{
//	_Packet_t* packet;
//	tagNode* next;
//} _Packet_Node_t;
//
//typedef struct tagPacketList
//{
//	_Packet_t** ppPktList;
//	int amount;
//} _Packet_List_t;

typedef struct tagRakSettings
{
	uint32_t packetSwapPoolSize;
	uint32_t maxNumberOfPeers;
} _Rak_Settings_t;

typedef struct tagRakClientInfo
{
	uint32_t swapIndex;
	RakNet::RakPeerInterface *peer;
#if _RAKNET_SUPPORT_PacketLogger==1
	RakNet::PacketLogger *messageHandler;
#endif
	int guard;			//zzAdd: 判断mem是否被改写.
	RakNet::Packet **receiveSwapPool;
    int     count;      //返回报文个数.
    int     lastIndex;  //当前处理索引.
} _Rak_Client_Info_t;

typedef struct _mcb
{
	void* block;
	size_t size;
	_mcb* next;
} mcb_t;

//这里使用static 是不正确的...
extern _Rak_Settings_t g_Settings;
extern _Rak_Client_Info_t** g_clientList;
extern bool g_initialized;
extern unsigned int g_nativeMemoryAllocated;

inline void BroadcastPacket2AllPeers(RakNet::RakPeerInterface* sender, RakNet::Packet* pkt)
{
	//if (g_initialized)
	{
		for (int i = 0; i < (int)g_Settings.maxNumberOfPeers; ++i)
		{
			_Rak_Client_Info_t* pClient = g_clientList[i];
			if (pClient != (_Rak_Client_Info_t*)0
				&& pClient->peer != 0)
			{
				(pClient->peer)->InsertPacket(pkt);
			}
		}
	}
}

extern "C"
{
    T_DLL void STDCALL RkN_Initialize2(_Rak_Settings_t* pSettings,const char *dataPath,const char *persistentDataPath);
    T_DLL int STDCALL RkN_CreatePeer();
    T_DLL int STDCALL RkN_Startup(int handle, const char* ip, unsigned short port, unsigned int maxConnections);
    T_DLL void* STDCALL RkN_ReceiveCycle2(int handle, int *result);
    T_DLL uint32_t STDCALL RkN_Send(int handle, const char *data, const int length, int priority, int reliability, char orderingChannel, uint64_t guid, bool broadcast, uint32_t forceReceiptNumber = 0);
    T_DLL void STDCALL RkN_Shutdown(int handle, int blockDuration = 0, unsigned char orderingChannel = 0, int disconnectionNotificationPriority = 3);
    T_DLL void STDCALL RkN_CloseConnection(int handle, uint64_t guid, bool sendDisconnectionNotification = true, unsigned char orderingChannel = 0, int disconnectionNotificationPriority = 3);
    T_DLL int STDCALL RkN_Connect(int handle, const char* host, unsigned short port, unsigned int sendConnectionAttemptCount = 4, unsigned int timeBetweenSendConnectionAttemptsMS = 1500, RakNet::TimeMS timeoutTime = 0);
    T_DLL int STDCALL RkN_GetConnectionState(int handle, uint64_t guid);
    T_DLL int STDCALL RkN_GetAveragePing(int handle, uint64_t guid);
    T_DLL void STDCALL RkN_Uninitialize();
	T_DLL void STDCALL RkN_ReleaseClient(int handle);

    T_DLL void STDCALL MemCopy(void *dst,void *src,int size);
    //T_DLL int STDCALL Compress(Bytef *dest, uLongf *destLen,const Bytef *source, uint64_t sourceLen);
    //T_DLL int STDCALL UnCompress(Bytef *dest, uLongf *destLen, const Bytef *source, uint64_t sourceLen);
    T_DLL void STDCALL RkN_xLog(int level, const char* host);
    T_DLL void STDCALL RkN_xLogFlush(int param);
    
    
    
    
    
}
#endif
