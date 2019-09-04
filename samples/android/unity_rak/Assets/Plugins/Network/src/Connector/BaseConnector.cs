using System;
using System.Collections.Generic;

using Comm.EventBus;
using RakNet;
using System.Text;
using Network.Protocol;

namespace Comm.Network
{

    internal enum EMainProtcol
    {
        //PUBLIC_LOGIN_MAIN = 0,          /// publish server 相关命令
        //LOGIN_MAIN_COMMAND = 1,          /// 游戏服务器主命令 EPlayerSubProtcol
        PVP_MAIN_COMMAND = 50,         /// 战斗相关命令 现在发到战斗服
        //ACTIVITY_MAIN_COMMAND = 18,         /// 活动公告协议
        //REQ_COMBAT_MAIN_COMMAND = 19,         /// 请求战斗的相关协议
        //RANK_MAIN_COMMAND = 24,         /// 排行榜主命令
        //NEAR_MAIN_COMMAND = 25,         /// 近场协议
        //SOCIAL_MAIN_COMMAND = 26,         /// 社交系统相关 ESocialSubProtcol
        //MAIL_GM_NOTIFY = 27,         /// 邮件、活动、GM通知等EMailNoticeSubProtcal
        //CHAT_MAIN_COMMAND = 28,         /// 聊天                                                        
        //ROOM_MAIN_COMMAND = 29,         /// 房间相关    
        //HONOR_MAIN_COMMAND = 31,         /// 荣誉相关
        //NEAR_FRIEND_COMMAND = 32,         /// 附近好友
        //BROADCAST_COMMAND = 33,         /// 广播(跑马灯)
        //PVPGROUP_MAIN_COMMAND = 34,         /// 2v2开黑
        //CORPS_MAIN_COMMAND = 35,         /// 战队
        //CLOUD_STORE_COMMAND = 37,         /// 云购
        //COMPRESS_COMMAND = 39,         /// 压缩

        CONNECT_MAIN_COMMAND = 127,        /// 客户端自定义的socket连接相关信息        

    }
    internal enum EConnectSubProtcol//主命令 127
    {
        SERVER_CONNECTED = 0,          /// 服务器连接成功
        SERVER_DISCONNECTED = 1,          /// 服务器断开连接  
        SERVER_TIMEOUT = 2,          /// 服务器连接超时
        SERVER_DISPOSE = 3,          /// 服务器连接释放

        SERVER_DATA = 4,
    }

    abstract public class BaseConnector
    {
        //static new Threa
        protected IPacketStatistic _pktStat = null;
        public IPacketStatistic pktStat
        {
            get { return _pktStat; }
            set { _pktStat = value; }
        }

        virtual public EConnectorState state
        {
            get;
            set;
        }

        //public static bool EnableTcp = false;

        public bool IsConnected
        {
            get { return state == EConnectorState.E_CON_ST_CONNECTED; }
        }

        public virtual void SetMode(int mode)
        {
        }

        public string mType
        {
            get { return string.Format("IP:{0};Port{1};state{2}", addr.ip, addr.port, state); }
        }

        //public ReconnectPolicy _reconnect = new ReconnectPolicy();   //管理重连


        protected AddressInfo _addr = null;
        public AddressInfo addr
        {
            get
            {
                return _addr;
            }
            set
            {
                _addr = value;
            }
        }

        virtual public int pingTime
        {
            get { return -1; }
        }

        public class DataWrapper : EventArgs
        {
            private byte _cmd = 0;
            public byte cmd
            {
                get { return _cmd; }
            }

            private NByteBuffer _data = null;
            public NByteBuffer data
            {
                get { return _data; }
            }

            public DataWrapper(NByteBuffer pp)
            {
                _data = pp;
            }

            public DataWrapper(byte cc, NByteBuffer pp)
            {
                _cmd = cc;
                _data = pp;
            }
        }

        protected event EventHandler evt_OnTimeout;
        protected event EventHandler evt_OnConnected;
        protected event EventHandler evt_OnClosed;
        protected event EventHandler<DataWrapper> evt_OnDataReceived;

