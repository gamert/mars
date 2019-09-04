namespace Comm.Network
{
    //通知重连代理...
    public delegate void OnNotifyReconnectionTimes(object handler, int cur, int max);
    public class DisconnectMonitor : BaseMonitor
    {
        private bool _isReconnecting = false;
        private int _triedCount = 0;
        private bool _retryLimitationNtf = false;
        OnNotifyReconnectionTimes m_fun;
        //
        public void SetOnNotifyReconnectionTimes(OnNotifyReconnectionTimes fun)
        {
            m_fun = fun;
        }

        public override void Reset ()
        {
            _triedCount = 0;
            _isReconnecting = false;
            _retryLimitationNtf = false;
            Pulse = 5.0f;   //首次
        }

        public DisconnectMonitor(OnNotifyReconnectionTimes fun)
        {
            m_fun = fun;
            SetData(0, 2);     //重连限制
            SetData(1, false);
            SetData(2, false);  //Renew标记
        }

        public override void SetObserver (object ob)
        {
            if(_handler != null)
            {
                _handler.RemoveDelegateOnClosed(OnClose);
                _handler.RemoveDelegateOnConnected(OnConnected);
            }

            _handler = (MessageHandlerBase)ob;
            if (_handler != null)
            {
                _handler.AddDelegateOnClosed(OnClose);
                _handler.AddDelegateOnConnected(OnConnected);
            }
        }

        //每5s启动? 
        protected override void OnInterval()
        {
            if (_handler != null && _handler.IsConnected() == false 
                && _isReconnecting == false 
                && !_retryLimitationNtf)
            {
                string reason = GetPrefix() + "::DisconnectMonitor::OnInterval:_triedCount=" + _triedCount + ";Pulse=" + Pulse;
                Log(reason);
                if(_handler.Reconnect("OnInterval"))
                {
                    _isReconnecting = true;
                }
                Pulse = 10f;
                ++_triedCount;
                //重连
                if (m_fun != null)
                    m_fun((object)_handler, _triedCount, (int)GetData(0));// ("OnLimitedRetryTimes", _handler);
            }
        }

        public override void Dispose()
        {
            if (_handler != null)
            {
                _handler.RemoveDelegateOnClosed(OnClose);
                _handler.RemoveDelegateOnConnected(OnConnected);
                _handler = null;
            }
        }

        //OnError触发: 
        private void OnClose() 
        {
            Log(GetPrefix() + "::DisconnectMonitor::OnClose:_isReconnecting=" + _isReconnecting+ ";_triedCount="+ _triedCount);

            if (Activate || _isReconnecting)
            {
                Pulse = _triedCount == 0 ? 0.1f : 6;   //增强断网处理。
                ResetTicks();

                //if ((bool)GetData(1) == true)
                //    Notify("OnNotifyReconnectionTimes", new object[] { (object)_handler, (object)_triedCount, GetData(0) });
                
                //ios关闭连接，需要重启？

                _isReconnecting = false;
                if (!_retryLimitationNtf)
                {
                    int retryCount = (int)GetData(0);
                    if (retryCount > 0 && _triedCount >= retryCount)    //8次
                    {
                        if (m_fun != null)
                            m_fun((object)_handler, _triedCount+1, (int)GetData(0));// ("OnLimitedRetryTimes", _handler);
                        _retryLimitationNtf = true;
                    }
                }
            }
        }

        private void OnConnected()
        {
            if (_isReconnecting)
            {
            }
            _triedCount = 0;
            _isReconnecting = false;
            _retryLimitationNtf = false;
            Pulse = 5.0f;   //首次
            if(Activate)
            {
                Notify("OnConnected", _handler);
            }
        }

        protected override void OnActivate(bool flag)
        {
            Log(GetPrefix()+"::DisconnectMonitor::OnActivate:flag = "+ flag + ";_isReconnecting = " + _isReconnecting + ";_triedCount=" + _triedCount);

            if (!flag)
            {
                _isReconnecting = false;
                _retryLimitationNtf = false;
            }
            else
            {
                ResetTicks();   //如果在这里执行，那么会导致2次间隔为 10s+4s（连接到返回失败dt）；
            }
            //if(_triedCount <= 1 && flag)
            //{
            //}
        }


    }
}
