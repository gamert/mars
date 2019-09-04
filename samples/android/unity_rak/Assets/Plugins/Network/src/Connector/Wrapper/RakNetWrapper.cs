using System;
using System.Runtime.InteropServices;

namespace RakNet
{
    //这个结构使用协议传
    //30ms取一次数据,每s 4k; 平均到16FPS
    [StructLayout(LayoutKind.Explicit, Pack = 8, Size = 24)]
    public struct _PacketHead_t
    {
        [FieldOffset(0)]
        public byte head_len;
        [FieldOffset(1)]
        public byte control_cmd;
        [FieldOffset(2)]
        public ushort source_port;
        [FieldOffset(4)]
        public ushort data_len;
        [FieldOffset(6)]
        public ushort _pad;
        [FieldOffset(8)]
        public ulong guid;
        [FieldOffset(16)]
        public ulong source_ip;

        //[FieldOffset(24)]
        //public int orderingIndex;
        //[FieldOffset(28)]
        //public int sequencingIndex;
        //[FieldOffset(32)]
        //public int reliability;
        //[FieldOffset(36)]
        //public int local_id;	//本地自增
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct _SystemAddress_t
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string ip;
        public ushort port;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct _Packet_List_t
    {
        public IntPtr ppPktList;
        public int amount;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct _Packet_Node_t
    {
        public IntPtr pPacket;
        public IntPtr pNext;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct _Rak_Settings_t
    {
        public uint packetSwapPoolSize;
        public uint maxNumberOfPeers;
    }

    public delegate void OnPacketReceived(ulong guid, int cmd, byte[] data, int length);
    public delegate void OnConnectionNotify(RakNetWrapper.Connection conn, int reason = 0);

    public enum PacketPriority
    {
        IMMEDIATE_PRIORITY,
        HIGH_PRIORITY,
        MEDIUM_PRIORITY,
        LOW_PRIORITY,

        LOW_PRNUMBER_OF_PRIORITIES
    }

    public enum PacketReliability
    {
        UNRELIABLE,
        UNRELIABLE_SEQUENCED,
        RELIABLE,
        RELIABLE_ORDERED,     //3 
        RELIABLE_SEQUENCED,
        UNRELIABLE_WITH_ACK_RECEIPT,
        RELIABLE_WITH_ACK_RECEIPT,
        RELIABLE_ORDERED_WITH_ACK_RECEIPT,

        NUMBER_OF_RELIABILITIES
    }

    public enum ConnectionAttemptResult
    {
        CONNECTION_ATTEMPT_STARTED,
        INVALID_PARAMETER,
        CANNOT_RESOLVE_DOMAIN_NAME,
        ALREADY_CONNECTED_TO_ENDPOINT,
        CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS,
        SECURITY_INITIALIZATION_FAILED,
        UNKNOWN
    }

    public enum StartupResult
    {
        RAKNET_STARTED,
        RAKNET_ALREADY_STARTED,
        INVALID_SOCKET_DESCRIPTORS,
        INVALID_MAX_CONNECTIONS,
        SOCKET_FAMILY_NOT_SUPPORTED,
        SOCKET_PORT_ALREADY_IN_USE,
        SOCKET_FAILED_TO_BIND,
        SOCKET_FAILED_TEST_SEND,
        PORT_CANNOT_BE_ZERO,
        FAILED_TO_CREATE_NETWORK_THREAD,
        COULD_NOT_GENERATE_GUID,
        STARTUP_OTHER_FAILURE
    }

    public enum ConnectionState
    {
        IS_PENDING,
        IS_CONNECTING,
        IS_CONNECTED,
        IS_DISCONNECTING,
        IS_SILENTLY_DISCONNECTING,
        IS_DISCONNECTED,
        IS_NOT_CONNECTED
    }

    //用于新的和服务器交互的协议ID
    public enum MessageIDTypes
    {
        //
        // RESERVED TYPES - DO NOT CHANGE THESE
        // All types from RakPeer
        //

        // For the user to use.  Start your first enumeration at this value.
        ID_USER_PACKET_ENUM = 134,
        ID_USER_PACKET_135 = 135,
        ID_USER_PACKET_136 = 136,
    }

    public static class RakNetWrapper
    {
        public class Connection
        {
            public ulong guid;
            public string ip;
            public ushort port;
        }

#if UNITY_ANDROID
        const string DLL_NAME = "360Nt";
#elif UNITY_STANDALONE_OSX
		const string DLL_NAME ="RakNetBundle";
#elif UNITY_IPHONE
		const string DLL_NAME ="__Internal";
#else
        const string DLL_NAME = "libRakNet";
#endif
        [DllImport(DLL_NAME)]
        public static extern void RkN_Initialize();

        [DllImport(DLL_NAME)]
        public static extern void RkN_Initialize2(IntPtr pSettings, string dataPath, string persistentDataPath);

        [DllImport(DLL_NAME)]
        public static extern int RkN_CreatePeer();

        [DllImport(DLL_NAME)]
        public static extern int RkN_Startup(int handle, string ip, ushort port, uint maxConnections);

        [DllImport(DLL_NAME)]
        public static extern void RkN_ReleaseClient(int handle);

        [DllImport(DLL_NAME)]
        public static extern IntPtr RkN_Receive(int handle);

        [DllImport(DLL_NAME)]
        public static extern uint RkN_Send(int handle, byte[] data, int length, int priority, int reliability, sbyte orderingChannel, ulong guid, bool broadcast, int forceReceiptNumber = 0);

        [DllImport(DLL_NAME)]
        public static extern void RkN_Shutdown(int handle, int blockDuration = 0, byte orderingChannel = 0, int disconnectionNotificationPriority = 3);

        [DllImport(DLL_NAME)]
        public static extern void RkN_CloseConnection(int handle, ulong guid, bool sendDisconnectionNotification = true, byte orderingChannel = 0, int disconnectionNotificationPriority = 3);

        [DllImport(DLL_NAME)]
        public static extern void RkN_SetMaximumIncomingConnections(int handle, ushort numberAllowed);

        [DllImport(DLL_NAME)]
        public static extern int RkN_Connect(int handle, string host, ushort port, uint sendConnectionAttemptCount = 6, uint timeBetweenSendConnectionAttemptsMS = 1000, uint timeoutTime = 0);

        [DllImport(DLL_NAME)]
        public static extern void RkN_Broadcast(int handle, byte[] data, int length, int priority, int reliability, sbyte orderingChannel, IntPtr pGUIDList, int amount, int forceReceiptNumber = 0);

        [DllImport(DLL_NAME)]
        public static extern int RkN_GetAveragePing(int handle, ulong guid);

        [DllImport(DLL_NAME)]
        public static extern IntPtr RkN_ReceiveCycle(int handle, int packetRetrieveLimit = -1);

        [DllImport(DLL_NAME)]
        public unsafe static extern void* RkN_ReceiveCycle2(int handle, out int result);
        //public static extern IntPtr RkN_ReceiveCycle2(int handle, int packetRetrieveLimit = -1);


        [DllImport(DLL_NAME)]
        public static extern void RkN_ReleasePacketList(IntPtr packetList);

        [DllImport(DLL_NAME)]
        public static extern void RkN_Uninitialize();

        [DllImport(DLL_NAME)]
        public static extern int RkN_GetConnectionState(int handle, ulong guid);

        //for fast MemCopy
        [DllImport(DLL_NAME)]
        public static extern void MemCopy(IntPtr dest, IntPtr src, int length);

        //for zip Compress
        [DllImport(DLL_NAME)]
        public unsafe static extern int Compress(byte* dest, ulong* destLen, byte* source, ulong sourceLen);

        [DllImport(DLL_NAME)]
        public unsafe static extern int UnCompress(byte* dest, ulong* destLen, byte* source, ulong sourceLen);

        //for xLog
        [DllImport(DLL_NAME)]
        public static extern void RkN_xLog(int level, string content);

        [DllImport(DLL_NAME)]
        public static extern void RkN_xLogFlush(int param);

        //2019/1/18: 增加网络事件通知，用于更新IPStack
//        [DllImport(DLL_NAME)]
//        public static extern int RkN_NetEvent(int param);

    }
}