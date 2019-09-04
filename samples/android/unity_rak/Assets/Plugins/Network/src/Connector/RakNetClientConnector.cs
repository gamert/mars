
#define USE_FFF

using System;
using UnityEngine;
using RakNet;

using Comm.EventBus;


namespace Comm.Network
{
    public enum kPeerOpenTag:byte
    {
        UDP = 0x1,
        TCP = 0x2,

        ALL = 0xFF,
    }

    //1. 初次链接，选择UDP+TCP(延迟1s)，并行启动Connect；谁快用谁；
    //2. 在Loading start和end时，分别触发一次网络Connect优化检查，如果当前为tcp，且udp可用，则切换到udp模式；
    public class CComplexConnect
    {
        public kPeerOpenTag _openTag = kPeerOpenTag.ALL;   //
        public RakNetPeer _tcp;
        public RakNetPeer _udp;

        float tcp_delay;
        string ip;
        int port;
        OnPeerState OnState;
        OnPeerMsg OnMsg;

        public void OnPeerConnected(RakNetPeer _peer)
        {
            if(_udp == _peer && tcp_delay>0)
            {
                tcp_delay = 0;//可以关闭tcp连接了...
            }
        }

        //处理报文+延迟链接tcp
        public void Update()
        {
            //
            if(tcp_delay > 0)
            {
                tcp_delay -= Time.deltaTime;
                if(tcp_delay <= 0)
                {
                    EConnectorState cs = GetPeerState(_udp);
                    if (cs != EConnectorState.E_CON_ST_CONNECTED)
                    {
                        if (_tcp != null)
                        {
                            _tcp.evt_OnState = OnState;
                            _tcp.mCB = OnMsg;
                            ConnectionAttemptResult res2 = _tcp.Connect(ip, (ushort)port);
                        }
                    }
                }
            }

            if (_udp != null)
                _udp.ReceiveCycle2();
            if (_tcp != null)
                _tcp.ReceiveCycle2();
        }
        public bool IsOpenUse(kPeerOpenTag modeTag) { return (_openTag & modeTag) != 0; }

        public void InitPeers(kPeerOpenTag mode)
        {
            _openTag = mode;
            ReInitPeers();
        }

        //初始化2个链接
        public void ReInitPeers()
        { 
            _ShutdownPeers();
            if (IsOpenUse(kPeerOpenTag.UDP))
            {
                _udp = RakNetPeerUDP.Create();
                _InitPeer(_udp);
            }

            if (IsOpenUse(kPeerOpenTag.TCP))
            {
                _tcp = RakNetPeerTCP.Create();
                _InitPeer(_tcp);//, _OnState, _OnMsg
            }
            else
            {
                _ShutdownPeer(ref _tcp);
            }
        }

        //, OnPeerState _OnState, OnPeerMsg _OnMsg
        void _InitPeer(RakNetPeer _peer)
        {
            if (_peer != null && _peer.Startup("0.0.0.0", 0, 1) == StartupResult.RAKNET_STARTED)
            {
                //_peer.evt_OnConnected += _OnPeerConnected;
                //_peer.evt_OnClosed += _OnPeerClosed;
                //_peer.evt_OnError += _OnPeerError;
            }
        }
        public static void _ShutdownPeer(ref RakNetPeer _peer)
        {
            if (_peer != null)
            {
                _peer.Shutdown();
                _peer.Release();
                _peer = null;
            }
        }

        public void _ShutdownPeers()
        {
            _ShutdownPeer(ref _udp);
            _ShutdownPeer(ref _tcp);
        }

