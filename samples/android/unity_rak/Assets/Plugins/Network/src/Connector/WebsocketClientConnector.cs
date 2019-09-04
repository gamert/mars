#if USE_BASTHTTP
#define C_SERVER

#if !C_SERVER
#define DATA_COMPRESS
#endif

using System;
using System.Collections;

using BestHTTP.WebSocket;
using BestHTTP.Cookies;

using Comm.EventBus;

namespace Comm.Network
{
    public class WebsocketClientConnector : BaseConnector
    {
        private WebSocket _websocket = null;

        public override void Connect(string ip, int port, string origin = "", string protocol = "", int uid = 0, byte[] token = null)
        {
            if (_websocket == null)
            {
                _addr = new AddressInfo(ip, port, origin, protocol);
                _uid = (uint)uid;
                this.token = token;
#if C_SERVER
                string url = "ws://" + ip + ":" + port + "?p=" + DateTime.UtcNow.Ticks;
                _websocket = new WebSocket(new System.Uri(url));
#else
                string url = "wss://" + ip + ":" + port + "?p=" + DateTime.UtcNow.Ticks;
                string tokenStr = Convert.ToBase64String(_token);   
                _websocket = new WebSocket(new System.Uri(url), origin, protocol);
                _websocket.InternalRequest.Cookies.Add(new Cookie("TOKEN", UnityEngine.WWW.EscapeURL(tokenStr)));
                _websocket.InternalRequest.Cookies.Add(new Cookie("PLATFORM_ID", uid.ToString()));
#endif
                _websocket.OnClosed += OnClose;
                _websocket.OnOpen += OnConnected;
                _websocket.OnBinary += OnMessageReceived;
                _websocket.OnError += OnError;
                _websocket.Open();

                NotifyEvents((int)EObjDeliverOPCode.E_OP_CONN_CONNECTING);
                state = EConnectorState.E_CON_ST_CONNECTING;
            }
        }

        public override void Shutdown(string reason)
        {
            Close(reason);
            mMsgBuffer.Purge();
        }

        public override void Close(string reason)
        {
            if (_addr != null)
                Log(_addr.ip + " WebsocketClientConnector: Close ;reason:" + reason);

            if (state > EConnectorState.E_CON_ST_CLOSING)
            {
                bool isopen = false;
                if (_websocket != null)
                {
                    isopen = _websocket.IsOpen;
                    try
                    {
                        _websocket.Close();
                    }
                    catch
                    {
                        isopen = false;
                    }
                }

                NotifyEvents((int)EObjDeliverOPCode.E_OP_CONN_CLOSING);
                state = EConnectorState.E_CON_ST_CLOSING;
                if (!isopen)
                {
                    OnClose(_websocket, 0, default(string));
                }
            }
        }

        protected override bool Send(byte[] data, int len, int reliability)
        {
            byte[] sizedData = null;
            SharedBuffer sb = null;

            if (data.Length != len)
            {
                sb = TPoolClass<SharedBuffer>.AutoCreate();
                if (sb.buffer.Length < len)
                    sb.buffer = new byte[Utility.NextPowerOfTwo(len)];

                sizedData = sb.buffer;
                Buffer.BlockCopy(data, 0, sizedData, 0, len);
            }
            else
            {
                sizedData = data;
                len = data.Length;
            }

            bool result = true;
            try
            {
                _websocket.Send(sizedData, 0, (ulong)len);
            }
            catch
            {
                OnClose(_websocket, 0, default(string));
                result = false;
            }
            finally
            {
                if (sb != null)
                    sb.Release();
            }

            return result;
        }

        virtual protected void OnConnected(WebSocket ws)
        {
            Log(_addr.ip + " WebsocketClientConnector: OnConnected!");
            state = EConnectorState.E_CON_ST_CONNECTED;
            NotifyEvents((int)EObjDeliverOPCode.E_OP_CONN_CONNECTED);
            InvokeEvent_OnConnected();
        }

        private void OnClose(WebSocket ws, UInt16 code, string message)
        {
            if(_addr!=null)
                Log(" WebsocketClientConnector: OnClosed!"+ _addr.ip);
            if (!string.IsNullOrEmpty(message))
                Log(_addr.ip + " WebsocketClientConnector: " + message);

            state = EConnectorState.E_CON_ST_CLOSED;

            //#8407 NullReferenceException
            //A null value was found where an object instance was required.
            if (_websocket != null)
            {
                _websocket.OnClosed -= OnClose;
                _websocket.OnError -= OnError;
                _websocket.OnBinary -= OnMessageReceived;
                _websocket.OnOpen -= OnConnected;
                _websocket = null;
            }

            NotifyEvents((int)EObjDeliverOPCode.E_OP_CONN_CLOSED);
            InvokeEvent_OnClosed();
        }
        NByteBuffer mMsgBuffer = new NByteBuffer();

