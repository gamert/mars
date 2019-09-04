using System;
using System.Collections.Generic;
using RakNet;

public enum SOCKET_STATUS
{
    CONNECTING = 0,         //正在连接
    CONNECTED = 1,          //已经连接
    CLOSING = 2,            //正在关闭
    CLOSED = 3,             //已经关闭
}


namespace Comm.Network
{

    /// <summary>
    /// 
    /// </summary>
    public class SocketBase : IDisposable
    {
        static int sguid = 1;
        public RakNetWrapper.Connection conn = null;
        //接收缓存...
        //public static byte[] s_Buffer = new byte[4096 * 2]; //至多为32k
        public SocketBase()
        {
            //guid = sguid++;
        }

        public BaseConnector m_Mgr
        {
            get;
            protected set;
        }

        public SOCKET_STATUS m_Status
        {
            get;
            set;
        }


        // <summary>
        /// tcp服务器ip地址
        /// </summary>
        public string tcpServerIp
        {
            get;
            set;
        }
        /// <summary>
        /// tcp 服务器的端口
        /// </summary>
        public int tcpPort
        {
            get;
            set;
        }

        public static void LogError(object message)
        {
            //UnityEngine.Debug.LogError(message);
            BaseConnector.Log(message.ToString());
        }

        public virtual int connectServer(string ip, int port, BaseConnector mgr)
        {
            m_Mgr = mgr;
            tcpServerIp = ip;
            tcpPort = port;
            //mNeedSendMsg = new List<BaseStructSend>();
            return 1;
        }

        public virtual bool Send(byte[] buffer, int offset, int size)
        {
            //            LogError("SocketBase::Send");
            return false;
        }

        public virtual void Update()
        {
        }

        public virtual void Dispose()
        {
            m_DataReceived = null;

            m_Connected = null;
            m_Connected = null;
            m_Closed = null;
            m_TimeOut = null;
        }

        public virtual bool Isconnected()
        {
            LogError("SocketBase::Isconnected");
            return false;
        }

        public event EventHandler Connected
        {
            add { m_Connected += value; }
            remove { m_Connected -= value; }
        }
        protected void OnConnected()
        {
            LogError("SocketBase::OnConnected");

            m_Status = SOCKET_STATUS.CONNECTED;

            //创建一个conn
            conn = new RakNetWrapper.Connection();
            conn.ip = this.tcpServerIp;
            conn.port = (ushort)tcpPort;
            conn.guid = (ulong)sguid++;

            if (m_Connected == null)
            {
                LogError("SocketBase::OnConnected:m_Connected == null");
                return;
            }

            m_Connected(this, EventArgs.Empty);
            //if (mNeedSendMsg.Count > 0)
            //{
            //    //Send(mNeedSendMsg[0]);
            //    mNeedSendMsg.RemoveAt(0);
            //}
        }


        public event EventHandler TimeOut
        {
            add { m_TimeOut += value; }
            remove { m_TimeOut -= value; }
        }

        protected void OnTimeOut()
        {
            //
            BaseConnector.Log("BaseSocket:OnTimeOut:"+ m_TimeOut);
            if (m_TimeOut == null)
            {
                return;
            }

            m_TimeOut(this, EventArgs.Empty);
#if LOG_NET
//            ServerPackInfo.GetInstance().AddString(8, this.GetType().Name + " OnTimeOut");
#endif
        }

        public event EventHandler<DataEventArgs> DataReceived
        {
            add { m_DataReceived += value; }
            remove { m_DataReceived -= value; }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="length"></param>
        protected void OnDataReceived(byte[] data, int offset, int length)
        {
            if (m_DataReceived == null)
            {
                LogError("SocketBase::OnDataReceived:length=" + length);
                return;
            }

            m_DataArgs.Data = data;
            m_DataArgs.Offset = offset;
            m_DataArgs.Length = length;

            m_DataReceived(this, m_DataArgs);
        }



        public event EventHandler Closed
        {
            add { m_Closed += value; }
            remove { m_Closed -= value; }
        }

        protected void OnClosed()
        {
            m_Status = SOCKET_STATUS.CLOSED;

            if (m_Closed != null)
                m_Closed(this, EventArgs.Empty);
        }


        public virtual void Close()
        {

        }
        public virtual void Shutdown()
        {
        }

        protected EventHandler<DataEventArgs> m_DataReceived;

        protected EventHandler m_Connected;
        protected EventHandler m_Closed;
        protected EventHandler m_TimeOut;

        //protected List<BaseStructSend> mNeedSendMsg = new List<BaseStructSend>();   //缓存消息...(在断线或者连接前...)
        protected DataEventArgs m_DataArgs = new DataEventArgs();

        protected const int m_TimeoutDuration = 5000;
        protected const int maxReceiveBuffLength = 4096*4;

    }



}
