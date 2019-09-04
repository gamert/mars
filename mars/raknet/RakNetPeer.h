#ifndef RAKNET_PEER_H
#define RAKNET_PEER_H

#include "RakNetWrapper.h"
using namespace RakNet;
namespace RakNet
{
    //实现一种
    class RakNetPeer
    {
	public:
		enum class E_CONN_NOTIFY_REASON
		{
			eReason_Nothing = 0,
			eReason_ClosedByRemote,
			eReason_ClosedByOurs,
			eReason_AlreadyConnected,
			eReason_ConnectAttemptFailed
		};

        std::vector<RakNetWrapper::Connection*> _connectionList ;
		
		RakNetWrapper::Connection* FindConnection(ulong guid,bool bRemove = false)
		{
			for (int i = 0; i < (int)_connectionList.size(); ++i)
			{
				if (_connectionList[i]->guid == guid)
				{
					RakNetWrapper::Connection* res = _connectionList[i];
					if (bRemove)
						_connectionList.erase(_connectionList.begin()+i);
					return res;
				}
			}
			return NULL;
		}


        int _handle = -1;
        std::string _host_ip;
        ushort _host_port;
        uint _max_connections = 1u;
        bool _started = false;

        OnConnectionNotify evt_OnConnected;
        OnConnectionNotify evt_OnClosed;
        OnConnectionNotify evt_OnError;
        OnPacketReceived mCB ;

        bool IsStarted()
        {
            return _started;
        }
		std::string host_ip()
        {
            return _host_ip;
        }

        void ClearConnections()
        {
            _connectionList.clear();
        }

        //MobaGo.Network.BaseConnector mgr;

        void Renew()
        {
            if (!_started && _handle != -1)
            {
                Startup(_host_ip, _host_port, _max_connections);
            }

            //对于客户端来说，renew后，连接全部失效。。
            ClearConnections();
        }


        //self imp...
        virtual StartupResult Startup(string ip, ushort port, uint maxConnections)
        {
            return StartupResult::RAKNET_STARTED;
        }

        virtual int GetAveragePing(int index)
        {
            return 0;
        }

        virtual ConnectionState GetConnectionState(int index)
        {
            return ConnectionState::IS_NOT_CONNECTED;
        }

        virtual void CloseConnection(int index = 0, bool sendDisconnectionNotification = false, byte orderingChannel = 0, PacketPriority priority = PacketPriority::IMMEDIATE_PRIORITY)
        {
        }
        virtual ConnectionAttemptResult Connect(string host, ushort port, uint sendConnectionAttemptCount = 3u, uint timeBetweenSendConnectionAttemptsMS = 1000u, uint timeoutTime = 10000u)
        {
            return ConnectionAttemptResult::UNKNOWN;
        }

        virtual void ReceiveCycle2()
        {

        }

        virtual uint Send(byte *data, int length, int index = 0, PacketPriority priority = PacketPriority::IMMEDIATE_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, sbyte orderingChannel = 0, bool broadcast = false, int forceReceiptNumber = 0)
        {
            return 0;
        }

        virtual void Shutdown(int blockDuration = 0, byte orderingChannel = 0, PacketPriority priority = PacketPriority::IMMEDIATE_PRIORITY)
        {
            if (_started)
            {
                evt_OnConnected = null;
                evt_OnClosed = null;
                evt_OnError = null;
                mCB = null;
                _started = false;
            }
        }

        virtual void Release()
        {
            _handle = -1;
        }

	};




    //
    class RakNetPeerUDP: public RakNetPeer
    {
	public:
        class _INTERNAL_CMD
        {
		public:
            const static int ID_CONNECTED_PING = 0;
            const static int ID_UNCONNECTED_PING = 1;
            const static int ID_CONNECTION_REQUEST_ACCEPTED = 16;
            const static int ID_CONNECTION_ATTEMPT_FAILED = 17;
            const static int ID_ALREADY_CONNECTED = 18;
            const static int ID_NEW_INCOMING_CONNECTION = 19;
            const static int ID_DISCONNECTION_NOTIFICATION = 21; //服务器close me。。。DeadConnection
            const static int ID_CONNECTION_LOST = 22;            //我丢失服务器连接.. DeadConnection
            const static int ID_CONNECTION_BANNED = 23;
            const static int ID_INVALID_PASSWORD = 24;
            const static int ID_INCOMPATIBLE_PROTOCOL_VERSION = 25;
            const static int ID_NO_FREE_INCOMING_CONNECTIONS = 20;
		};


