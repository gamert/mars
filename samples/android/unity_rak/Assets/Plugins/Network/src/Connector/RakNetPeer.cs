
using RakNet;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Comm.Network
{
    //1 = connected,2 = close,3 = errro
    public delegate void OnPeerState(RakNetPeer peer,int state, int reason = 0);
    //cmd for Raknet cmd:
    public delegate void OnPeerMsg(RakNetPeer peer, int cmd, byte[] data, int length);

    //实现一种
    public class RakNetPeer
    {
        public enum E_CONN_NOTIFY_REASON
        {
            eReason_Nothing = 0,
            eReason_ClosedByRemote,
            eReason_ClosedByOurs,
            eReason_AlreadyConnected,
            eReason_ConnectAttemptFailed
        }

        protected List<RakNetWrapper.Connection> _connectionList = new List<RakNetWrapper.Connection>();

        public int _handle = -1;
        protected string _host_ip;
        protected ushort _host_port;
        protected uint _max_connections = 1u;
        protected bool _started = false;

        public RakNetWrapper.Connection activeConn;    //当前活动链接...关闭后，则职

        public OnPeerState evt_OnState;     //1，2，3
        //public OnPeerState evt_OnConnected;
        //public OnPeerState evt_OnClosed;
        //public OnPeerState evt_OnError;
        public OnPeerMsg mCB;  //bind to the RakNetClientConnector. _internal_onPacketReceived

        public bool IsStarted
        {
            get { return _started; }
        }
        public string host_ip
        {
            get { return _host_ip; }
        }
        //给链接优先级，用于替换。。。UDP 默认为20 ,tcp 为10
        public int con_priority = 0;

        //
        public string Dump()
        {
            return string.Format("{0}(guid={1},port={2},state={3});", this.GetType().Name,
                (activeConn != null ? activeConn.guid : 0), 
                (activeConn != null ? activeConn.port : 0),
                GetConnectionState(0));
        }

        public void ClearConnections()
        {
            _connectionList.Clear();
        }

        public Comm.Network.BaseConnector mgr;

        public void Renew()
        {
            if (!_started && _handle != -1)
            {
                Startup(_host_ip, _host_port, _max_connections);
            }

            //对于客户端来说，renew后，连接全部失效。。
            ClearConnections();
        }


        //self imp...
        public virtual StartupResult Startup(string ip, ushort port, uint maxConnections)
        {
            return StartupResult.RAKNET_STARTED;
        }

        public virtual int GetAveragePing(int index)
        {
            return 0;
        }

        public virtual ConnectionState GetConnectionState(int index)
        {
            return ConnectionState.IS_NOT_CONNECTED;
        }

        public virtual void CloseConnection(int index = 0, bool sendDisconnectionNotification = true, byte orderingChannel = 0, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY)
        {
        }
        public virtual ConnectionAttemptResult Connect(string host, ushort port, uint sendConnectionAttemptCount = 3u, uint timeBetweenSendConnectionAttemptsMS = 1000u, uint timeoutTime = 10000u)
        {
            return ConnectionAttemptResult.UNKNOWN;
        }

        public virtual unsafe void ReceiveCycle2()
        {

        }

        public virtual uint Send(byte[] data, int length, int index = 0, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY, PacketReliability reliability = PacketReliability.RELIABLE_ORDERED, sbyte orderingChannel = 0, bool broadcast = false, int forceReceiptNumber = 0)
        {
            return 0;
        }

        public virtual void Shutdown(int blockDuration = 0, byte orderingChannel = 0, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY)
        {
            if (_started)
            {
                evt_OnState = null;
                //evt_OnConnected = null;
                //evt_OnClosed = null;
                //evt_OnError = null;
                mCB = null;
                _started = false;
            }
        }

        public virtual void Release()
        {
            _handle = -1;
        }

    }




    //
    class RakNetPeerUDP: RakNetPeer
    {
        private class _INTERNAL_CMD
        {
            public const int ID_CONNECTED_PING = 0;
            public const int ID_UNCONNECTED_PING = 1;            
            public const int ID_CONNECTION_REQUEST_ACCEPTED = 16;
            public const int ID_CONNECTION_ATTEMPT_FAILED = 17;
            public const int ID_ALREADY_CONNECTED = 18;
            public const int ID_NEW_INCOMING_CONNECTION = 19;
            public const int ID_DISCONNECTION_NOTIFICATION = 21; //服务器close me。。。DeadConnection
            public const int ID_CONNECTION_LOST = 22;            //我丢失服务器连接.. DeadConnection
            public const int ID_CONNECTION_BANNED = 23;
            public const int ID_INVALID_PASSWORD = 24;
            public const int ID_INCOMPATIBLE_PROTOCOL_VERSION = 25;
            public const int ID_NO_FREE_INCOMING_CONNECTIONS = 20;
        }


       // private static bool is_x64 = false;


        private static bool s_initialized = false;

        private const uint kMaxPeers = 128u;
        private const uint kPacketSwapPoolSize = 1024u;



        public static unsafe RakNetPeerUDP Create()
        {
            if (s_initialized == false)
            {
                _Rak_Settings_t conf;
                conf.maxNumberOfPeers = kMaxPeers;
                conf.packetSwapPoolSize = kPacketSwapPoolSize;

                IntPtr pSettings = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(_Rak_Settings_t)));
                Marshal.StructureToPtr(conf, pSettings, false);               
                RakNetWrapper.RkN_Initialize2(pSettings,UnityEngine.Application.dataPath, UnityEngine.Application.persistentDataPath);
                Marshal.DestroyStructure(pSettings, typeof(_Rak_Settings_t));

                //if (IntPtr.Size == 8)
                //    is_x64 = true;

                UnityEngine.Debug.Log("sizeof(_PacketHead_t)=" + sizeof(_PacketHead_t)+ ";IntPtr.Size="+ IntPtr.Size);

                s_initialized = true;
            }

            RakNetPeerUDP peer = new RakNetPeerUDP();
            peer._handle = RakNetWrapper.RkN_CreatePeer();  //创建一个peer:
            if (peer._handle >= 0)
                return peer;

            UnityEngine.Debug.LogError("RakNetPeerUDP create fail:");

            return null;
        }


        //启动
        public override StartupResult Startup(string ip, ushort port, uint maxConnections)
        {
            if (!_started)
            {
                StartupResult result = (StartupResult)RakNetWrapper.RkN_Startup(_handle, ip, port, maxConnections);
                if (result == StartupResult.RAKNET_STARTED)
                {
                    _host_ip = ip;
                    _host_port = port;
                    _max_connections = maxConnections;
                    _started = true;
                }
                con_priority = 20;
                return result;
            }

            return StartupResult.RAKNET_ALREADY_STARTED;
        }

        //从
        public override ConnectionState GetConnectionState(int index)
        {
            if (index >= 0 && index < _connectionList.Count)
            {
                return (ConnectionState)RakNetWrapper.RkN_GetConnectionState(_handle, _connectionList[index].guid);
            }

            return ConnectionState.IS_NOT_CONNECTED;
        }

        public void SetMaximumIncomingConnections(ushort numberAllowed)
        {
            if (_started)
            {
                RakNetWrapper.RkN_SetMaximumIncomingConnections(_handle, numberAllowed);
            }
        }

		private void InternalClose(RakNetWrapper.Connection conn, bool sendDisconnectionNotification = true, byte orderingChannel = 0, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY)
        {
            if (conn != null && _started)
            {
                RakNetWrapper.RkN_CloseConnection(_handle, conn.guid, sendDisconnectionNotification, orderingChannel, (int)priority);
            }
        }

		public void CloseConnection(string targetIp, ushort targetPort, bool sendDisconnectionNotification = true, byte orderingChannel = 0, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY)
        {
            RakNetWrapper.Connection conn = _connectionList.Find(
                delegate(RakNetWrapper.Connection raw)
                {
                    return raw.ip == targetIp && raw.port == targetPort;
                }
            );

            InternalClose(conn, sendDisconnectionNotification, orderingChannel, priority);
        }

		public override void CloseConnection(int index = 0, bool sendDisconnectionNotification = true, byte orderingChannel = 0, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY)
        {
            if (index >= 0 && index < _connectionList.Count)
            {
                InternalClose(_connectionList[index], sendDisconnectionNotification, orderingChannel, priority);
            }
        }

		public override void Shutdown(int blockDuration = 0, byte orderingChannel = 0, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY)
        {
            if (_started)
            {
                for (int ii = 0; ii < _connectionList.Count; ++ii)
                    InternalClose(_connectionList[ii], true, orderingChannel, priority);

                RakNetWrapper.RkN_Shutdown(_handle, blockDuration, orderingChannel, (int)priority);
                _started = false;
            }
        }

        public override void Release ()
        {
                RakNetWrapper.RkN_ReleaseClient(_handle);
                _handle = -1;
        }

        uint InternalSend(RakNetWrapper.Connection conn, byte[] data, int length, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY, PacketReliability reliability = PacketReliability.RELIABLE_ORDERED, sbyte orderingChannel = 0, bool broadcast = false, int forceReceiptNumber = 0)
        {
            uint result = 0u;
            if (conn != null)
            {
//                UnityEngine.Debug.LogWarning("["+UnityEngine.Time.frameCount+"]InternalSend:length=" + length+ ";reliability="+ reliability);
                result = RakNetWrapper.RkN_Send(_handle, data, length, (int)priority, (int)reliability, orderingChannel, conn.guid, broadcast, forceReceiptNumber);
            }
            return result;
        }

        uint Send(byte[] data, int length, ulong guid, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY, PacketReliability reliability = PacketReliability.RELIABLE_ORDERED, sbyte orderingChannel = 0, bool broadcast = false, int forceReceiptNumber = 0)
        {
            uint result = 0u;
            if (_started)
            {
                RakNetWrapper.Connection conn = _connectionList.Find(
                    delegate(RakNetWrapper.Connection raw)
                    {
                        return raw.guid == guid;                        
                    }
                );

                result = InternalSend(conn, data, length, priority, reliability, orderingChannel, broadcast, forceReceiptNumber);
            }

            return result;
        }

        public void Broadcast(byte[] data, int length, ulong[] guidList, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY, PacketReliability reliability = PacketReliability.RELIABLE_ORDERED, sbyte orderingChannel = 0, int forceReceiptNumber = 0)
        {
            if (_started && guidList != null && guidList.Length > 0)
            {
                IntPtr pGUIDList = Marshal.AllocHGlobal(sizeof(ulong) * guidList.Length);
                for (int ii = 0; ii < guidList.Length; ++ii)
                {
                    Marshal.WriteInt64(pGUIDList, ii * sizeof(ulong), (long)guidList[ii]);
                }
                RakNetWrapper.RkN_Broadcast(_handle, data, length, (int)priority, (int)reliability, orderingChannel, pGUIDList, guidList.Length, forceReceiptNumber);
                Marshal.FreeHGlobal(pGUIDList);
            }
        }

        public override uint Send(byte[] data, int length, int index = 0, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY, PacketReliability reliability = PacketReliability.RELIABLE_ORDERED, sbyte orderingChannel = 0, bool broadcast = false, int forceReceiptNumber = 0)
        {
            uint result = 0u;
            if (_started && index >= 0 && index < _connectionList.Count)
            {
                RakNetWrapper.Connection conn = _connectionList[index];
                result = InternalSend(conn, data, length, priority, reliability, orderingChannel, broadcast, forceReceiptNumber);
            }
            return result;
        }

        public override ConnectionAttemptResult Connect(string host, ushort port, uint sendConnectionAttemptCount = 3u, uint timeBetweenSendConnectionAttemptsMS = 1000u, uint timeoutTime = 10000u)
        {
            return (ConnectionAttemptResult)RakNetWrapper.RkN_Connect(_handle, host, port, sendConnectionAttemptCount, timeBetweenSendConnectionAttemptsMS, timeoutTime);            
        }
        
        public int GetAveragePing (string host, ushort port)
        {
            if (_started)
            {
                RakNetWrapper.Connection conn = _connectionList.Find(
                    delegate (RakNetWrapper.Connection raw)
                    {
                        return raw.ip == host && raw.port == port;
                    }
                );

                if (conn != null)
                    return Math.Min(RakNetWrapper.RkN_GetAveragePing(_handle, conn.guid), 1000);
            }

            return 1000;
        }

        public override int GetAveragePing (int index)
        {
            if (_started && index >= 0 && index < _connectionList.Count)
            {
                RakNetWrapper.Connection conn = _connectionList[index];
                return Math.Min(RakNetWrapper.RkN_GetAveragePing(_handle, conn.guid), 1000);
            }

            return 1000;
        }

        public bool ReceivePacket(OnPacketReceived cb)
        {
            if (_started)
            {
            //ZZTODO:
                //IntPtr pPacket = RakNetWrapper.RkN_Receive(_handle);
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
        public void ReceiveCycle(OnPacketReceived cb)
        {
            if (_started)
            {
            //ZZTODO:
                //IntPtr pList = IntPtr.Zero;
                //int amount = 0;
                //do
                //{
                //    pList = RakNetWrapper.RkN_ReceiveCycle(_handle);
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
        public unsafe static int SubtractPtr32(byte* ptr1, byte* ptr2)
        {
            return (int)(ptr1 - ptr2);
        }
        public static int AlignSize(int size, int alignment)
        {
            size += --alignment;
            size &= ~alignment;
            return size;
        }
        //C# for cpp call:
        //@packBufferPtr: cpp buffer ptr
        private unsafe static void FlushCppImmediatePackBuffer(void* packBufferPtr)
        {
            byte* ptr = (byte*)packBufferPtr;
            int num = *(int*)ptr;
            ptr += 4;
            //SGW.wrapperFuncs[num](ref ptr);
        }

        //public unsafe void LogPacket(byte* ptr)
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

        public override unsafe void ReceiveCycle2()
        {
            if (_started)
            {
                int result = 0;
                //UnityEngine.Profiler.BeginSample("RakNetWrapper.RkN_ReceiveCycle2");//0B,0ms
                byte* ptr = (byte*)RakNetWrapper.RkN_ReceiveCycle2(_handle,out result) ;   //
                //UnityEngine.Profiler.EndSample();
                if (ptr!=null)
                {
                    int left = (result & 0xFFFF) >> 16;
                    int amount = (result & 0xFFFF);
                    int i = 0;
                    if(amount > 0)
                    {
                        //RakNetWrapper.RkN_xLog(1, "ReceiveCycle2:amount="+ amount);

                        for (; i < amount; ++i)
                        {
                            //LogPacket(ptr);

                            //UnityEngine.Profiler.BeginSample("RakNetWrapper._PacketHead_t*");
                            _PacketHead_t* pkt = (_PacketHead_t*)ptr;
                            //UnityEngine.Profiler.EndSample();

                           // UnityEngine.Profiler.BeginSample("RakNetWrapper.ProcessPacket");  //376B,0.11ms;44B = 上层；call进去之后，即损失24B
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

        unsafe void safe_add_conn(_PacketHead_t* pkt)
        {
            activeConn = _connectionList.Find(
                delegate (RakNetWrapper.Connection raw)
                {
                    return raw.guid == pkt->guid;
                }
            );

            if (activeConn == null)
            {
                UnityEngine.Debug.Log("Rak:ProcessPacket:pkt->guid" + pkt->guid + ";control_cmd=" + pkt->control_cmd);
                activeConn = new RakNetWrapper.Connection();
                activeConn.guid = pkt->guid;
                //   conn.ip = pkt->source_ip;		//ZZTODO
                activeConn.port = pkt->source_port;
                _connectionList.Add(activeConn);
            }
        }


        /// <summary>
        /// [EDITOR]zzTODO:不确定是否因为unsafe的关系，会导致本函数的call 增加20B
        /// 1. int转object,损失20B
        /// 2. 
        /// </summary>
        /// <param name="pkt"></param>
        /// <param name="__ptr"></param>
        private unsafe void ProcessPacket (_PacketHead_t *pkt, byte* __ptr)
        {
            //UnityEngine.Debug.Log(string.Format("pkt[Ord{0},Seq{1}] Reli={2};local_id={3};Len={4}",
            //    pkt->orderingIndex, pkt->sequencingIndex, pkt->reliability, pkt->local_id, pkt->data_len));

            //BaseConnector.Log("pkt="+);

            if (pkt->control_cmd == _INTERNAL_CMD.ID_CONNECTION_REQUEST_ACCEPTED
                || pkt->control_cmd == _INTERNAL_CMD.ID_NEW_INCOMING_CONNECTION
//               || pkt->control_cmd == _INTERNAL_CMD.ID_ALREADY_CONNECTED
               )
            {
                safe_add_conn(pkt);
                // : (int)E_CONN_NOTIFY_REASON.eReason_AlreadyConnected
                //if (evt_OnConnected != null)
                //    evt_OnConnected(this,(int)pkt->control_cmd);
                if (evt_OnState!=null)
                    evt_OnState(this, 1, (int)pkt->control_cmd);
            }
            else if (pkt->control_cmd == _INTERNAL_CMD.ID_CONNECTION_LOST
                || pkt->control_cmd == _INTERNAL_CMD.ID_DISCONNECTION_NOTIFICATION)
            {
                RakNetWrapper.Connection conn = _connectionList.Find(
                    delegate (RakNetWrapper.Connection raw)
                    {
                        return raw.guid == pkt->guid;
                    }
                );

                Comm.Network.BaseConnector.Log(string.Format("ProcessPacket:{0};guid={1};conn={2}", pkt->control_cmd , pkt->guid, conn));

                if (conn != null)
                {
                    _connectionList.Remove(conn);
                    if (evt_OnState != null && activeConn == conn)
                    {
                        evt_OnState(this, 2, pkt->control_cmd == _INTERNAL_CMD.ID_DISCONNECTION_NOTIFICATION ? (int)E_CONN_NOTIFY_REASON.eReason_ClosedByRemote : (int)E_CONN_NOTIFY_REASON.eReason_ClosedByOurs);
                        //关闭
                        activeConn = null;
                    }
                    else
                    {
                        Comm.Network.BaseConnector.Log(string.Format("ProcessPacket LOST:{0};guid={1};activeConn={2}", pkt->control_cmd, pkt->guid, activeConn));
                    }
                }
            }
            else if (pkt->control_cmd == _INTERNAL_CMD.ID_CONNECTION_ATTEMPT_FAILED)
            {
                if (evt_OnState != null)
                    evt_OnState(this,3, (int)E_CONN_NOTIFY_REASON.eReason_ConnectAttemptFailed);
            }
            else if (pkt->control_cmd == _INTERNAL_CMD.ID_ALREADY_CONNECTED)
            {
                safe_add_conn(pkt);
                if (evt_OnState != null)
                    evt_OnState(this, 3,(int)E_CONN_NOTIFY_REASON.eReason_AlreadyConnected);
            }

            if (pkt->data_len > 0 && mCB != null)
            {
                //
                byte[] data = (pkt->data_len> 2048) ? new byte[(int)pkt->data_len]: s_data;
                fixed (byte* value = data)
                {
                    RakNetWrapper.MemCopy(new IntPtr((void*)value), new IntPtr(__ptr), (int)pkt->data_len);
                }

                //UnityEngine.Profiler.BeginSample("RakNetWrapper.ProcessPacket.cb"); //380B,0.05ms(+24B) /20B -->0B; 0.03ms
                //pkt->guid
                mCB(this, pkt->control_cmd, data, (int)pkt->data_len);
                //UnityEngine.Profiler.EndSample();
            }
        }//end 
        static byte[] s_data = new byte[2048];   //临时fix mem:

    }//end class     
}