        protected List<IMessagePlugin> _pluginList = new List<IMessagePlugin>();
        private byte[] _sharedSendBuffer = new byte[8192];
        protected int _send_seq = 0;

        protected byte[] _token = null;
        public byte[] token
        {
            get { return _token; }
            set
            {
                _token = value;
                if (_token != null)
                {
                    if (_token != null && _token.Length > 0)
                    {
                        if (_convertableToken == null || _convertableToken.Length != _token.Length)
                            _convertableToken = new sbyte[_token.Length];

                        for (int ii = 0; ii < _token.Length; ++ii)
                            _convertableToken[ii] = (sbyte)_token[ii];
                    }
                }
                else
                {
                    _convertableToken = null;
                }
            }
        }

        protected sbyte[] _convertableToken = null;
        public sbyte[] convertableToken
        {
            get { return _convertableToken; }
        }

        protected uint _uid = 0u;
        public uint uid
        {
            get { return _uid; }
            set { _uid = value; }
        }

        protected const int kMaxSentPerTick = 4;
        protected const int kMaxPacketsOnWire = 64;


        public const int DEF_RELIABILITY = 3;

        protected class SendContext : AbstractSmartObj
        {
            public object msg = null;
            public int len = 0;
            public int reliability = DEF_RELIABILITY;  //

            public override void OnRelease()
            {
                msg = null;
                len = 0;
                reliability = DEF_RELIABILITY;    //默认不可靠？  
                //if (args != null)
                //{
                //    args.Clear();
                //    args = null;
                //}
            }
        }

        protected Queue<SendContext> _sendQueue = new Queue<SendContext>();
        protected Queue<SendContext> _resendQueue = new Queue<SendContext>();
        private static SendContext _reservedIns = new SendContext();

        protected bool _auto_reconnect = false;
        protected float _auto_reconnect_imm = 0;     //控制立即重连.
        virtual public bool autoReconnect
        {
            get { return _auto_reconnect; }
            set { _auto_reconnect = value; }
        }

        public void AddCallback_OnConnected(EventHandler eh)
        {
            if (eh != null)
                evt_OnConnected += eh;
        }

        public void AddCallback_OnClosed(EventHandler eh)
        {
            if (eh != null)
                evt_OnClosed += eh;
        }

        public void AddCallback_OnTimeout(EventHandler eh)
        {
            if (eh != null)
                evt_OnTimeout += eh;
        }

        public void RemoveCallback_OnConnected(EventHandler eh)
        {
            if (eh != null)
                evt_OnConnected -= eh;
        }

        public void RemoveCallback_OnClosed(EventHandler eh)
        {
            if (eh != null)
                evt_OnClosed -= eh;
        }

        public void RemoveCallback_OnTimeout(EventHandler eh)
        {
            if (eh != null)
                evt_OnTimeout -= eh;
        }

        protected void NotifyEvents(int op)
        {
            Utility.NotifyEvents(this, EventTokenTable.et_connector, op);
        }

        protected void InvokeEvent_OnConnected()
        {
            //zz: 如果连接ok，那么关闭自动重连?
            _auto_reconnect = false;

            if (evt_OnConnected != null)
                evt_OnConnected(this, EventArgs.Empty);
        }

        protected void InvokeEvent_OnClosed()
        {
            if (evt_OnClosed != null)
                evt_OnClosed(this, EventArgs.Empty);
        }

        protected void InvokeEvent_OnTimeout()
        {
            if (evt_OnTimeout != null)
                evt_OnTimeout(this, EventArgs.Empty);
        }

        protected void InvokeEvent_OnDataReceived(DataWrapper wrapper)
        {
            if (evt_OnDataReceived != null)
                evt_OnDataReceived(this, wrapper);
        }

        virtual public void Shutdown(string reason) { }

