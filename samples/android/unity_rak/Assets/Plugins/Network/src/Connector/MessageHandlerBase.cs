using UnityEngine;
using System;

namespace Comm.Network
{


    public interface IMHInitializer
    {
        BaseConnector CreateConnector();
        IMonitor[] CreateMonitors();
        BaseConnector RenewConnector(BaseConnector currentConn);
    }


    abstract public class MessageHandlerBase : IPacketStatistic, IDisposable
    {
        protected BaseConnector _conn = null;
        protected IMonitor[] _monitors = null;
        protected NetworkStatistic _stat = new NetworkStatistic();

        private int _receivedPacketsWithinSec = 0;
        private int _sentPacketsWithinSec = 0;
        private float _elapsedTime = 0f;
        private bool _fastConnChk = false;

        public NetworkStatistic stat
        {
            get { return _stat; }
        }

        public bool IsValid
        {
            get { return _conn != null; }
        }

        //public bool fastConnectionLossChk
        //{
        //    get { return _fastConnChk; }
        //    set {
        //        if (_fastConnChk != value)
        //        {
        //            _fastConnChk = value;
        //            if (_ping == null)
        //            {
        //                if (_monitors != null && _monitors[(int)EMonitorType.E_MT_PING] != null)
        //                {
        //                    _ping = _monitors[(int)EMonitorType.E_MT_PING];
        //                }
        //            }

        //            if (_ping != null)
        //            {
        //                BaseMonitor bm = (BaseMonitor)_ping;
        //                if (_fastConnChk)
        //                {
        //                    bm.AddListener("OnPingLoss", OnFastCheckDisconn);
        //                }
        //                else
        //                {
        //                    bm.RemoveListener("OnPingLoss", OnFastCheckDisconn);
        //                }
        //            }
        //        }
        //    }
        //}



        protected IMonitor _ping = null;
        public int pingTime
        {
            get
            {
                if (_ping == null)
                {
                    _ping = GetMonitor((int)EMonitorType.E_MT_PING);
                }

                if (_ping != null && _ping.ContainsData(0))
                    return (int)_ping.GetData(0);

                return 0;
            }
        }

        public BaseConnector conn
        {
            get
            {
                return _conn;
            }
        }

        protected VoidCallback evt_OnConnected;
        protected VoidCallback evt_OnClosed;
        protected VoidCallback evt_OnTimeout;
        protected NetworkMsgErrorCallback evt_OnMsgError;
        protected NetworkMsgErrorCallback evt_OnReachabilityChange;
        protected IMHInitializer _initializer = null;
        protected bool _monitorActiveFlag = false;

        protected enum E_RENEW_MODE
        {
            NONE = 0,
            RAW,
            STANDARD
        }
        protected E_RENEW_MODE _renewMode = E_RENEW_MODE.NONE;

        public MessageHandlerBase(IMHInitializer initializer)
        {
            if (initializer != null)
            {
                _initializer = initializer;
                _conn = initializer.CreateConnector();
                _monitors = initializer.CreateMonitors();

                for (int ii = 0; ii < _monitors.Length; ++ii)
                {
                    if (_monitors[ii] != null)
                        _monitors[ii].SetObserver(this);
                }

                InitConnCallbacks();
            }
        }

        protected void InitConnCallbacks()
        {
            if (_conn != null)
            {
                _conn.AddCallback_OnConnected(OnConnected);
                _conn.AddCallback_OnClosed(OnClosed);
                _conn.AddCallback_OnTimeout(OnTimeout);
                _conn.pktStat = this;
            }
        }

        public bool IsConnected()
        {
            return _conn != null ? _conn.IsConnected : false;
        }

