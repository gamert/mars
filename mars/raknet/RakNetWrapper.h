#ifndef RakNetWrapper__H
#define RakNetWrapper__H

#include "src/RakNetConnector.h"

#include <vector>
#include <string>
using namespace std;


typedef unsigned int uint32_t;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char byte;
typedef char sbyte;
typedef uint64_t ulong;
//typedef int *IntPtr;
#define null NULL

namespace RakNet
{
    //这个结构使用协议传 
    //30ms取一次数据,每s 4k; 平均到16FPS
    //[StructLayout(LayoutKind.Explicit, Pack = 8, Size = 24)]
    struct _PacketHead_t
    {
        //[FieldOffset(0)]
        byte head_len;
        //[FieldOffset(1)]
        byte control_cmd;
        //[FieldOffset(2)]
        ushort source_port;
        //[FieldOffset(4)]
        ushort data_len;
        //[FieldOffset(6)]
        ushort _pad;
        //[FieldOffset(8)]
        ulong guid;
        //[FieldOffset(16)]
        ulong source_ip;
	};

    //[StructLayout(LayoutKind.Sequential)]
 //   struct _SystemAddress_t
 //   {
 //      // [MarshalAs(UnmanagedType.LPStr)]
 //       string ip;
 //       ushort port;
	//};

    //[StructLayout(LayoutKind.Sequential)]
    struct _Packet_List_t
    {
        IntPtr ppPktList;
        int amount;
	};

    //[StructLayout(LayoutKind.Sequential)]
    struct _Packet_Node_t
    {
        IntPtr pPacket;
        IntPtr pNext;
	};

    //[StructLayout(LayoutKind.Sequential)]
 //   struct _Rak_Settings_t
 //   {
 //       uint packetSwapPoolSize;
 //       uint maxNumberOfPeers;
	//};

	//enum class PacketPriority
	//{
	//	IMMEDIATE_PRIORITY,
	//	HIGH_PRIORITY,
	//	MEDIUM_PRIORITY,
	//	LOW_PRIORITY,

	//	LOW_PRNUMBER_OF_PRIORITIES
	//};

	//enum class PacketReliability
	//{
	//	UNRELIABLE,
	//	UNRELIABLE_SEQUENCED,
	//	RELIABLE,
	//	RELIABLE_ORDERED,     //3 
	//	RELIABLE_SEQUENCED,
	//	UNRELIABLE_WITH_ACK_RECEIPT,
	//	RELIABLE_WITH_ACK_RECEIPT,
	//	RELIABLE_ORDERED_WITH_ACK_RECEIPT,

	//	NUMBER_OF_RELIABILITIES
	//};

	//enum class ConnectionAttemptResult
	//{
	//	CONNECTION_ATTEMPT_STARTED,
	//	INVALID_PARAMETER,
	//	CANNOT_RESOLVE_DOMAIN_NAME,
	//	ALREADY_CONNECTED_TO_ENDPOINT,
	//	CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS,
	//	SECURITY_INITIALIZATION_FAILED,
	//	UNKNOWN
	//};

	//enum class StartupResult
	//{
	//	RAKNET_STARTED,
	//	RAKNET_ALREADY_STARTED,
	//	INVALID_SOCKET_DESCRIPTORS,
	//	INVALID_MAX_CONNECTIONS,
	//	SOCKET_FAMILY_NOT_SUPPORTED,
	//	SOCKET_PORT_ALREADY_IN_USE,
	//	SOCKET_FAILED_TO_BIND,
	//	SOCKET_FAILED_TEST_SEND,
	//	PORT_CANNOT_BE_ZERO,
	//	FAILED_TO_CREATE_NETWORK_THREAD,
	//	COULD_NOT_GENERATE_GUID,
	//	STARTUP_OTHER_FAILURE
	//};

	//enum class ConnectionState
	//{
	//	IS_PENDING,
	//	IS_CONNECTING,
	//	IS_CONNECTED,
	//	IS_DISCONNECTING,
	//	IS_SILENTLY_DISCONNECTING,
	//	IS_DISCONNECTED,
	//	IS_NOT_CONNECTED
	//};

    //用于新的和服务器交互的协议ID
	enum class MessageIDTypes
	{
		//
		// RESERVED TYPES - DO NOT CHANGE THESE
		// All types from RakPeer
		//