       // static bool is_x64 = false;


		static bool s_initialized;// = false;

        const static uint kMaxPeers = 128u;
        const static uint kPacketSwapPoolSize = 1024u;


        static RakNetPeer *Create()
        {
            if (s_initialized == false)
            {
                _Rak_Settings_t conf;
                conf.maxNumberOfPeers = kMaxPeers;
                conf.packetSwapPoolSize = kPacketSwapPoolSize;
         
                RkN_Initialize2(&conf,"", "");

                //if (IntPtr.Size == 8)
                //    is_x64 = true;

                //UnityEngine.Debug.Log("sizeof(_PacketHead_t)=" + sizeof(_PacketHead_t)+ ";IntPtr.Size="+ IntPtr.Size);

                s_initialized = true;
            }

            RakNetPeer *peer = new RakNetPeerUDP();
            peer->_handle = RkN_CreatePeer();  //创建一个peer:
            if (peer->_handle >= 0)
                return peer;

            return null;
        }


        //启动
        virtual StartupResult Startup(string ip, ushort port, uint maxConnections)
        {
            if (!_started)
            {
                StartupResult result = (StartupResult)RkN_Startup(_handle, ip.c_str(), port, maxConnections);
                if (result == StartupResult::RAKNET_STARTED)
                {
                    _host_ip = ip;
                    _host_port = port;
                    _max_connections = maxConnections;
                    _started = true;
                }

                return result;
            }

            return StartupResult::RAKNET_ALREADY_STARTED;
        }

        //从
        ConnectionState GetConnectionState(int index)
        {
            if (index >= 0 && index < (int)_connectionList.size())
            {
                return (ConnectionState)RkN_GetConnectionState(_handle, _connectionList[index]->guid);
            }

            return ConnectionState::IS_NOT_CONNECTED;
        }

        void SetMaximumIncomingConnections(ushort numberAllowed)
        {
            if (_started)
            {
               // RkN_SetMaximumIncomingConnections(_handle, numberAllowed);
            }
        }

		void InternalClose(RakNetWrapper::Connection *conn, bool sendDisconnectionNotification = false, byte orderingChannel = 0, PacketPriority priority = PacketPriority::IMMEDIATE_PRIORITY)
        {
            if (conn != null && _started)
            {
                RkN_CloseConnection(_handle, conn->guid, sendDisconnectionNotification, orderingChannel, (int)priority);
            }
        }

		void CloseConnection(string targetIp, ushort targetPort, bool sendDisconnectionNotification = false, byte orderingChannel = 0, PacketPriority priority = PacketPriority::IMMEDIATE_PRIORITY)
        {
            //Connection conn = _connectionList.Find(
            //    delegate(Connection raw)
            //    {
            //        return raw.ip == targetIp && raw.port == targetPort;
            //    }
            //);

            //InternalClose(conn, sendDisconnectionNotification, orderingChannel, priority);
        }

		void CloseConnection(int index = 0, bool sendDisconnectionNotification = false, byte orderingChannel = 0, PacketPriority priority = PacketPriority::IMMEDIATE_PRIORITY)
        {
            if (index >= 0 && index < (int)_connectionList.size())
            {
                InternalClose(_connectionList[index], sendDisconnectionNotification, orderingChannel, priority);
            }
        }

		void Shutdown(int blockDuration = 0, byte orderingChannel = 0, PacketPriority priority = PacketPriority::IMMEDIATE_PRIORITY)
        {
            if (_started)
            {
                for (int ii = 0; ii < (int)_connectionList.size(); ++ii)
                    InternalClose(_connectionList[ii], true, orderingChannel, priority);

                RkN_Shutdown(_handle, blockDuration, orderingChannel, (int)priority);
                _started = false;
            }
        }

        void Release ()
        {
                RkN_ReleaseClient(_handle);
                _handle = -1;
        }