        abstract public void Connect(string ip, int port, string origin = "", string protocol = "", int uid = 0, byte[] token = null);
        abstract public void NewConnect(string ip, int port, string origin = "", string protocol = "", int uid = 0, byte[] token = null);
        abstract public void Close(string reason);
        /// <summary>
        /// 
        /// </summary>
        /// <param name="reason"></param>
        /// <returns>false 失败；true成功启动</returns>
        abstract public bool Reconnect(string reason);

        //首次连接失败...
        virtual public void OnConnectFail()
        {
        }

        virtual public void Update()
        {
            if (IsConnected)
            {
                while (_resendQueue.Count > 0 && _sendQueue.Count < kMaxPacketsOnWire)
                {
                    SendContext ctx = _resendQueue.Dequeue();
                    _sendQueue.Enqueue(ctx);
                }

                if (_sendQueue.Count > 0)
                {
                    UnityEngine.Debug.LogWarning("Connector::Resend():count = " + _sendQueue.Count);
                    int amount = kMaxSentPerTick;
                    while (amount > 0 && _sendQueue.Count > 0)
                    {
                        SendContext ctx = _sendQueue.Peek();
                        byte[] data = null;
                        int usedSz = Serialize(ctx.msg, out data);

                        if (usedSz > 0 && Send(data, usedSz, ctx.reliability))
                        {
                            AddStatisticOfSentPacket(ctx.msg, usedSz);
                            ctx = _sendQueue.Dequeue();
                            ctx.Release();
                            --amount;
                        }
                        else
                        {
                            ctx = _sendQueue.Dequeue();
                            _resendQueue.Enqueue(ctx);
                        }
                    }
                }
            }
        }

     

        //默认总是发送成功...
        virtual public int SendMsg(object msg, int len = -1, bool immediateSent = true, int reliability = DEF_RELIABILITY)
        {
            EConnectorState _state = state;// EConnectorState.E_CON_ST_CONNECTED;    //state
            if (_state == EConnectorState.E_CON_ST_CONNECTED)
            {
                if (immediateSent == false)
                {
                    SendContext ctx = TPoolClass<SendContext>.AutoCreate(20);
                    ctx.msg = msg;
                    ctx.len = len;
                    ctx.reliability = reliability;

                    if (_sendQueue.Count < kMaxPacketsOnWire)
                    {
                        _sendQueue.Enqueue(ctx);
                    }
                    else
                    {
                        _resendQueue.Enqueue(ctx);
                    }
                }
                else
                {
                    byte[] data = null;
                    int usedSz = Serialize(msg, out data);
                    if (usedSz > 0 && Send(data, usedSz, reliability))
                    {
                        AddStatisticOfSentPacket(msg, usedSz);
                    }
                    else
                    {
                        SendContext ctx = TPoolClass<SendContext>.AutoCreate(20);
                        ctx.msg = msg;
                        ctx.len = len;
                        ctx.reliability = reliability;
                        _resendQueue.Enqueue(ctx);
                    }
                }
            }
            else
            {
                SendContext ctx = TPoolClass<SendContext>.AutoCreate(20);
                ctx.msg = msg;
                ctx.len = len;
                ctx.reliability = reliability;
                _resendQueue.Enqueue(ctx);
            }
            return len;
        }

        //
        public bool FindPlugin(IMessagePlugin plugin,bool bRemove = false)
        {
            for (int ii = 0; ii < _pluginList.Count; ++ii)
            {
                if (_pluginList[ii]  == plugin)
                {
                    if (bRemove)
                        _pluginList.RemoveAt(ii);
                     return true;
                }
            }
            return false;
        }

        public void AddPlugin(IMessagePlugin plugin)
        {
            if (plugin != null)
            {
                if (FindPlugin(plugin))
                {
#if DEBUG
                    BaseConnector.Log("AddPlugin exist: "+ plugin);
#endif
                    return;
                }

                plugin.conn = this;
                _pluginList.Add(plugin);
            }
        }
        public void AddPlugin<T>() where T : IMessagePlugin, new()
        {
            T plugin = FindPlugin<T>();
            if (plugin == null)
            {
                plugin = new T();
                plugin.conn = this;
                _pluginList.Add(plugin);
            }
        }