        public static EConnectorState GetPeerState(RakNetPeer _peer)
        {
            if (_peer != null)
            {
                ConnectionState st = _peer.GetConnectionState(0);
                if (st == ConnectionState.IS_CONNECTED)
                    return EConnectorState.E_CON_ST_CONNECTED;
                else if (st == ConnectionState.IS_PENDING
                    || st == ConnectionState.IS_CONNECTING)
                    return EConnectorState.E_CON_ST_CONNECTING;
                else if (st == ConnectionState.IS_DISCONNECTING
                    || st == ConnectionState.IS_SILENTLY_DISCONNECTING)
                    return EConnectorState.E_CON_ST_CLOSING;
                else if (st == ConnectionState.IS_NOT_CONNECTED
                    || st == ConnectionState.IS_DISCONNECTED)
                    return EConnectorState.E_CON_ST_CLOSED;
            }
            return EConnectorState.E_CON_ST_CLOSED;
        }
        //_mode = 0
        public static bool CanReplace(RakNetPeer _old, RakNetPeer _new,int mode)
        {
            if (_old == null || (_old.con_priority < _new.con_priority && mode == 1))
                return true;
            EConnectorState cs = GetPeerState(_old);
            return cs != EConnectorState.E_CON_ST_CONNECTED;
        }

        //如果一个peer错误，另一个是否等待？
        public bool OnPeerErrorWait(RakNetPeer _peer, int reason)
        {
            if(_peer == _tcp )
            {
                EConnectorState cs = GetPeerState(_udp);
                if (cs == EConnectorState.E_CON_ST_CONNECTING || cs == EConnectorState.E_CON_ST_CONNECTED)
                    return true;
            }
            else if (_peer == _udp)
            {
                EConnectorState cs = GetPeerState(_tcp);
                if (cs == EConnectorState.E_CON_ST_CONNECTING || cs == EConnectorState.E_CON_ST_CONNECTED)
                    return true;
            }
            return false;
        }
        //执行重连... tag = kPeerOpenTag
        public ConnectionAttemptResult Connect(string _ip, int _port, int tag, OnPeerState _OnState, OnPeerMsg _OnMsg)
        {
            bool bDelay = false;
            ConnectionAttemptResult res = ConnectionAttemptResult.UNKNOWN;
            if (_udp != null && (tag&0x1)!=0)
            {
                _udp.evt_OnState = _OnState;
                _udp.mCB = _OnMsg;
                res = _udp.Connect(_ip, (ushort)_port);
                bDelay = true;
            }

            //补充初始化...
            if ((tag & 0x2) != 0)
            {
                if(_tcp == null)
                {
                    _tcp = RakNetPeerTCP.Create();
                    _InitPeer(_tcp);//, _OnState, _OnMsg
                }
                tcp_delay = bDelay ? 2.5f : 0.05f;
                ip = _ip;
                port = _port;
                OnState = _OnState;
                OnMsg = _OnMsg;
            }
            return res;
        }
    }
    /// <summary>
    /// 战斗服链接...(TCP/UDP) ：当2次失败时，自动切换到tcp连接。
    /// </summary>
    public class RakNetClientConnector : BaseConnector
    {
        protected const byte kCustomUserMsg = 134;
        protected const byte kCustomPacketControlCode = 135;
        protected const byte kCustomUncompressedPacketControlCode = 136;
        protected const byte kCustomFrameSyncChannelCode = 150;
        protected const byte kCustomFrameSyncSectionDataCode = 151;
        protected const float kAuthTimeout = 10f;    //如果为被授权，需要重连    

        //
        protected RakNetPeer m_peer = null;
        private int _connectAttemptTimesPassed = 0;     //连接尝试次数
        private const int kConnectAttemptTimes = 4;     //总尝试次数
        private float _tryReconnectWaitTicks = 0f;      //重连等待时间
        private float _wait_auth_ticks = 0f;
        float bimm_udp = 0f;

        //bool _bUdp = true;      //当前使用的协议策略
        int _mode = 0;          //

        //统计连接失败信息
        int _connectFails = 0;  //单次连接成功前的失败次数

        private float _CheckTicks = 0f;                  //

        private const float kReconnectInterval = 4.5f;    //[自动重连]间隔，0616fix: 将4s改为10s
        //zzModi:
        static private byte[] _decompressBuf = new byte[64*1024];
        static private byte[] _compressBuf = _decompressBuf;


        CComplexConnect m_ComplexConnect = new CComplexConnect();
        public RakNetPeer peer
        {
            get { return m_peer; }
        }

