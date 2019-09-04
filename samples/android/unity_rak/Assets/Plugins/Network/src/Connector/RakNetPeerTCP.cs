#define USE_FFF

using FlatBuffers;
using RakNet;
using System;
using System.Collections;
using System.Collections.Generic;


namespace Comm.Network
{
    //TCP连接会话
    class TcpSession
    {
        //
        public int doConnect(string host, ushort port, Comm.Network.BaseConnector mgr)
        {
            if (m_Socket == null)
            {
                m_Socket = new TcpSocketImp();

                m_Socket.Connected += new EventHandler(clientTcmSocket_Connected);
                m_Socket.Closed += new EventHandler(clientTcmSocket_Closed);
                m_Socket.DataReceived += new EventHandler<DataEventArgs>(clientTcmSocket_DataReceived);
                m_Socket.TimeOut += new EventHandler(clientTcmSocket_TimeOut);
            }
            int res = m_Socket.connectServer(host, port, mgr);
            return res;
        }
        public void pickMsgQueue()
        {
            lock (((ICollection)mQueue).SyncRoot)
            {
                for (int i = 0; i < mQueue.Count; i++)
                {
                    mProcessList.Add(mQueue[i]);
                }
                mQueue.Clear();
            }
        }


        /// <summary>
        /// ///////////////////////////////////////////////////////////////////////////////////////
        /// </summary>
        //zzAdd for tcp：
        const int HEAD_LENGTH = 4;
        private object queueLock = new object();
        public bool needSync = false;   //是否需要同步小兵

        private List<BaseStructRev> mQueue = new List<BaseStructRev>();    //txzNote: for net threads 
        // private List<BaseStructRev> mQueue2 = new List<BaseStructRev>();   //txzAdd: for net threads  and mainthread.
        public List<BaseStructRev> mProcessList = new List<BaseStructRev>(32);//txzNote: mainthread

        //如果网络不可达，则先加入请求等待队列
        //private Queue<BaseStructSend> mWaitQueue = new Queue<BaseStructSend>();

        //private TEventDispatcher<SocketManager> m_eventDispatcher;
        private byte[] lastLeftByteCombat;
        private void SetLastByte(byte[] mbyte)
        {
            lastLeftByteCombat = mbyte;
        }
        public TcpSocketImp m_Socket;

        //
        private void clientTcmSocket_Connected(object sender, EventArgs e)
        {
            //TDebug.LogError("clientTcmSocket_Connected");
            //lock (((ICollection)mQueue).SyncRoot)
            {
              //  EnqueueImp(BaseStructRevPool.SafeCreate((ushort)EMainProtcol.CONNECT_MAIN_COMMAND, (ushort)EConnectSubProtcol.SERVER_CONNECTED, (sender as SocketBase)));
            }
        }
        private void clientTcmSocket_Closed(object sender, EventArgs e)
        {
            //lock (((ICollection)mQueue).SyncRoot)
            {
               // EnqueueImp(BaseStructRevPool.SafeCreate((ushort)EMainProtcol.CONNECT_MAIN_COMMAND, (ushort)EConnectSubProtcol.SERVER_DISCONNECTED, (sender as SocketBase)));
            }
        }

        private void clientTcmSocket_TimeOut(object sender, EventArgs e)
        {
            //lock (((ICollection)mQueue).SyncRoot)
            {
            //    EnqueueImp(BaseStructRevPool.SafeCreate((ushort)EMainProtcol.CONNECT_MAIN_COMMAND, (ushort)EConnectSubProtcol.SERVER_TIMEOUT, (sender as SocketBase)));
            }
        }

        //中转指令
        const ushort _subCommand = 134; //参考: kCustomUserMsg