        uint InternalSend(RakNetWrapper::Connection *conn, byte *data, int length, PacketPriority priority = PacketPriority::IMMEDIATE_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, sbyte orderingChannel = 0, bool broadcast = false, int forceReceiptNumber = 0)
        {
            uint result = 0u;
            if (conn != null)
            {
//                //UnityEngine.Debug.LogWarning("["+//UnityEngine.Time.frameCount+"]InternalSend:length=" + length+ ";reliability="+ reliability);
                result = RkN_Send(_handle, (const char *)data, length, (int)priority, (int)reliability, orderingChannel, conn->guid, broadcast, forceReceiptNumber);
            }
            return result;
        }

        uint Send(byte *data, int length, ulong guid, PacketPriority priority = PacketPriority::IMMEDIATE_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, sbyte orderingChannel = 0, bool broadcast = false, int forceReceiptNumber = 0)
        {
            uint result = 0u;
            if (_started)
            {
				RakNetWrapper::Connection *conn = FindConnection(guid);// _connectionList.Find(
                //    delegate(RakNetWrapper::Connection raw)
                //    {
                //        return raw.guid == guid;                        
                //    }
                //);

                result = InternalSend(conn, data, length, priority, reliability, orderingChannel, broadcast, forceReceiptNumber);
            }

            return result;
        }

        //void Broadcast(byte *data, int length, ulong[] guidList, PacketPriority priority = PacketPriority::IMMEDIATE_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, sbyte orderingChannel = 0, int forceReceiptNumber = 0)
        //{
        //    if (_started && guidList != null && guidList.Length > 0)
        //    {
        //        IntPtr pGUIDList = Marshal.AllocHGlobal(sizeof(ulong) * guidList.Length);
        //        for (int ii = 0; ii < guidList.Length; ++ii)
        //        {
        //            Marshal.WriteInt64(pGUIDList, ii * sizeof(ulong), (long)guidList[ii]);
        //        }
        //        RkN_Broadcast(_handle, data, length, (int)priority, (int)reliability, orderingChannel, pGUIDList, guidList.Length, forceReceiptNumber);
        //        Marshal.FreeHGlobal(pGUIDList);
        //    }
        //}

        uint Send(byte *data, int length, int index = 0, PacketPriority priority = PacketPriority::IMMEDIATE_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, sbyte orderingChannel = 0, bool broadcast = false, int forceReceiptNumber = 0)
        {
            uint result = 0u;
            if (_started && index >= 0 && index < (int)_connectionList.size())
            {
                RakNetWrapper::Connection *conn = _connectionList[index];
                result = InternalSend(conn, data, length, priority, reliability, orderingChannel, broadcast, forceReceiptNumber);
            }
            return result;
        }

        ConnectionAttemptResult Connect(string host, ushort port, uint sendConnectionAttemptCount = 3u, uint timeBetweenSendConnectionAttemptsMS = 1000u, uint timeoutTime = 10000u)
        {
            return (ConnectionAttemptResult)RkN_Connect(_handle, host.c_str(), port, sendConnectionAttemptCount, timeBetweenSendConnectionAttemptsMS, timeoutTime);            
        }
        
        int GetAveragePing (string host, ushort port)
        {
            //if (_started)
            //{
            //    RakNetWrapper::Connection conn = _connectionList.Find(
            //        delegate (RakNetWrapper::Connection raw)
            //        {
            //            return raw.ip == host && raw.port == port;
            //        }
            //    );

            //    if (conn != null)
            //        return Math.Min(RkN_GetAveragePing(_handle, conn.guid), 1000);
            //}

            return 1000;
        }

        int GetAveragePing (int index)
        {
            //if (_started && index >= 0 && index < _connectionList.size())
            //{
            //    RakNetWrapper::Connection conn = _connectionList[index];
            //    return Math.Min(RkN_GetAveragePing(_handle, conn.guid), 1000);
            //}

            return 1000;
        }