        public void RemovePlugin<T>() where T : IMessagePlugin
        {
            T plugin = FindPlugin<T>();
            if (plugin != null)
                _pluginList.Remove(plugin);
        }

        public T FindPlugin<T>() where T : IMessagePlugin
        {
            for (int ii = 0; ii < _pluginList.Count; ++ii)
            {
                if (_pluginList[ii] is T)
                    return (T)_pluginList[ii];
            }
            return default(T);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="msg"></param>
        /// <param name="data"></param>
        /// <returns></returns>
        protected int Serialize(object msg, out byte[] data)
        {
            int usedSz = 0;
            data = _sharedSendBuffer;
#if RELEASE_VERSION
            try
#endif
            {
                for (int ii = 0; ii < _pluginList.Count; ++ii)
                {
                    if (_pluginList[ii].ProcessBeforeSent(msg, ref _sharedSendBuffer, out usedSz))
                    {
                        return usedSz;
                    }
                }
            }
#if RELEASE_VERSION
            catch (Exception e)
            {
                UnityEngine.Debug.LogError(string.Format("BaseConnector::Serialize: Plugin::ProcessBeforeSent failed - {0}", e.Message));
            }
#endif

            return DefaultSerialize(msg, out data);
        }

        virtual protected int DefaultSerialize(object msg, out byte[] data)
        {
            data = _sharedSendBuffer;
            return 0;
        }

        abstract protected bool Send(byte[] data, int len, int reliability = DEF_RELIABILITY); //    Hashtable args   
        virtual protected void DropMsgCustom(object msg) { }

        private void DropMsg(object msg)
        {
            if (msg is AbstractSmartObj)
            {
                AbstractSmartObj obj = (AbstractSmartObj)msg;
                obj.Release();
            }
            else
            {
                DropMsgCustom(msg);
            }
        }

        virtual protected bool HookReceivedData(NByteBuffer msg)
        {
            AddStatisticOfReceivedPacket((object)msg.Data, msg.dataLen());
            return false;
        }

        virtual protected void AddStatisticOfSentPacket(object msg, int len)
        {
            if (_pktStat != null)
                _pktStat.AddStatisticOfSentPacket(msg, len);
        }

        virtual protected void AddStatisticOfReceivedPacket(object msg, int len)
        {
            if (_pktStat != null)
                _pktStat.AddStatisticOfReceivedPacket(msg, len);
        }

        virtual protected void Distribute(object packet) { }

        //
        //public static void DumpBuffer(byte[] bytes, int pos, int len)
        //{
        //    StringBuilder str = new StringBuilder(len * 3 + 32);
        //    for (int i = 0; i < len; i++)
        //    {
        //        str.AppendFormat("{0:X}", bytes[pos + i]);
        //        if (i != len - 1)
        //        {
        //            str.Append(",");
        //        }
        //    }
        //    Log("BC::DumpBuffer:bytes=" + str.ToString());
        //}\

        //如果重载...
        public delegate void fnLog(object message);
        public static fnLog netLog = null;
        public static void Log(object message)
        {
            if (netLog != null)
            {
                netLog(message);
                return;
            }

#if UNITY_EDITOR
            float time = 0;
            try
            {
                time = UnityEngine.Time.time;
            }
            catch(Exception e)
            {
            }
            UnityEngine.Debug.LogWarning(time + ": "+message.ToString());
#endif
            //_frameLog(message);
            //DebugHelper.
#if !UNITY_STANDALONE_OSX
            //RakNetWrapper.RkN_xLog(1, string.Format("[{0}|{1}]{2}", UnityEngine.Time.frameCount, UnityEngine.Time.time, message));
            RakNetWrapper.RkN_xLog(1, message.ToString());
#endif
        }

        //
        public static void NetEvent(int param,string reason)
        {
//            int res = 0;
//#if !UNITY_STANDALONE_OSX
//            res = RakNetWrapper.RkN_NetEvent(param);
//#endif
//            Log(string.Format("NetEvent[{0}]:{1},reason={2}", param, res, reason));
        }

        internal void RestartConnector()
        {
            throw new NotImplementedException();
        }
    }