        public void SetKeepAliveInterval(float interval)
        {
            if (_ping == null)
            {
                _ping = GetMonitor((int)EMonitorType.E_MT_PING);
            }

            if (_ping != null)
                _ping.Pulse = interval;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="ip"></param>
        /// <param name="port"></param>
        /// <param name="KAInterval">心跳间隔, 战斗服默认2s,大厅默认3s</param>
        /// <param name="origin"></param>
        /// <param name="protocol"></param>
        /// <param name="uid"></param>
        /// <param name="token"></param>
        public void Connect(string ip, int port, float KAInterval = 2f, string origin = "", string protocol = "", int uid = 0, byte[] token = null)
        {
#if UNITY_EDITOR
            //测试IP
            if(ip == "8.8.8.8")
            {
                EventArgs e = new EventArgs();
                OnConnected(null, e);
                conn.state = EConnectorState.E_CON_ST_CONNECTED;    
                return;
            }
#endif

            if (_conn != null)
            {
                _conn.Connect(ip, port, origin, protocol, uid, token);
                SetKeepAliveInterval(KAInterval);
            }
        }

        public void NewConnect(string ip, int port, float KAInterval = 2f, string origin = "", string protocol = "", int uid = 0, byte[] token = null)
        {
            if (_conn != null)
            {
                _conn.NewConnect(ip, port, origin, protocol, uid, token);
                SetKeepAliveInterval(KAInterval);
            }
        }

        private void OnConnected(object sender, EventArgs e)
        {
            EnableMonitors(true);
            if (evt_OnConnected != null)
                evt_OnConnected();
        }

        private void OnClosed(object sender, EventArgs e)
        {
            if (evt_OnClosed != null)
                evt_OnClosed();
        }

        private void OnTimeout(object sender, EventArgs e)
        {
            if (evt_OnTimeout != null)
                evt_OnTimeout();
        }

        private void OnReachabilityChange(int internetReachability)
        {
            if (evt_OnReachabilityChange != null)
                evt_OnReachabilityChange(internetReachability);
        }

        /// <summary>
        /// 
        /// </summary>
        public void Update()
        {
            if (_conn != null)
            {
                //Profiler.BeginSample("_conn.Update()"); //zz:所有上层的mem: 404B/44B
                _conn.Update();
                //Profiler.EndSample();

                if (_monitors != null)
                {
                    for (int ii = 0; ii < (int)EMonitorType.num && ii < _monitors.Length; ++ii)
                    {
                        if (_monitors[ii] != null)
                            _monitors[ii].Update(UnityEngine.Time.smoothDeltaTime);
                    }
                }

                UpdateStat();

                if (_conn.IsConnected == false)
                {
                    if (_renewMode == E_RENEW_MODE.RAW)
                    {
                        RenewRawConnection();
                        _renewMode = E_RENEW_MODE.NONE;
                    }
                    else if (_renewMode == E_RENEW_MODE.STANDARD)
                    {
                        RenewConnection();
                        _renewMode = E_RENEW_MODE.NONE;
                    }
                }

                //只要网络变化了？弹框提示...
                if (this.netStateChanged)
                {
                    if (this.internetReachability == 0)
                    {
                        //CUIManager.instance.OpenSendMsgAlert(Singleton<CTextManager>.GetInstance().GetText("NetworkConnecting"), 10, enUIEventID.None);
                    }
                    else
                    {
                        BaseConnector.Log("MHB::Update:ForceTick by netStateChanged " + netStateChanged + "; internetReachability = " + internetReachability);
                        IMonitor mon = GetMonitor((int)EMonitorType.E_MT_DISCONNECT);
                        if(mon!=null)
                            mon.ForceTick();
                        //Debug.LogWarning();
                        //Reconnect("MHB::Update:Reconnect by netStateChanged " + netStateChanged + ";internetReachability=" + internetReachability);
                        //CUIManager.instance.CloseSendMsgAlert();
                    }
                    OnReachabilityChange(internetReachability);
                    this.netStateChanged = false;
                }
            }

            OnUpdate();
        }

        abstract protected void OnUpdate();
        virtual public void LateUpdate() { }
        //zzModi: 修改名字:
        public void CloseConnector(bool shutdown ,string reason)    //= false
        {
            if (_conn != null)
            {
                // disable monitors
                EnableMonitors(false);
                if (!shutdown)
                    _conn.Close("MHB::Close::shutdown=" + shutdown+ ";reason="+ reason);
                else
                    _conn.Shutdown("MHB::Close::shutdown=" + shutdown + ";reason=" + reason);
            }
        }

        public void AddDelegateOnConnected(VoidCallback cb)
        {
            if (cb != null)
            {
                evt_OnConnected -= cb;
                evt_OnConnected += cb;
            }
        }

        public void AddDelegateOnTimeout(VoidCallback cb)
        {
            if (cb != null)
            {
                evt_OnTimeout -= cb;
                evt_OnTimeout += cb;
            }
        }

        public void AddDelegateOnClosed(VoidCallback cb)
        {
            if (cb != null)
            {
                evt_OnClosed -= cb;
                evt_OnClosed += cb;
            }
        }

        public void AddDelegateOnReachabilityChange(NetworkMsgErrorCallback cb)
        {
            if (cb != null)
            {
                evt_OnReachabilityChange -= cb;
                evt_OnReachabilityChange += cb;
            }
        }

        public void RemoveDelegateOnConnected(VoidCallback cb)
        {
            if (cb != null)
                evt_OnConnected -= cb;
        }

        public void RemoveDelegateOnTimeout(VoidCallback cb)
        {
            if (cb != null)
                evt_OnTimeout -= cb;
        }

        public void RemoveDelegateOnClosed(VoidCallback cb)
        {
            if (cb != null)
                evt_OnClosed -= cb;
        }

        public void AddDelegateMessageError(NetworkMsgErrorCallback cb)
        {
            if (cb != null)
            {
                evt_OnMsgError -= cb;
                evt_OnMsgError += cb;
            }
        }

        public void RemoveDelegateMessageError(NetworkMsgErrorCallback cb)
        {
            if (cb != null)
                evt_OnMsgError -= cb;
        }

        public void RemoveDelegateOnReachabilityChange(NetworkMsgErrorCallback cb)
        {
            if (cb != null)
                evt_OnReachabilityChange -= cb;
        }        

        public void NotifyMessageError(int errCode)
        {
            if (evt_OnMsgError != null)
                evt_OnMsgError(errCode);
        }

        public IMonitor GetMonitor(int index)
        {
            if (_monitors != null && index >= 0 && index < _monitors.Length)
                return _monitors[index];
            return null;
        }

        //DisconnectMonitor:触发重连..
        //false: 失败
        public bool Reconnect(string reason)
        {
            if (_conn != null)
                return _conn.Reconnect("MHB::Reconnect;reason:" + reason);
            return false;
        }

        public void Dispose()
        {
            if (_monitors != null)
            {
                for (int ii = 0; ii < _monitors.Length; ++ii)
                {
                    IDisposable disp = _monitors[ii] as IDisposable;
                    if (disp != null)
                        disp.Dispose();
                }
            }

            if (_conn != null)
            {
                _conn.Shutdown("MessageHandlerBase::Dispose");
            }
        }

        protected void EnableMonitors(bool flag)
        {
            if (_monitors != null)
            {
                for (int ii = 0; ii < _monitors.Length; ++ii)
                {
                    if (_monitors[ii] != null)
                    {
                        _monitors[ii].Activate = flag;
                        if (flag) // reset observer
                            _monitors[ii].SetObserver(this);
                    }
                }
            }
        }

        public void RenewRawConnection(bool immediate = true)
        {
            BaseConnector.Log("MHB:RenewRawConnection:immediate="+ immediate);
            if (!immediate)
            {
                _renewMode = E_RENEW_MODE.RAW;
            }
            else
            {
                if (_conn != null)
                {
                    _conn.RemoveCallback_OnClosed(OnClosed);
                    _conn.RemoveCallback_OnConnected(OnConnected);
                    _conn.RemoveCallback_OnTimeout(OnTimeout);
                }

                EnableMonitors(false);
                BaseConnector conn = _initializer.RenewConnector(_conn);//_initializer.CreateConnector();           

                conn.AddCallback_OnClosed(OnClosed);
                conn.AddCallback_OnConnected(OnConnected);
                conn.AddCallback_OnTimeout(OnTimeout);
                conn.pktStat = this;

                _conn = conn;
            }
        }

        /// <summary>
        /// 重新创建
        /// </summary>
        /// <param name="immediate"></param>
        public void RenewConnection(bool immediate = true)
        {
            BaseConnector.Log("MHB::RenewConnection:immediate="+ immediate);
            if (!immediate)
            {
                _renewMode = E_RENEW_MODE.STANDARD;
            }
            else
            {
                AddressInfo addr = null;
                uint id = 0u;
                byte[] token = null;
                if (_conn != null)
                {
                    addr = _conn.addr;
                    id = _conn.uid;
                    token = _conn.token;
                    _conn.RemoveCallback_OnClosed(OnClosed);
                    _conn.RemoveCallback_OnConnected(OnConnected);
                    _conn.RemoveCallback_OnTimeout(OnTimeout);
                }
                //临时关闭？
                EnableMonitors(false);
                BaseConnector conn = _initializer.RenewConnector(_conn);//_initializer.CreateConnector();

                conn.AddCallback_OnClosed(OnClosed);
                conn.AddCallback_OnConnected(OnConnected);
                conn.AddCallback_OnTimeout(OnTimeout);

                // post renew
                if (_conn != conn)
                {
                    PostRenewConnection(_conn, conn);
                    _conn.Close("MHB::RenewConnection:_conn != conn");
                    _conn = conn;
                }

                if (addr != null)
                    conn.addr = addr;

                conn.uid = id;
                conn.token = token;
                conn.pktStat = this;

                if (addr != null)
                    EnableMonitors(true);
            }
        }

        virtual protected void PostRenewConnection(BaseConnector old_c, BaseConnector new_c) { }

        public void AddStatisticOfSentPacket(object msg, int len)
        {
            _stat.totalSentPacketNum++;
            _stat.totalSentBytes += (ulong)len;
            ++_sentPacketsWithinSec;
        }

        public void AddStatisticOfReceivedPacket(object msg, int len)
        {
            _stat.totalReceivedPacketNum++;
            _stat.totalReceivedBytes += (ulong)len;
            ++_receivedPacketsWithinSec;
        }

        public void UpdateStat()
        {
            if (_conn != null && _conn.state > EConnectorState.E_CON_ST_CLOSED)
            {
                _elapsedTime += Time.deltaTime;
                if (_elapsedTime > 1f || Mathf.Abs(_elapsedTime - 1f) < 0.01f)
                {
                    _stat.sentPPS = _sentPacketsWithinSec / _elapsedTime;
                    _stat.receivePPS = _receivedPacketsWithinSec / _elapsedTime;

                    _elapsedTime = 0f;
                    _sentPacketsWithinSec = 0;
                    _receivedPacketsWithinSec = 0;
                }
            }
        }

        public void ResetStat()
        {
            _stat.totalSentBytes = 0uL;
            _stat.totalSentPacketNum = 0;
            _stat.totalReceivedBytes = 0uL;
            _stat.totalReceivedPacketNum = 0;
            _stat.sentPPS = 0f;
            _stat.receivePPS = 0f;
        }

        // plugins
        public void AddPlugin<T>() where T : IMessagePlugin, new()
        {
            if (_conn != null)
                _conn.AddPlugin<T>();
        }

        public void RemovePlugin<T>() where T : IMessagePlugin
        {
            if (_conn != null)
                _conn.RemovePlugin<T>();
        }

        public T FindPlugin<T>() where T : IMessagePlugin
        {
            if (_conn != null)
                return _conn.FindPlugin<T>();

            return default(T);
        }

        // ping loss check, using for fast checking disconnection
        protected void OnFastCheckDisconn(object args)
        {
            BaseConnector.Log("MHB::OnFastCheckDisconn");

            if (_conn != null)
                _conn.Close("MHB::OnFastCheckDisconn");
            // renew connection
            //RenewConnection();
        }

        private int internetReachability = 0;   //默认
        private bool netStateChanged;
        public void OnNetStateChanged(string text)
        {
            netStateChanged = true;

            //Application.internetReachability == NetworkReachability.NotReachable
            BaseConnector.Log("MHB::OnNetStateChanged:" + text+ ";APP.Reachability="+ Application.internetReachability);
            int state = 0;
            if(text!=null)
            {
                int.TryParse(text, out state);
                internetReachability = state;
                if (internetReachability!=0)
                {
                    RenewConnection();
                }
                else
                {
                    //fix: 如果连接上，那么通知上层已经断网...

                }
            }
        }
    }
}