        //txzFix: net threads lock.
        int reenter = 0;
        private void clientTcmSocket_DataReceived(object sender, DataEventArgs e)
        {
            SocketBase pSock = (sender as SocketBase);
#if ENABLE_PROFILER
            if (reenter > 0)
            { }
            //TDebug.LogError("pre reenter=" + reenter);
            else
                reenter++;
#endif
            // lock (((ICollection)mQueue).SyncRoot)
            lock (queueLock)
            {
                //zzAdd for test...
#if LOG_NET
//                ServerPackInfo.GetInstance().AddMsgRaw(1, e.Data, e.Length);
#endif
                //==

                // TDebug.Log("DataReceived:ThreadId=" + Thread.CurrentThread.ManagedThreadId);
                byte[] mRev = e.Data;
                int offset = e.Offset;
                int bufferLength = e.Length;
#if USE_FFF
                SplitPackage(pSock, mRev, offset, bufferLength);
#else
                //zz: 来自于内部命令...
                //TODO: 优化...(ushort)EConnectSubProtcol.SERVER_DATA
                //BaseStructRev mStruct = BaseStructRevPool.SafeCreate((ushort)EMainProtcol.PVP_MAIN_COMMAND, _subCommand, pSock);
                //byte[] data = new byte[bufferLength];
                //Buffer.BlockCopy(mRev, 0, data, 0, bufferLength);
                //mStruct.data = data;
                //EnqueueImp(mStruct);
#endif
            }
#if ENABLE_PROFILER
            reenter--;
            //if (reenter > 0)
            //    TDebug.LogError("post reenter=" + reenter);
#endif
        }
#if USE_FFF
        /// <summary>
        /// split package from server
        /// </summary>
        /// <param name="bytes"></param>
        /// <param name="index"></param>
        void SplitPackage(SocketBase pSock, byte[] bytes, int index, int bufferlength)
        {
            //	TDebug.Log("SplitPackage:bufferlength="+bufferlength);

            byte[] lastLeftByte = lastLeftByteCombat;
            byte[] currentByte = null;

            if (lastLeftByte != null && lastLeftByte.Length > 0)
            {
                currentByte = new byte[lastLeftByte.Length + bufferlength];

                Buffer.BlockCopy(lastLeftByte, 0, currentByte, 0, lastLeftByte.Length);
                Buffer.BlockCopy(bytes, index, currentByte, lastLeftByte.Length, bufferlength);
                SetLastByte(null);
            }
            else
            {
                currentByte = new byte[bufferlength];
                Buffer.BlockCopy(bytes, index, currentByte, 0, bufferlength);
            }

            int _position = 0;
            while (_position < currentByte.Length)
            {
                if (_position + HEAD_LENGTH > currentByte.Length)
                    break;
                //handle one package...
                BaseStructRev pp = null;
                _position = newPackage(out pp, _position, currentByte, currentByte.Length, pSock, 0x1);
                if (pp == null)
                {
                    break;
                }
                Enqueue(pp, pSock);
            }
            int left = currentByte.Length - _position;
            if (left > 0)
            {
                lastLeftByte = new byte[left];
                SetLastByte(lastLeftByte);
                Buffer.BlockCopy(currentByte, _position, lastLeftByte, 0, left);
            }
        }

        void SplitPackage(byte[] bytes, int index, int bufferlength,
            SocketBase pSocket, List<BaseStructRev> lstBs)
        {

            byte[] currentByte = new byte[bufferlength];
            Buffer.BlockCopy(bytes, index, currentByte, 0, bufferlength);

            int _position = 0;
            while (_position < currentByte.Length)
            {
                if (_position + HEAD_LENGTH > currentByte.Length)
                    break;
                //handle one package...
                BaseStructRev pp = null;
                _position = newPackage(out pp, _position, currentByte, currentByte.Length, pSocket, 1);
                if (pp == null)
                {
                    break;
                }
                Enqueue(pp, pSocket, lstBs);
            }
        }


