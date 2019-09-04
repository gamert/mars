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

#include "RakNetConnectorDef.h"


#endif