        protected enum EAuthState
        {
            E_AUTH_DENIED = 0,
            E_AUTH_WAIT_INTERNAL_CONNECTED,
            E_AUTH_CHECK,
            E_AUTH_GRANTED
        }
#if UNITY_EDITOR
        protected EAuthState _authSt = EAuthState.E_AUTH_GRANTED;   //默认可以发送
#else
        protected EAuthState _authSt = EAuthState.E_AUTH_DENIED;
#endif

        //public override bool autoReconnect
        //{
        //    get
        //    {
        //        return base.autoReconnect;
        //    }
        //    set
        //    {
        //        base.autoReconnect = value;
        //        //if (value == false)
        //        //    _tryReconnect = false;
        //    }
        //}

        public override int pingTime
        {
            get
            {
                return state == EConnectorState.E_CON_ST_CONNECTED ? m_peer.GetAveragePing(0) : 1000;
            }
        }

        //取得实时Peer状态:
        EConnectorState GetPeerState()
        {
            return CComplexConnect.GetPeerState(m_peer);
        }

        public RakNetClientConnector(int tag)
        {
            m_ComplexConnect.InitPeers((kPeerOpenTag)tag);
            //InitPeer(true);
            state = EConnectorState.E_CON_ST_CLOSED;
        }

        //必要的链接...
        public override void SetMode(int mode)
        {
            this._mode = mode;
            if(this._mode == 1 && _addr!=null)
            {
                ConnectUDP(false);
            }
        }
        //fix: isOfflineMessage : cmd=18 可能关闭了tcp,同时又
        void ConnectUDP(bool Force)
        {
            if ((this.m_peer != m_ComplexConnect._udp) || Force)
            {
                //doConnect(true);
                ConnectionAttemptResult res = m_ComplexConnect.Connect(_addr.ip, (ushort)_addr.port, 0x1, this._OnPeerState, this._internal_onPacketReceived);
                Log(string.Format("RNC::ConnectUDP:res={0};_auto_reconnect={1};_wait_auth_ticks={2}", res, _auto_reconnect, _wait_auth_ticks));
            }
            else
            {
                CComplexConnect._ShutdownPeer(ref m_ComplexConnect._tcp);
            }
        }

        //void InitPeer(bool bUdp = true)
        //{
        //    if(_peer!=null)
        //    {
        //        //Close("InitPeer");
        //        Shutdown("InitPeer");
        //    }
        //   // bUdp = false;
        //    if (bUdp)
        //        _peer = RakNetPeerUDP.Create();
        //    else
        //        _peer = RakNetPeerTCP.Create();
        //    _bUdp = bUdp;
        //    if (_peer != null && _peer.Startup("0.0.0.0", 0, 1) == StartupResult.RAKNET_STARTED)
        //    {
        //        //_peer.evt_OnConnected += _OnPeerConnected;
        //        //_peer.evt_OnClosed += _OnPeerClosed;
        //        //_peer.evt_OnError += _OnPeerError;
        //        _peer.mCB = _internal_onPacketReceived;
        //    }
        //    else
        //    {
        //        _peer = null;
        //    }
        //}


        //如果派生类改写，那么执行新的逻辑... 
        protected virtual void OnInternalConnected()
        {
            _authSt = EAuthState.E_AUTH_GRANTED;
            state = EConnectorState.E_CON_ST_CONNECTED;
            Log("RNC::OnInternalConnected");  
            InvokeEvent_OnConnected();
        }
        //派生类改写，
        protected virtual void OnInternalClosed()
        {
            Log("RNC::OnInternalClosed");
            InvokeEvent_OnClosed();
        }

        //如果同时都链接上？
        void _OnPeerState(RakNetPeer _peer,int state, int reason)
        {
            switch(state)
            {
                case 1:
                    _OnPeerConnected(_peer,reason);
                    break;
                case 2:
                    _OnPeerClosed(_peer, reason);
                    break;
                case 3:
                    _OnPeerError(_peer, reason);
                    break;
            }
        }