        bool ReceivePacket(OnPacketReceived cb)
        {
            if (_started)
            {
            //ZZTODO:
                //IntPtr pPacket = RkN_Receive(_handle);
                //bool isValidPointer = is_x64 ? pPacket.ToInt64() != 0 : pPacket.ToInt32() != 0;
                //object obj = isValidPointer ? Marshal.PtrToStructure(pPacket, typeof(_Packet_t)) : null;

                //if (obj != null)
                //{
                //    _Packet_t pkt = (_Packet_t)obj;
                //    ProcessPacket(pkt, cb);
                //    return true;
                //}
            }

            return false;
        }

        //返回byte 数组指针，大约计算一下每s所产生的报文多少？ 
        //使用c-c#交换缓存进行快速包交换处理..
        void ReceiveCycle(OnPacketReceived cb)
        {
            if (_started)
            {
            //ZZTODO:
                //IntPtr pList = IntPtr.Zero;
                //int amount = 0;
                //do
                //{
                //    pList = RkN_ReceiveCycle(_handle);
                //    amount = 0;                    
                //    if (!is_x64 ? pList.ToInt32() != 0 : pList.ToInt64() != 0)
                //    {
                //        _Packet_List_t pktListContainer = (_Packet_List_t)Marshal.PtrToStructure(pList, typeof(_Packet_List_t));
                //        amount = pktListContainer.amount;
                //        for (int ii = 0; ii < amount; ++ii)
                //        {
                //            IntPtr atom = Marshal.ReadIntPtr(pktListContainer.ppPktList, ii * IntPtr.Size);
                //            _Packet_t pkt = (_Packet_t)Marshal.PtrToStructure(atom, typeof(_Packet_t));
                //            ProcessPacket(pkt, cb);
                //        }
                //    }
                //} while (amount > 0);
            }
        }
        static int SubtractPtr32(byte* ptr1, byte* ptr2)
        {
            return (int)(ptr1 - ptr2);
        }
        static int AlignSize(int size, int alignment)
        {
            size += --alignment;
            size &= ~alignment;
            return size;
        }
        //C# for cpp call:
        //@packBufferPtr: cpp buffer ptr
        static void FlushCppImmediatePackBuffer(void* packBufferPtr)
        {
            byte* ptr = (byte*)packBufferPtr;
            //int num = *(int*)ptr;
            ptr += 4;
            //SGW.wrapperFuncs[num](ref ptr);
        }

        //void LogPacket(byte* ptr)
        //{
        //    FileStream fs = new FileStream("LogPacket.dat", FileMode.OpenOrCreate);
        //    if(fs!=null)
        //    {
        //        //数据保存到磁盘中
        //        BinaryWriter bw = new BinaryWriter(fs);
        //        if(bw!=null)
        //        {
        //            _PacketHead_t* pkt = (_PacketHead_t*)ptr;
        //            int len = sizeof(_PacketHead_t) + pkt->data_len;
        //            bw.Write(len);
        //            for(int i = 0;i < len;++i)
        //                bw.Write(ptr[i]);
        //            bw.Flush();
        //            bw.Close();
        //        }
        //        fs.Close();
        //    }
        //}

        void ReceiveCycle2()
        {
            if (_started)
            {
                int result = 0;
                //UnityEngine.Profiler.BeginSample("RkN_ReceiveCycle2");//0B,0ms
                byte* ptr = (byte*)RkN_ReceiveCycle2(_handle,&result) ;   //
                //UnityEngine.Profiler.EndSample();
                if (ptr!=null)
                {
                    //int left = (result & 0xFFFF) >> 16;
                    int amount = (result & 0xFFFF);
                    int i = 0;
                    if(amount > 0)
                    {
                        //RkN_xLog(1, "ReceiveCycle2:amount="+ amount);

                        for (; i < amount; ++i)
                        {
                            //LogPacket(ptr);

                            //UnityEngine.Profiler.BeginSample("_PacketHead_t*");
                            _PacketHead_t* pkt = (_PacketHead_t*)ptr;
                            //UnityEngine.Profiler.EndSample();

                           // UnityEngine.Profiler.BeginSample("ProcessPacket");  //376B,0.11ms;44B = 上层；call进去之后，即损失24B
                            ProcessPacket(pkt, ptr + sizeof(_PacketHead_t));
                          //  UnityEngine.Profiler.EndSample();

                            int len = sizeof(_PacketHead_t) + pkt->data_len;
                            ptr += AlignSize(len, 4);
                        }

                    }
                    //catch(Exception e)
                    //{
                    //    UnityEngine.Debug.LogError(i+"/"+ amount+":" +e.ToString());
                    //}
                }//end ptr
            }
        }