		// For the user to use.  Start your first enumeration at this value.
		ID_USER_PACKET_ENUM = 134,
		ID_USER_PACKET_135 = 135,
		ID_USER_PACKET_136 = 136,
	};

    class RakNetWrapper
    {
	public:
        class Connection
        {
		public:
            ulong guid;
            string ip;
            ushort port;
		};

//#if UNITY_ANDROID
//        const string DLL_NAME = "360Nt";
//#elif UNITY_STANDALONE_OSX
//		const string DLL_NAME ="RakNetBundle";
//#elif UNITY_IPHONE
//		const string DLL_NAME ="__Internal";
//#else
//        const string DLL_NAME = "libRakNet";
//#endif
//        [DllImport(DLL_NAME)]
//        static extern void RkN_Initialize();
//
//        [DllImport(DLL_NAME)]
//        static extern void RkN_Initialize2(IntPtr pSettings, string dataPath, string persistentDataPath);
//
//        [DllImport(DLL_NAME)]
//        static extern int RkN_CreatePeer();
//
//        [DllImport(DLL_NAME)]
//        static extern int RkN_Startup(int handle, string ip, ushort port, uint maxConnections);
//
//        [DllImport(DLL_NAME)]
//        static extern void RkN_ReleaseClient(int handle);
//
//        [DllImport(DLL_NAME)]
//        static extern IntPtr RkN_Receive(int handle);
//
//        [DllImport(DLL_NAME)]
//        static extern uint RkN_Send(int handle, byte[] data, int length, int priority, int reliability, sbyte orderingChannel, ulong guid, bool broadcast, int forceReceiptNumber = 0);
//
//        [DllImport(DLL_NAME)]
//        static extern void RkN_Shutdown(int handle, int blockDuration = 0, byte orderingChannel = 0, int disconnectionNotificationPriority = 3);
//
//        [DllImport(DLL_NAME)]
//        static extern void RkN_CloseConnection(int handle, ulong guid, bool sendDisconnectionNotification = true, byte orderingChannel = 0, int disconnectionNotificationPriority = 3);
//
//        [DllImport(DLL_NAME)]
//        static extern void RkN_SetMaximumIncomingConnections(int handle, ushort numberAllowed);
//
//        [DllImport(DLL_NAME)]
//        static extern int RkN_Connect(int handle, string host, ushort port, uint sendConnectionAttemptCount = 6, uint timeBetweenSendConnectionAttemptsMS = 1000, uint timeoutTime = 0);
//
//        [DllImport(DLL_NAME)]
//        static extern void RkN_Broadcast(int handle, byte[] data, int length, int priority, int reliability, sbyte orderingChannel, IntPtr pGUIDList, int amount, int forceReceiptNumber = 0);
//
//        [DllImport(DLL_NAME)]
//        static extern int RkN_GetAveragePing(int handle, ulong guid);
//
//        [DllImport(DLL_NAME)]
//        static extern IntPtr RkN_ReceiveCycle(int handle, int packetRetrieveLimit = -1);
//
//        [DllImport(DLL_NAME)]
//        unsafe static extern void* RkN_ReceiveCycle2(int handle, out int result);
//        //static extern IntPtr RkN_ReceiveCycle2(int handle, int packetRetrieveLimit = -1);
//
//
//        [DllImport(DLL_NAME)]
//        static extern void RkN_ReleasePacketList(IntPtr packetList);
//
//        [DllImport(DLL_NAME)]
//        static extern void RkN_Uninitialize();
//
//        [DllImport(DLL_NAME)]
//        static extern int RkN_GetConnectionState(int handle, ulong guid);
//
//        [DllImport(DLL_NAME)]
//        static extern void MemCopy(IntPtr dest, IntPtr src, int length);
//
//        [DllImport(DLL_NAME)]
//        unsafe static extern int Compress(byte* dest, ulong* destLen, byte* source, ulong sourceLen);
//
//        [DllImport(DLL_NAME)]
//        unsafe static extern int UnCompress(byte* dest, ulong* destLen, byte* source, ulong sourceLen);
//
//        [DllImport(DLL_NAME)]
//        static extern void RkN_xLog(int level, string content);
//
//        [DllImport(DLL_NAME)]
//        static extern void RkN_xLogFlush(int param);
	};


	typedef void(*OnPacketReceived)(ulong guid, int cmd, byte *data, int length);
	typedef void(*OnConnectionNotify)(RakNetWrapper::Connection *conn, int reason);
}
#endif