        byte[] data_left = null;
        public void OnMessageReceived(WebSocket ws, byte[] data)
        {
            int len = data.Length;
            AddStatisticOfReceivedPacket(data, len);

            //zz: 如果数据被处理过 且还有剩余数据，那么要多次处理...
            if (mMsgBuffer.dataLen() != 0)
            {
                BaseConnector.Log("WCC: 剩余报文，mMsgBuffer.dataLen=" + mMsgBuffer.dataLen());
                mMsgBuffer.Purge();
            }

            //if(data_left != null)
            //{
            //    mMsgBuffer.Append(data_left, 0, data_left.Length);
            //    data_left = null;
            //}
            //else
            //{
            //    int ll = data.Length / 2;
            //    data_left = new byte[data.Length - ll];

            //    byte[] data_rigth = new byte[ll];
            //    Buffer.BlockCopy(data, 0, data_rigth, 0, ll);
            //    Buffer.BlockCopy(data, ll, data_left, 0, data.Length - ll);
            //    data = data_rigth;
            //}


            mMsgBuffer.Append(data, 0, data.Length);
            _Handle();
            //if (data_left != null)
            //{
            //    mMsgBuffer.Append(data_left, 0, data_left.Length);
            //    data_left = null;

            //    _Handle();
            //}
        }

        private void _Handle()
        {
            int maxhandled = 4;
            int handled = 0;
            do
            {
                handled = 0;
                for (int ii = 0; ii < _pluginList.Count; ++ii)
                {
                    if (_pluginList[ii].Process(mMsgBuffer))
                    {
                        handled++;
                        break;
                    }
                }
            } while (mMsgBuffer.dataLen() > 0 && handled > 0 && (--maxhandled) > 0);

            //zz:仅当还有剩余数据，则继续处理
            if (mMsgBuffer.dataLen() > 0 && !HookReceivedData(mMsgBuffer))
            {
#if DATA_COMPRESS
                DataWrapper wrapper = new DataWrapper(decompressed_data, len);
#else
                DataWrapper wrapper = new DataWrapper(0, mMsgBuffer);
#endif
                InvokeEvent_OnDataReceived(wrapper);
            }
        }

        private void OnError(WebSocket ws, Exception ex)
        {
            Log("WebsocketClientConnector::OnError!");

            NotifyEvents((int)EObjDeliverOPCode.E_OP_CONN_ERROR);
            Close("WebsocketClientConnector::OnError!");
            if (_websocket != null)
            {
                OnClose(_websocket, 0, ws.InternalRequest.Response != null ? ws.InternalRequest.Response.Message : default(string));
            }
        }

        protected override bool HookReceivedData(NByteBuffer msg)
        {
            return false;
        }

        public override bool Reconnect(string reason)
        {
            Log("WebsocketClientConnector::Reconnect:state=" + state + ";reason:" + reason);

            if (_addr != null && state == EConnectorState.E_CON_ST_CLOSED)
            {
#if C_SERVER
                string url = "ws://" + _addr.ip + ":" + _addr.port + "?p=" + DateTime.UtcNow.Ticks;
                _websocket = new WebSocket(new System.Uri(url));
#else
                string url = "wss://" + _addr.ip + ":" + _addr.port + "?p=" + DateTime.UtcNow.Ticks;
                string tokenStr = Convert.ToBase64String(_token);
                _websocket = new WebSocket(new Uri(url), _addr.origin, _addr.protocol);
                _websocket.InternalRequest.Cookies.Add(new Cookie("TOKEN", UnityEngine.WWW.EscapeURL(tokenStr)));
                _websocket.InternalRequest.Cookies.Add(new Cookie("PLATFORM_ID", uid.ToString()));
#endif
                _websocket.OnClosed += OnClose;
                _websocket.OnOpen += OnConnected;
                _websocket.OnBinary += OnMessageReceived;
                _websocket.OnError += OnError;
                _websocket.Open();

                NotifyEvents((int)EObjDeliverOPCode.E_OP_CONN_CONNECTING);
                state = EConnectorState.E_CON_ST_CONNECTING;
                return true;
            }
            return false;
        }

        //Websocket close failed, 临时使用for c server
        public override void NewConnect(string ip, int port, string origin = "", string protocol = "", int uid = 0, byte[] token = null)
        {
            if (_websocket != null)
            {
                _websocket.OnClosed -= OnClose;
                _websocket.OnOpen -= OnConnected;
                _websocket.OnBinary -= OnMessageReceived;
                _websocket.OnError -= OnError;
            }

            string url = "ws://" + ip + ":" + port + "?p=" + DateTime.UtcNow.Ticks;

            Log("WebsocketClientConnector::NewConnect:url=" + url);

            _addr = new AddressInfo(ip, port, origin, protocol);

            _uid = (uint)uid;
            this.token = token;

            //mMsgBuffer.Purge();

            string tokenStr = Convert.ToBase64String(_token);
            _websocket = new WebSocket(new System.Uri(url));
            _websocket.OnClosed += OnClose;
            _websocket.OnOpen += OnConnected;
            _websocket.OnBinary += OnMessageReceived;
            _websocket.OnError += OnError;
            _websocket.Open();

            NotifyEvents((int)EObjDeliverOPCode.E_OP_CONN_CONNECTING);
            state = EConnectorState.E_CON_ST_CONNECTING;
        }
    }
}
#endif