        //等待c通知>
        private void _OnPeerConnected(RakNetPeer _peer, int reason)
        {
            //如果2个同时链接？
            if(_peer!= m_peer)
            {
                bool bb = CComplexConnect.CanReplace(m_peer, _peer, _mode);
                if (!bb)
                {
                    _peer.CloseConnection();
                    return;
                }
                //如果udp连接？
                m_ComplexConnect.OnPeerConnected(_peer);
                //关闭前连接..
                if (m_peer!=null)
                    m_peer.CloseConnection();
                m_peer = _peer;
            }

            RakNetWrapper.Connection conn = _peer.activeConn;
            Log(string.Format("RNC::_OnPeerState:{0};reason={1};state={2};connectFails={3}", _peer.Dump(), reason, state, _connectFails));

            _tryReconnectWaitTicks = 0;
            //_connectAttemptTimesPassed = 0;
            //_auto_reconnect = false;
            _authSt = EAuthState.E_AUTH_CHECK;
            _wait_auth_ticks = kAuthTimeout;
            _connectFails = 0;
            OnInternalConnected();
        }

        //如果连接失败？
        //@conn == null
        private void _OnPeerError(RakNetPeer _peer, int reason)
        {
            RakNetPeer.E_CONN_NOTIFY_REASON res = (RakNetPeer.E_CONN_NOTIFY_REASON)reason;
            bool bWait = m_ComplexConnect.OnPeerErrorWait(_peer, reason);

            Log(string.Format("RNC::_OnPeerError:{0},reason={1},_auto_reconnect={2};_connectAttemptTimesPassed={3};bWait={4}", _peer.Dump(), res, _auto_reconnect,_connectAttemptTimesPassed, bWait));
            //如果本错误被Connector处理掉，那么就先返回...
            if (bWait)
            {
                if (reason == (int)RakNetPeer.E_CONN_NOTIFY_REASON.eReason_AlreadyConnected)
                {
                    if (_peer != null)
                        _peer.CloseConnection();
                    bimm_udp = 0.5f;
                }
                return;
            }

            state = EConnectorState.E_CON_ST_CLOSED;
            if ( reason == (int)RakNetPeer.E_CONN_NOTIFY_REASON.eReason_AlreadyConnected)
            {
                Log("RNC::Close:state=" + state+";reason="+ res);
                if(_peer!=null)
                    _peer.CloseConnection();
                OnInternalClosed();
                _auto_reconnect_imm = 0.5f;// Reconnect();
            }
            else if (reason == (int)RakNetPeer.E_CONN_NOTIFY_REASON.eReason_ConnectAttemptFailed)
            {
                _connectFails++;
                if (_auto_reconnect && _connectAttemptTimesPassed < kConnectAttemptTimes)
                {
                    //切换到tcp...
                    if(_connectAttemptTimesPassed == 1 && m_ComplexConnect.IsOpenUse(kPeerOpenTag.TCP))
                    {
                        m_ComplexConnect.ReInitPeers();   //有机会再次切回来?
                        _connectAttemptTimesPassed = 1; //fix: 保持失败次数..
                    }
                    _tryReconnectWaitTicks = (_connectAttemptTimesPassed == 0) ? 0.1f : (_connectAttemptTimesPassed+1);    //首次失败后，立即重连；
                }
                else
                {
                    if(_auto_reconnect)
                    {
                        //切回upd
                        choose_net_mode("_OnPeerError[ConnectAttemptFailed]:");
                        //弹窗提示玩家: 连接失败，是否进行网络诊断？
                        OnConnectFail();
                    }
                    OnInternalClosed();
                }
                _RenewImp(_peer,true, "_OnPeerError::eReason_ConnectAttemptFailed");
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="conn"></param>
        /// <param name="reason"></param>
        private void _OnPeerClosed(RakNetPeer _peer, int reason)
        {
            bool bMain = _peer == this.m_peer;
            RakNetPeer.E_CONN_NOTIFY_REASON r = (RakNetPeer.E_CONN_NOTIFY_REASON)reason;
            Log(string.Format("RNC::_OnPeerClosed:{0},reason={1},{2}", _peer.Dump(), r, bMain));
            //主peer关闭，
            if (bMain)
            {
                state = EConnectorState.E_CON_ST_CLOSED;
                _tryReconnectWaitTicks = 0;
                _authSt = EAuthState.E_AUTH_DENIED;
                _wait_auth_ticks = 0f;

                OnInternalClosed();
                if (r == RakNetPeer.E_CONN_NOTIFY_REASON.eReason_ClosedByRemote)
                {
                    MEObjDeliver e = TPoolClass<MEObjDeliver>.AutoCreate();
                    e.opcode = (int)EObjDeliverOPCode.E_OP_CONN_CLOSE_BY_REMOTE;
                    Mercury.instance.Broadcast(EventTokenTable.et_game_framework, this, e);
                }
            }
            else
            {
                //无关peer关闭...

            }
        }



        NByteBuffer mMsgBuffer = new NByteBuffer();
        // public delegate void OnPacketReceived(ulong guid, int cmd, byte[] data, int length);
        /// <summary>
        /// Raknet报文回调: 
        /// </summary>
        /// <param name="srcIP"></param>
        /// <param name="srcPort"></param>
        /// <param name="cmd"> RAKNET 尾端的命令字</param>
        /// <param name="data"></param>
        /// <param name="length"></param>
        //static object o = new int();
        private void _internal_onPacketReceived(RakNetPeer _peer, int cmd, byte[] data, int length)
        {
            AddStatisticOfReceivedPacket(data, length);

#if USE_FFF
            if (cmd == kCustomPacketControlCode
                || cmd == kCustomUncompressedPacketControlCode
                || cmd == kCustomFrameSyncChannelCode
                || cmd == kCustomFrameSyncSectionDataCode)
#else
            if (cmd == kCustomUserMsg)
#endif
            {
                //Profiler.BeginSample("_internal_onPacketReceived:decompressed");

                int len = length;
                byte[] decompressed_data = data;
#if USE_FFF
                if (cmd == kCustomUncompressedPacketControlCode
                    || cmd == kCustomFrameSyncChannelCode)
                {                    
                    decompressed_data = data;
                }
                else
                {
                    len = Utility.Gzip_DecompressData(data, out decompressed_data);
                }
                len = Utility.SparseArray_Decompress(decompressed_data, len, ref _decompressBuf);
                decompressed_data = _decompressBuf;
                //Profiler.EndSample();
                //第一个报文一定是Auth报文?               
                if (_authSt == EAuthState.E_AUTH_CHECK)
                {
                    HandleAuth(decompressed_data, len);
                    return;
                }
#endif

                //zz: fast set...没有粘包等错误
                mMsgBuffer.HookSet(decompressed_data,0, len);
                if (_authSt == EAuthState.E_AUTH_GRANTED)
                {
                    {
                        for (int ii = 0; ii < _pluginList.Count; ++ii)
                        {
                            //Profiler.BeginSample("_pluginList.Process");    //352B,0.09ms->20B-0.03ms
                            bool b = _pluginList[ii].Process(mMsgBuffer, cmd);//decompressed_data, len
                            //Profiler.EndSample();
                            if (b)
                            {
                                return;
                            }
                        }
                    }

                    //for普通的协议绑定处理...
                    if (!HookReceivedData(mMsgBuffer))
                    {
                        DataWrapper wrapper = new DataWrapper((byte)cmd, mMsgBuffer);
                        InvokeEvent_OnDataReceived(wrapper);
                    }
                }
            }
            else
            {
                mMsgBuffer.HookSet(data, 0, length);
                DataWrapper wrapper = new DataWrapper((byte)cmd, mMsgBuffer);
                InvokeEvent_OnDataReceived(wrapper);
            }
        }

        public override void Connect(string ip, int port, string origin = "", string protocol = "", int uid = 0, byte[] _token = null)
        {
            //zz1014: 自动开启重连>
            _auto_reconnect = true; //
            _connectAttemptTimesPassed = 0;
            //==
            _addr = new AddressInfo(ip, port, origin, protocol);
            _uid = (uint)uid;
            this.token = _token;

            doConnect();
        }

        //执行连接
        //fix: ipv6的地址保存后，在ipv4的地址下，会导致错误.
        ConnectionAttemptResult doConnect()
        {
            UpdatePeerState("RNC::doConnect[Pre]:");
            //重新登..
            if (state == EConnectorState.E_CON_ST_CONNECTED)
            {
                Close("doConnect: pre E_CON_ST_CONNECTED");
            }
            if ((state == EConnectorState.E_CON_ST_CLOSED || state == EConnectorState.E_CON_ST_CLOSING))
            {
                //bug: 如果前面udp连通过，那么将关闭tcp，从而导致在单纯udp连接下，无法成功...
                ConnectionAttemptResult res = m_ComplexConnect.Connect(_addr.ip, (ushort)_addr.port,0x3, this._OnPeerState, this._internal_onPacketReceived);
                if (res == ConnectionAttemptResult.CONNECTION_ATTEMPT_STARTED)
                {
                    _authSt = EAuthState.E_AUTH_WAIT_INTERNAL_CONNECTED;
                    _wait_auth_ticks = 0f;
                    _send_seq = 0;
                    state = EConnectorState.E_CON_ST_CONNECTING;
                }
                else if (res == ConnectionAttemptResult.CANNOT_RESOLVE_DOMAIN_NAME)
                {
                    //bug2010/1/17: 当动态切换ipv6网络，rak无法connect
                    //fix: 异常情况关闭..
                    //OnInternalClosed();
                    m_ComplexConnect.ReInitPeers();
                }
                Log(string.Format("RNC::doConnect:res={0};_auto_reconnect={1};_wait_auth_ticks={2},{3}", res ,_auto_reconnect, _wait_auth_ticks, _addr.ip));
                return res;
            }
            else
            {
                Log(string.Format("RNC::doConnect:state={0};_peer={1}", state, m_peer));
            }
            return ConnectionAttemptResult.UNKNOWN;
        }

        public override void NewConnect(string ip, int port, string origin = "", string protocol = "", int uid = 0, byte[] token = null)
        {
        }

        //
        public override void Update()
        {
            //检测超时等.
            base.Update();

            //检查连接状态ping等
            //if(state == EConnectorState.E_CON_ST_CONNECTED)
            //{
            //  
            //}
            m_ComplexConnect.Update();

            //接收报文
            //Profiler.BeginSample("_peer.ReceiveCycle2"); //
            if (m_peer != null)
            {
                _CheckTicks += Time.smoothDeltaTime;
                if(_CheckTicks > 2) //每2s同步一个
                {
                    _CheckTicks = 0;
                    _UpdatePeerStateImp(false,"RNC::Update:");
                }
            }

            //Profiler.EndSample();
            if(bimm_udp > 0)
            {
                bimm_udp -= Time.smoothDeltaTime;
                if(bimm_udp <=0)
                    ConnectUDP(true);
            }

            //设置了自动重连？
            if (_auto_reconnect_imm > 0)
            {
                _auto_reconnect_imm -= Time.smoothDeltaTime;
                if (_auto_reconnect_imm <= 0)
                    Reconnect("_auto_reconnect_imm = true");
            }
            else if (_auto_reconnect)
            {
                if (_tryReconnectWaitTicks > 0 && Time.smoothDeltaTime > 0)
                {
                    _tryReconnectWaitTicks -= Time.smoothDeltaTime;
                    if(_tryReconnectWaitTicks <= 0)
                    {
                        _connectAttemptTimesPassed++;   //总共连10次...
                        Reconnect("Update:_tryReconnectWaitTicks");
                        return;
                    }

                    //this.UpdatePeerState("RNC::_auto_reconnect:");
                    //EConnectorState ts = state;
                    //if (ts == EConnectorState.E_CON_ST_CLOSED || ts == EConnectorState.E_CON_ST_CLOSING)
                    //{
                    //    if (_tryReconnectWaitTicks > kReconnectInterval)    //每4s重连一次？
                    //    {

                    //    }
                    //}
                }
                //else
                //{
                //    Log("RNC::Update:state=" + ts);
                //}
                //}
            }

            //检查授权超时...
            if (_authSt == EAuthState.E_AUTH_CHECK && _wait_auth_ticks > 0 && Time.smoothDeltaTime > 0)//
            {
                _wait_auth_ticks -= Time.smoothDeltaTime;
                doAuthFailCheck();
            }
        }


        void doAuthFailCheck()
        {
            if (_wait_auth_ticks <= 0)
            {
                _authSt = EAuthState.E_AUTH_DENIED;
                //                    _CloseImp(string.Format("Update:_authSt = EAuthState.E_AUTH_DENIED;_auto_reconnect={0};_connectAttemptTimesPassed={1}", _auto_reconnect, _connectAttemptTimesPassed));
                _RenewImp(m_peer,true, string.Format("Update:_authSt = EAuthState.E_AUTH_DENIED;_auto_reconnect={0};_connectAttemptTimesPassed={1}", _auto_reconnect, _connectAttemptTimesPassed));  //

                _connectFails++;
                if (_auto_reconnect)
                {
                    if (_connectAttemptTimesPassed < kConnectAttemptTimes)
                    {
                        _tryReconnectWaitTicks = (_connectAttemptTimesPassed == 0) ? 0.1f : (_connectAttemptTimesPassed + 3);    //首次失败后，立即重连；
                    }
                    else
                    {
                        //如果是超时失败...提示用户...
                        OnConnectFail();
                    }
                }
            }
        }

        /// <summary>
        /// APP 关闭连接
        /// </summary>
        /// <param name="reason"></param>
        public override void Close(string reason)
        {
            _connectAttemptTimesPassed = 0; //如果外部关闭，那么，就清理
            if (_auto_reconnect)
            {
                _auto_reconnect = false;    //fix:: 如果连接超时，导致重连时，因为服务器战斗关闭==》app关闭==》online状态触发Close时，需要关闭自动重连。
            }
            //_tryReconnect = false;
            _tryReconnectWaitTicks = 0;

            //_CloseImp(reason);
            //如果一次战斗结束后...
            choose_net_mode(reason);
        }

        //
        void choose_net_mode(string reason)
        {
//            if (_bUdp == false)
            {
                Log("RNC::choose_net_mode:ReInitPeers: _bUdp->true;" + reason);
                m_ComplexConnect.ReInitPeers();
                UpdatePeerState("RNC::choose_net_mode:");
            }
        }

        //fix: 对于关闭来说，盲关更安全
        void _CloseImp(string reason)
        {
//            UpdatePeerState("RNC::CloseImp");
            Log(string.Format("RNC::CloseImp:state={0};_authSt={1};reason:{2}", state, _authSt,reason));
            //            if (state > EConnectorState.E_CON_ST_CLOSING)
            if (this.m_peer != null)
                m_peer.CloseConnection();
            UpdatePeerState("RNC::CloseImp2");
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="msg"></param>
        /// <param name="len"></param>
        /// <param name="immediateSent"></param>
        /// <param name="args"></param>
        //public override int SendMsg(object msg, int len = -1, bool immediateSent = true, Hashtable args = null)
        //{
        //    if (_authSt == EAuthState.E_AUTH_GRANTED && GetPeerState() == EConnectorState.E_CON_ST_CONNECTED)
        //    {
        //        return base.SendMsg(msg, len, immediateSent, args);
        //    }
        //    else
        //    {
        //        // drop
        //        AbstractSmartObj obj = msg is AbstractSmartObj ? (AbstractSmartObj)msg : null;
        //        if (obj != null)
        //            obj.Release();
        //        return 0;
        //    }
        //}

        /// <summary>
        /// 
        /// </summary>
        /// <param name="data">要发送的报文数据</param>
        /// <param name="len"></param>
        /// <param name="args"></param>
        /// <returns></returns>
        protected override bool Send(byte[] data, int len, int reliability = DEF_RELIABILITY)
        {
            if (m_peer != null)
            {
                //UnityEngine.Profiler.BeginSample("RAK:Send:Compress");
                byte rakcmd = data[0];
                Buffer.BlockCopy(data, 1, data, 0, len - 1);
                int usedSz = Utility.SparseArray_Compress(data, len - 1, ref _compressBuf);

                Buffer.BlockCopy(_compressBuf, 0, _compressBuf, 1, usedSz);
                _compressBuf[0] = rakcmd;

                len = usedSz + 1;
                data = _compressBuf;

                PacketPriority priority = PacketPriority.IMMEDIATE_PRIORITY;
                //PacketReliability reliability = PacketReliability.RELIABLE_ORDERED;
                //if (args != null)
                //{
                //    if (args.ContainsKey("packet_priority"))
                //        priority = (PacketPriority)args["packet_priority"];
                //    if (args.ContainsKey("packet_reliability"))
                //        reliability = (PacketReliability)args["packet_reliability"];
                //}
                //UnityEngine.Profiler.EndSample();

                //UnityEngine.Profiler.BeginSample("RAK:Send");
                m_peer.Send(data, len, 0, priority, (PacketReliability)reliability, 0, false, 0);
                //UnityEngine.Profiler.EndSample();
                //Dump(data, len);

                ++_send_seq;
                return true;
            }

            return false;
        }
        /*
        void Dump(byte[] data, int len)
        {
            string buf = "Dump "+len+":";
            for(int i = 0;i < len;++i)
            {
                string hexOutput = String.Format("{0:X} ", data[i]); buf += hexOutput;
            }
            Debug.LogError(buf);
        }
*/

        //
        public void UpdatePeerState(string reason)
        {
            _UpdatePeerStateImp(true,reason);
        }

        //
        void _UpdatePeerStateImp(bool bLog,string reason)
        {
            EConnectorState _state = GetPeerState();
            if (state != _state)
            {
                if(bLog)
                {
                    Log("RNC::UpdatePeerState:state=" + state + "==>_state=" + _state + ";reason=" + reason);
                }
                state = _state;
            }
        }



        public override bool Reconnect(string reason)
        {
            Log("RNC::Reconnect; reason:" + reason);
            if (_addr != null )
            {
                return doConnect() == ConnectionAttemptResult.CONNECTION_ATTEMPT_STARTED;
            }
            return false;
        }



        public override void Shutdown(string reason)
        {
            Log(string.Format("RNC::Shutdown:state={0};reason={1};_peer={2}", state, reason, m_peer));
            if (state > EConnectorState.E_CON_INVALID_PARAMETER)
            {
                m_ComplexConnect._ShutdownPeers();// _ShutdownPeer(ref m_peer);
                m_peer = null;

                //_tryReconnect = false;
                _connectAttemptTimesPassed = 0;
                _authSt = EAuthState.E_AUTH_DENIED;

               UpdatePeerState("RNC::Shutdown");
            }
        }

        virtual protected void HandleAuth(byte[] data, int len) { }

        //zzfix0930: Android和ios全部重建
        public void RenewConnector(bool force = false)
        {
            //this._auto_reconnect = false;   //外部renew ，自动关闭

            if (this.m_peer != null)
            {
//#if UNITY_IPHONE
                force = true;
                _RenewImp(m_peer,force, "RenewConnector");
//#else
//                _RenewImp(force, "RenewConnector");
//#endif
            }
        }

        void _RenewImp(RakNetPeer _peer, bool force,string reason)
        {
            Log(string.Format("_RenewImp:({0},{1}),_peer={2}", force, reason, _peer));
            if (_peer != null)
            {
                if(force)
                {
                    _peer.Shutdown();// ("MHRakNetInitializer::RenewConnector");
                    _peer.Renew();
                    if(_peer == m_peer)
                        UpdatePeerState("RNC::RenewConnector");
                }
                else
                {
                    if (_peer == m_peer)
                        _CloseImp("RNC::RenewConnector");
                }
            }
        }


    }
}