        /// <summary>
        /// handle one package... 
        /// </summary>
        /// <param name="mStruct"></param>
        /// <param name="_position"></param>
        /// <param name="currentByte"></param>
        /// <param name="buffLen"></param>
        /// <param name="mSocket"></param>
        /// <param name="tag">是否需要rc4</param>
        /// <returns></returns>
        int newPackage(out BaseStructRev mStruct, int _position, byte[] currentByte, int buffLen, SocketBase mSocket, int tag)
        {
            byte[] head = new byte[HEAD_LENGTH];
            int headLengthIndex = _position + HEAD_LENGTH;

            Buffer.BlockCopy(currentByte, _position, head, 0, HEAD_LENGTH);

            int headInt = BitConverter.ToInt32(head, 0);

            ushort command = (ushort)((headInt >> 16) & 0xffff);
            ushort mcommand = (ushort)((command >> 8) & 0xff);

            ushort mSubcommand = (ushort)(command & 0xff);

            //BaseStructRev 
            mStruct = null;

            int length = (int)((headInt & 0xffff) - HEAD_LENGTH);
            if (mcommand == 0 && mSubcommand == 0)
            {
                // TDebug.LogError(MsgRaw_t.CurTimeHHMMSS() + " NET: newPackage M=" + mcommand + ":S=" + mSubcommand + ":L=" + length + ":buffLen=" + buffLen + ":mSocket=" + mSocket.mType);
                needSync = true;
                return buffLen; //丢弃当前报文...
            }

            if (length > 0)
            {
                if (headLengthIndex + length > buffLen)
                {
                    if (length > 1024)
                    {
                        //过大的报文？
                        //TDebug.LogError(MsgRaw_t.CurTimeHHMMSS() + " NET: newPackage M=" + mcommand + ":S=" + mSubcommand + ":L=" + length + ":buffLen=" + buffLen);
                        if ((length > 1024 * 12))
                        {
                            needSync = true;
                            return buffLen; //丢弃当前报文...
                        }
                    }

                    return _position;
                }

                mStruct = BaseStructRevPool.SafeCreate(mcommand, mSubcommand, mSocket);

                byte[] data = new byte[length];
                Buffer.BlockCopy(currentByte, headLengthIndex, data, 0, length);

                //解密+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                //if (tag == 1 && mcommand != (int)EMainProtcol.PUBLIC_LOGIN_MAIN)
                //{
                //    data = RC4Crypt.RC4.DecryptEx(data);
                //}
                //解密+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                mStruct.data = data;

                //if (RemoteEvent.mEventArray.ContainsKey(mcommand))
                //{
                //    lock (((ICollection)mQueue).SyncRoot)
                //    {
                //        mQueue.Enqueue(mStruct);
                //    }
                //}
                //else
                //{
                //    TDebug.Log("can not find cmd=" + mcommand + "subcmd=" + mSubcommand);
                //}
                //Enqueue(mStruct);
                //return mStruct;


                _position = headLengthIndex + length;
            }
            else
            {
                if (length == 0)
                {
                    mStruct = BaseStructRevPool.SafeCreate(mcommand, mSubcommand, mSocket);
                    //lock (((ICollection)mQueue).SyncRoot)
                    //{
                    //    mQueue.Enqueue();
                    //}
                }
                _position = headLengthIndex;
            }
            return _position;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="srcData"></param>
        /// <param name="Length"></param>
        /// <param name="buffLen"></param>
        /// <returns></returns>
        unsafe byte[] unc_data(byte[] srcData, int Length, ref int buffLen)
        {
            ulong estimateLen = 0;
            byte[] currentByte = null;
            ulong srcLen = (ulong)Length;

            for (int ii = 1; ii < 8; ++ii)
            {
                estimateLen = ((srcLen * 8) << ii) + 256;
                currentByte = new byte[estimateLen];

                fixed (byte* pDstValue = currentByte, pSrcValue = srcData)
                {
                    int res2 = RakNetWrapper.UnCompress(pDstValue, &estimateLen, pSrcValue, srcLen);
                    //buffLen = _uncompress(pStruct.data, pStruct.data.Length, currentByte, estimateLen);
                    if (res2 == 0)
                    {
                        buffLen = (int)estimateLen;
                        return currentByte;
                    }
                    else
                    {
                        //TDebug.LogWarning(string.Format("unc_data :Length={0};raw len={1}", estimateLen, srcLen));
                    }
                }
            }
            return null;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="pStruct"></param>
        void Enqueue(BaseStructRev pStruct, SocketBase mSocket, List<BaseStructRev> lstBs = null)
        {
            //NetManager.GetInstance().OnNotifyRemoteEvent(mRev, mRev.mCommand, mRev.mSubCommand);
            //compress package？
            if (pStruct.mCommand == 0x39 && pStruct.mSubCommand == 0x00)
            {
#if ENABLE_PROFILER
                //                TDebug.Log("uncompress:" + buffLen + ",pre=" + pStruct.data.Length);
                //                int nPackage = 1;
#endif
                int buffLen = 0;
                byte[] currentByte = unc_data(pStruct.data, pStruct.data.Length, ref buffLen);
                int _position = 0;
                while (_position < buffLen)
                {
                    if (_position + HEAD_LENGTH > buffLen)
                        break;
                    //handle one package...
                    BaseStructRev pp = null;
                    _position = newPackage(out pp, _position, currentByte, buffLen, mSocket, 0);
                    if (pp == null)
                    {
                        break;
                    }
                    if (lstBs != null)
                    {
                        lstBs.Add(pp);
                    }
                    else
                    {
                        EnqueueImp(pp);
                    }
#if ENABLE_PROFILER
                    //TDebug.Log("newPackage:" + nPackage++ + ",cmd=" + pp.mCommand + ",sub=" + pp.mSubCommand);
#endif
                }
                //int left = buffLen - _position;
                //if (left > 0)
                //{
                //}
            }
            else
            {
                if (lstBs != null)
                {
                    lstBs.Add(pStruct);
                }
                else
                {
                    EnqueueImp(pStruct);
                }
            }
        }
#endif

        /// <summary>
        /// 
        /// </summary>
        /// <param name="pStruct"></param>
        void EnqueueImp(BaseStructRev pStruct)
        {
            lock (((ICollection)mQueue).SyncRoot)
            {
                mQueue.Add(pStruct);
            }
        }
    }


    //
    //实现一个Rak的tcp agent，当udp无法链接时，换做同端口的tcp进行链接尝试
    //报文结构： 长度2B+1B maincmd +1B subcmd; maincmd=39,压缩 50；subcmd=0-4保留；其他同RAK前缀命令（135-）
    // 相当于在现有udp协议上，增加3B
    class RakNetPeerTCP : RakNetPeer
    {
        TcpSession _session = new TcpSession();

        static int s__handle = 1;
        RakNetPeerTCP()
        {
            _handle = s__handle++;
        }

        float m_ConnectTimeout = 0;

        public static unsafe RakNetPeerTCP Create()
        {
            RakNetPeerTCP peer = new RakNetPeerTCP();
            if (peer._handle >= 0)
                return peer;
            return null;
        }

        //self imp...
        public override StartupResult Startup(string ip, ushort port, uint maxConnections)
        {
            if (!_started)
            {
                _host_ip = ip;
                _host_port = port;
                _max_connections = maxConnections;
                con_priority = 10;
                _started = true;
                //             return result;
            }
            return StartupResult.RAKNET_STARTED;
        }

        public override int GetAveragePing(int index)
        {
            return 0;
        }

        public override ConnectionState GetConnectionState(int index)
        {
            if (_session.m_Socket != null)
            {
                if (_session.m_Socket.m_Status == SOCKET_STATUS.CONNECTED)
                {
                    return ConnectionState.IS_CONNECTED;
                }
                else if (_session.m_Socket.m_Status == SOCKET_STATUS.CONNECTING)
                {
                    return ConnectionState.IS_CONNECTING;
                }
                else if (_session.m_Socket.m_Status == SOCKET_STATUS.CLOSING)
                {
                    return ConnectionState.IS_DISCONNECTING;
                }
            }
            return ConnectionState.IS_NOT_CONNECTED;
        }

        public override void CloseConnection(int index = 0, bool sendDisconnectionNotification = false, byte orderingChannel = 0, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY)
        {
            if (_session.m_Socket != null)
            {
                _session.m_Socket.Close();
            }
        }

        //
        public override ConnectionAttemptResult Connect(string host, ushort port, uint sendConnectionAttemptCount = 3u, uint timeBetweenSendConnectionAttemptsMS = 1000u, uint timeoutTime = 10000u)
        {
            int res = _session.doConnect(host, port, mgr);

            if (res == 0)
            {
                return ConnectionAttemptResult.UNKNOWN;
            }
            m_ConnectTimeout = 4f;  //链接超时控制

            return ConnectionAttemptResult.CONNECTION_ATTEMPT_STARTED;
        }

        /// <summary>
        /// 处理网络链接或者报文...
        /// </summary>
        public override unsafe void ReceiveCycle2()
        {
            //增加超时控制...
            if (m_ConnectTimeout > 0)
            {
                m_ConnectTimeout -= UnityEngine.Time.deltaTime;
                if (m_ConnectTimeout <= 0)
                {
                    if (_session.m_Socket != null)
                    {
                        _session.m_Socket.Close();
                        if (evt_OnState != null)
                        {
                            evt_OnState(this, 3,(int)RakNetPeer.E_CONN_NOTIFY_REASON.eReason_ConnectAttemptFailed);
                        }
                    }
                }
            }
            _session.pickMsgQueue();

            int nPvpCmd = 0;
            for (int i = 0; i < _session.mProcessList.Count; i++)
            {
                BaseStructRev mRev = _session.mProcessList[i];
                //_session.mProcessList[i] = null;
                //if(mRev)

                //if (mRev.mCommand != (int)EMainProtcol.CONNECT_MAIN_COMMAND)
                //{
                //    BNeedReconnect = false;
                //}
                //战斗服收到消息默认网络正常
                if (mRev.mCommand == (int)EMainProtcol.PVP_MAIN_COMMAND)
                {
                    nPvpCmd++;
                    if (mCB != null)
                        mCB(this, (int)mRev.mSubCommand, mRev.data, mRev.data.Length);
                }
                else
                if (mRev.mCommand == (int)EMainProtcol.CONNECT_MAIN_COMMAND)
                {
                    if (mRev.mSubCommand == (short)EConnectSubProtcol.SERVER_CONNECTED)
                    {
                        m_ConnectTimeout = 0;

                        //mCB = null;
                        SocketBase mSocket = (SocketBase)mRev.mSocket;
                        if (mSocket.conn != null)
                        {
                            activeConn = mSocket.conn;
                            //UnityEngine.Debug.Log("Rak:ProcessPacket:pkt->guid" + pkt->guid + ";control_cmd=" + pkt->control_cmd);
                            _connectionList.Add(activeConn);
                        }
                        // : (int)E_CONN_NOTIFY_REASON.eReason_AlreadyConnected
                        if (evt_OnState != null)
                        {
                            evt_OnState(this, 1, (int)0);//(mRev.mSocket.conn
                        }
                    }
                    else if (mRev.mSubCommand == (short)EConnectSubProtcol.SERVER_DISCONNECTED)
                    {
                        m_ConnectTimeout = 0;

                        if (evt_OnState != null)//mRev.mSocket!=null? mRev.mSocket.conn:nul
                            evt_OnState(this,1,true ? (int)E_CONN_NOTIFY_REASON.eReason_ClosedByRemote : (int)E_CONN_NOTIFY_REASON.eReason_ClosedByOurs);

                        activeConn = null;
                    }
                    else if (mRev.mSubCommand == (short)EConnectSubProtcol.SERVER_TIMEOUT)
                    {
                        m_ConnectTimeout = 0;
                        if (evt_OnState != null)
                        {//_session.m_Socket!=null?_session.m_Socket.conn:null
                            evt_OnState(this,3, (int)RakNetPeer.E_CONN_NOTIFY_REASON.eReason_ConnectAttemptFailed);
                        }
                        //if (evt_OnClosed != null)
                        //    evt_OnClosed(mRev.mSocket.conn, false ? (int)E_CONN_NOTIFY_REASON.eReason_ClosedByRemote : (int)E_CONN_NOTIFY_REASON.eReason_ClosedByOurs);
                        activeConn = null;
                    }
                    else if (mRev.mSubCommand == (short)EConnectSubProtcol.SERVER_DISPOSE)
                    {
                        m_ConnectTimeout = 0;
                        //todo:
                        activeConn = null;
                    }
                }

                //
                //BaseStructRevPool.SafeRecycle(mRev);
#if ENABLE_PROFILER
                //TDebug.Log("收到包=" + mRev.mCommand + ":" + mRev.mSubCommand);
#endif
                //RaiseEvent(RemoteEvent.mEventArray[mRev.mCommand], mRev.mSocket, mRev);
                //NetManager.GetInstance().OnNotifyRemoteEvent(mRev, mRev.mCommand, mRev.mSubCommand);
            }

            //if (nPvpCmd > 0)
            //{
            //  //  UILogicManager.GetInstance().RaiseUIEvent(UILogicEventDefine.EUILogicEvent.UILogicEvent_RevCombatHeart, null);
            //  //  RemoteManager.GetInstance().BWaitCHeart = false;
            //}
            _session.mProcessList.Clear();

        }
        //TCP: 大厅使用
        public byte m_subCmd = 0;
        //需要构造一个报文
        public override uint Send(byte[] data, int length, int index = 0, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY, PacketReliability reliability = PacketReliability.RELIABLE_ORDERED, sbyte orderingChannel = 0, bool broadcast = false, int forceReceiptNumber = 0)
         {
            if (_session.m_Socket != null)
            {
#if USE_FFF
                int head_len = 3 + (m_subCmd > 0 ? 1 : 0);
                int ll = length + head_len;
                byte[] buffer = new byte[ll];
                ByteBuffer bb = ByteBuffer.SBuffer(buffer);
                bb.PutUshort(0, (ushort)ll);
                buffer[2] = (byte)EMainProtcol.PVP_MAIN_COMMAND;
                if (m_subCmd>0)
                    buffer[3] = (byte)m_subCmd;
                //                buffer[3] = (byte)EConnectSubProtcol.SERVER_DATA;
                Buffer.BlockCopy(data, 0, buffer, head_len, length);
                int offset = 0;
#else
                //直接发送的报文，前面为134 CMD，拿掉;
                //bugfix: 不能使用shared buffer 发送到线程
                byte[] buffer = new byte[length];
                Buffer.BlockCopy(data, 0, buffer, 0, length);   //数据可能来自于shaderdbuff
                int offset = 1;
                int ll = length - offset;
#endif
                bool res = _session.m_Socket.Send(buffer, offset, ll);
                return res ? 1u : 0u;
            }
            return 0;
        }

        public override void Shutdown(int blockDuration = 0, byte orderingChannel = 0, PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY)
        {
            if (_started)
            {
                m_ConnectTimeout = 0;
                if (_session.m_Socket != null)
                {
                    _session.m_Socket.Shutdown();
                    _session.m_Socket = null;
                }
                _started = false;
            }
        }

        public override void Release()
        {
            if (_session.m_Socket != null)
            {
                _session.m_Socket.Dispose();
                _session.m_Socket = null;
            }
            m_ConnectTimeout = 0;
            _handle = -1;
        }
    }
}