        /// <summary>
        /// [EDITOR]zzTODO:不确定是否因为unsafe的关系，会导致本函数的call 增加20B
        /// 1. int转object,损失20B
        /// 2. 
        /// </summary>
        /// <param name="pkt"></param>
        /// <param name="__ptr"></param>
        void ProcessPacket (_PacketHead_t *pkt, byte* __ptr)
        {
           if (pkt->control_cmd == _INTERNAL_CMD::ID_CONNECTION_REQUEST_ACCEPTED
                || pkt->control_cmd == _INTERNAL_CMD::ID_NEW_INCOMING_CONNECTION
//               || pkt->control_cmd == _INTERNAL_CMD::ID_ALREADY_CONNECTED
               )
            {
				RakNetWrapper::Connection *conn = FindConnection(pkt->guid);// _connectionList.Find(
                //    delegate (RakNetWrapper::Connection raw)
                //    {
                //        return raw.guid == pkt->guid;
                //    }
                //);

                if (conn == null)
                {
                    //UnityEngine.Debug.Log("Rak:ProcessPacket:pkt->guid"+ pkt->guid+ ";control_cmd="+ pkt->control_cmd);
                    conn = new RakNetWrapper::Connection();
                    conn->guid = pkt->guid;
                 //   conn.ip = pkt->source_ip;		//ZZTODO
                    conn->port = pkt->source_port;
                    _connectionList.push_back(conn);
                }
                // : (int)E_CONN_NOTIFY_REASON::eReason_AlreadyConnected
                if (evt_OnConnected != null)
                    evt_OnConnected(conn,(int)pkt->control_cmd);
            }
            else if (pkt->control_cmd == _INTERNAL_CMD::ID_CONNECTION_LOST
                || pkt->control_cmd == _INTERNAL_CMD::ID_DISCONNECTION_NOTIFICATION)
            {
               // MobaGo.Network.BaseConnector.Log("ProcessPacket:" + pkt->control_cmd+ ";guid="+ pkt->guid);

				RakNetWrapper::Connection *conn = FindConnection(pkt->guid,true);// _connectionList.Find(
                //    delegate (RakNetWrapper::Connection raw)
                //    {
                //        return raw.guid == pkt->guid;
                //    }
                //);

                if (conn != null)
                {
                    //_connectionList.erase(conn);
                    if (evt_OnClosed != null)
                        evt_OnClosed(conn, pkt->control_cmd == _INTERNAL_CMD::ID_DISCONNECTION_NOTIFICATION ? (int)E_CONN_NOTIFY_REASON::eReason_ClosedByRemote : (int)E_CONN_NOTIFY_REASON::eReason_ClosedByOurs);
                }
            }
            else if (pkt->control_cmd == _INTERNAL_CMD::ID_CONNECTION_ATTEMPT_FAILED)
            {
                if (evt_OnError != null)
                    evt_OnError(null, (int)E_CONN_NOTIFY_REASON::eReason_ConnectAttemptFailed);
            }
            else if (pkt->control_cmd == _INTERNAL_CMD::ID_ALREADY_CONNECTED)
            {
                if (evt_OnError != null)
                    evt_OnError(null, (int)E_CONN_NOTIFY_REASON::eReason_AlreadyConnected);
            }

            if (pkt->data_len > 0 && mCB != null)
            {
                //
                byte *data = (pkt->data_len> 512) ? new byte[(int)pkt->data_len]: s_data;
                if (byte* value = data)
                {
                    memcpy(value, __ptr, (int)pkt->data_len);
                }

                //UnityEngine.Profiler.BeginSample("ProcessPacket.cb"); //380B,0.05ms(+24B) /20B -->0B; 0.03ms
                mCB(pkt->guid, pkt->control_cmd, data, (int)pkt->data_len);
                //UnityEngine.Profiler.EndSample();
            }
        }//end 
        static byte s_data[512];   //临时fix mem:

	};//end class     
}

#endif