    //public enum NetResult
    //{
    //    Success = 0,
    //    Error = 1,
    //    NetworkException = 2,
    //    Timeout = 3,
    //    CError = 4,
    //    PeerStopSession = 5,
    //    PeerCloseConnection = 6,
    //    ConnectFailed = 7,
    //    TokenSvrError,
    //}

    //public delegate uint tryReconnectDelegate(uint nCount, uint nMax);
    ////改进重连..
    //public class ReconnectPolicy
    //{
    //    private BaseConnector connector;

    //    private tryReconnectDelegate callback;

    //    private bool sessionStopped;

    //    private float reconnectTimeWait;

    //    private uint reconnectCount = 4u;

    //    private uint tryCount;

    //    private uint connectTimeout = 10u;

    //    public bool shouldReconnect;

    //    //for Init or Disconnect
    //    public void SetConnector(BaseConnector inConnector, tryReconnectDelegate inEvent, uint tryMax)
    //    {
    //        StopPolicy();
    //        connector = inConnector;
    //        callback = inEvent;
    //        reconnectCount = tryMax;
    //    }
    //    //
    //    public void StopPolicy()
    //    {
    //        sessionStopped = false;
    //        shouldReconnect = false;
    //        reconnectTimeWait = connectTimeout;
    //        tryCount = 0u;
    //    }

    //    //当重连次数不==0时，使用，否则使用立即重连...
    //    //
    //    public void StartPolicy(NetResult result, int timeWait)
    //    {
    //        switch (result)
    //        {
    //            case NetResult.Success:
    //                shouldReconnect = false;
    //                sessionStopped = false;
    //                return;
    //            case NetResult.Error:
    //                shouldReconnect = true;
    //                sessionStopped = true;
    //                reconnectTimeWait = (float)((tryCount != 0u) ? timeWait : 0);
    //                return;
    //            case NetResult.NetworkException:
    //                shouldReconnect = true;
    //                sessionStopped = false;
    //                reconnectTimeWait = (float)((tryCount != 0u) ? timeWait : 0);
    //                return;
    //            case NetResult.Timeout:
    //            case NetResult.CError:
    //            case NetResult.PeerStopSession:
    //            case NetResult.ConnectFailed:
    //            case NetResult.TokenSvrError:
    //                shouldReconnect = true;
    //                sessionStopped = true;
    //                reconnectTimeWait = (float)((tryCount != 0u) ? timeWait : 0);
    //                return;
    //            case NetResult.PeerCloseConnection:
    //                shouldReconnect = true;
    //                sessionStopped = true;
    //                reconnectTimeWait = (float)((tryCount != 0u) ? timeWait : 0);
    //                return;
    //        }
    //    }

    //    //for Update() or ForceReconnect
    //    public void UpdatePolicy(bool bForce)
    //    {
    //        if (connector != null && !connector.IsConnected)
    //        {
    //            if (bForce)
    //            {
    //                reconnectTimeWait = connectTimeout;
    //                tryCount = reconnectCount;
    //                if (sessionStopped)
    //                {
    //                    connector.RestartConnector();
    //                }
    //                else
    //                {
    //                    connector.RestartConnector();
    //                }
    //            }
    //            else
    //            {
    //                reconnectTimeWait -= Time.unscaledDeltaTime;    //The timeScale-independent time in seconds it took to complete the last frame
    //                if (reconnectTimeWait < 0f)
    //                {
    //                    tryCount += 1u;
    //                    reconnectTimeWait = connectTimeout;
    //                    uint num = tryCount;
    //                    if (callback != null)
    //                    {
    //                        num = callback(num, reconnectCount);
    //                    }
    //                    if (num > reconnectCount)
    //                    {
    //                        return;
    //                    }
    //                    tryCount = num;
    //                    if (sessionStopped)
    //                    {
    //                        connector.RestartConnector();
    //                    }
    //                    else
    //                    {
    //                        connector.RestartConnector();
    //                    }
    //                }
    //            }
    //        }
    //    }
    //}



}
