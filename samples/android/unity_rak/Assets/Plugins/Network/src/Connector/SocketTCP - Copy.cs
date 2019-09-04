//using System;
//using System.Net;
//using System.Net.Sockets;
//using System.Runtime.InteropServices;
//using System.Threading;

//namespace MobaGo.Network
//{
//    //
//    /// <summary>
//    /// for TcpSocketImp
//    /// </summary>
//    class TcpSocketImp : SocketBase
//    {
//        //private const int ReceiveOperation = 1, SendOperation = 0;

//        //private static AutoResetEvent[] autoSendReceiveEvents = new AutoResetEvent[]
//        // {
//        //    new AutoResetEvent(false),
//        //    new AutoResetEvent(false)
//        // };
//        /// <summary>  
//        /// 服务器监听端点  
//        /// </summary>  
//        private IPEndPoint _remoteEndPoint;

//        AsyncCallback m_asyncCB;
//        public TcpSocketImp()
//        {
//            m_asyncCB = new AsyncCallback(sendCallback);
//        }


//        /// <summary>
//        /// connect tcp server
//        /// </summary>
//        public override int connectServer(string ip, int port, BaseConnector mgr)
//        {
//            base.connectServer(ip, port, mgr);
//            if (_clientSock != null && _clientSock.Connected)
//            {
//                return 0;
//            }
//            //BaseConnector.Log("connectServer");

//            //服务器IP地址
//            try
//            {
//                IPAddress ipAddress = IPAddress.Parse(tcpServerIp);

//                if (ipAddress.AddressFamily == AddressFamily.InterNetwork)
//                {
//                    //创建IPV4 Socket对象
//                    _clientSock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
//                }
//                else if (ipAddress.AddressFamily == AddressFamily.InterNetworkV6)
//                {
//                    //创建IPV6 Socket对象
//                    _clientSock = new Socket(AddressFamily.InterNetworkV6, SocketType.Stream, ProtocolType.Tcp);
//                }

//                BaseConnector.Log("Create Socket : " + tcpServerIp + "," + tcpPort+";"+ ipAddress.AddressFamily+ ";NoDelay=" + _clientSock.NoDelay);

//                //_clientSock.ReceiveBufferSize = 64 * 1024;
//                //_clientSock.SendBufferSize = 32 * 1024;
//                //_clientSock.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveTimeout,10000);
//                //_clientSock.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendTimeout, 5000);
//                //_clientSock.NoDelay = true;
//                //服务器端口
//                _remoteEndPoint = new IPEndPoint(ipAddress, tcpPort);
//                //
//                mRevEvent = new SocketAsyncEventArgs();
//                mRevEvent.UserToken = _clientSock;
//                mRevEvent.RemoteEndPoint = _remoteEndPoint;
//                mRevEvent.Completed += new EventHandler<SocketAsyncEventArgs>(mRevEvent_Completed);

//                //#if !WIN_CLIENT
//                //                BaseConnector.Log("connectServer==" + tcpServerIp + "," + tcpPort);
//                //#endif
//                //开始异步连接    
//                //Returns true if the I/O operation is pending.
//                // Returns:
//                //     Returns true if the I/O operation is pending. The System.Net.Sockets.SocketAsyncEventArgs.Completed
//                //     event on the e parameter will be raised upon completion of the operation. Returns
//                //     false if the I/O operation completed synchronously. In this case, The System.Net.Sockets.SocketAsyncEventArgs.Completed
//                //     event on the e parameter will not be raised and the e object passed as a parameter
//                //     may be examined immediately after the method call returns to retrieve the result
//                //     of the operation.
//                if (!_clientSock.ConnectAsync(mRevEvent))
//                {
//                   // ProcessConnected(connectArgs);
//                }

//                m_Status = SOCKET_STATUS.CONNECTING;
//                return 1;
//            }
//            catch (Exception e)
//            {
//                BaseConnector.Log("connectServer fail: e=" + e);
//            }
//            return 0;
//        }

//        //事件回调: 
//        private void mRevEvent_Completed(object sender, SocketAsyncEventArgs e)
//        {
//            if (e.LastOperation == SocketAsyncOperation.Connect)
//            {
//                ProcessConnect(sender as Socket, null, e);
//                return;
//            }
//            ProcessReceive(e);
//        }

//        //bug: not implement excetion..
//        //+		e	"System.NotImplementedException: The requested feature is not implemented.\r\n  at System.Net.Sockets.Socket.IOControl
//        //(IOControlCode ioControlCode, System.Byte[] optionInValue, System.Byte[] optionOutValue) [0x00000] in <filename unknown>:0 \r\n  
//        //at MobaGo.Network.TcpSocketImp.SetKeepALive (System.Net.Sockets.Socket socket) [0x00065] 
//        //
//        void SetKeepALive(Socket socket)
//        {
//            //fix:在C#中利用Keep-Alive处理Socket网络异常断开的方法
//            //https://www.cnblogs.com/ngcheck/articles/2891687.html
//            /* 
//             1、客户端程序异常。
//                 对于这种情况，我们很好处理，因为客户端程序异常退出会在服务端引发ConnectionReset的Socket异常
//                 （就是WinSock2中的10054异常）。只要在服务端处理这个异常就可以了。
//             2、网络链路异常。
//                 网络链路异常使用超时处理， 如：网线拔出、交换机掉电、客户端机器掉电。当出现这些情况的时候服务端不会出现任何异常
//                 IOControlCode 枚举指定控制代码，为 Socket 设置低级操作模式。 struct tcp_keepalive
//                 {
//                     u_long onoff; //是否启用Keep-Alive
//                     u_long keepalivetime; //多长时间后开始第一次探测（单位：毫秒）
//                     u_long keepaliveinterval; //探测时间间隔（单位：毫秒）
//                 };

//            try
//            {
//                uint dummy = 0;
//                byte[] inOptionValues = new byte[Marshal.SizeOf(dummy) * 3];
//                BitConverter.GetBytes((uint)1).CopyTo(inOptionValues, 0);
//                BitConverter.GetBytes((uint)5000).CopyTo(inOptionValues, Marshal.SizeOf(dummy));
//                BitConverter.GetBytes((uint)5000).CopyTo(inOptionValues, Marshal.SizeOf(dummy) * 2);
//                socket.IOControl(IOControlCode.KeepAliveValues, inOptionValues, null);
//            }
//            catch (Exception e)
//            {
//                BaseConnector.Log("SetKeepALive fail: e=" + e);
//            }
//			*/
//        }

//        /// <summary>
//        /// 处理链接...
//        /// </summary>
//        /// <param name="socket"></param>
//        /// <param name="state"></param>
//        /// <param name="e"></param>
//        protected void ProcessConnect(Socket socket, object state, SocketAsyncEventArgs e)
//        {
//            if (e.SocketError == SocketError.WouldBlock)
//            {
//                LogError("ProcessConnect SocketException=" + e.SocketError.ToString());//+ "," + this.mType
//                //				return;
//            }

//            if (e != null && e.SocketError != SocketError.Success)
//            {
//                LogError(new SocketException((int)e.SocketError));
//                OnTimeOut();
//                return;
//            }

//            if (socket == null)
//            {
//                LogError(new SocketException((int)SocketError.ConnectionAborted));
//                OnTimeOut();
//                return;
//            }
//            if (e == null)
//                e = new SocketAsyncEventArgs();


//            m_Status = SOCKET_STATUS.CONNECTED;
//            if (Buffer.Array == null)
//                Buffer = new ArraySegment<byte>(new byte[maxReceiveBuffLength], 0, maxReceiveBuffLength);
//            e.SetBuffer(Buffer.Array, Buffer.Offset, Buffer.Count);

//            SetKeepALive(socket);

//            OnConnected();
//            StartReceive();
//        }

//        /// <summary>
//        /// 
//        /// </summary>
//        void StartReceive()
//        {
//            bool raiseEvent;


//            if (_clientSock == null)
//                return;

//            try
//            {
//                raiseEvent = _clientSock.ReceiveAsync(mRevEvent);
//            }
//            catch (SocketException exc)
//            {
//                LogError("StartReceive::SocketException " + exc.ErrorCode);
//                return;
//            }
//            catch (Exception e)
//            {
//                m_Status = SOCKET_STATUS.CLOSING;
//                if (e is System.ObjectDisposedException)
//                {
//                    BaseConnector.Log("StartReceive0:" + e.Message);
//                }
//                else if (e is NullReferenceException)
//                {
//                    BaseConnector.Log("StartReceive1:" + e.Message);
//                }
//                else
//                {
//                    //#if !WIN_CLIENT
//                    //                    RemoteManager.GetInstance().DelayDisposeSocket(this.m_Mgr);
//                    //#endif
//                }
//                OnTimeOut();
//                return;
//            }
//            //BaseConnector.Log("收到数据了，，，，，，，" + raiseEvent);
//            if (!raiseEvent)
//                ProcessReceive(mRevEvent);
//        }


//        /// <summary>
//        /// 处理异步事件:
//        /// </summary>
//        /// <param name="e"></param>
//        private void ProcessReceive(SocketAsyncEventArgs e)
//        {
//            if (e.SocketError == SocketError.WouldBlock)
//            {
//                LogError("ProcessReceive:1SocketException=" + e.SocketError.ToString());
//            }

//            if (e.SocketError != SocketError.Success)
//            {
//                //SocketException=Interrupted,GAME_SOCKET
//                LogError("ProcessReceive:2SocketException=" + e.SocketError.ToString() );
//                m_Status = SOCKET_STATUS.CLOSING;
//                OnTimeOut();
//                return;
//            }
//            if (e.BytesTransferred == 0)
//            {
//                LogError("ProcessReceive:BytesTransferred == 0");
//                m_Status = SOCKET_STATUS.CLOSING;
//                OnTimeOut();
//                return;
//            }

//            OnDataReceived(e.Buffer, e.Offset, e.BytesTransferred);
//            StartReceive();
//        }


//        #region sendmessage
//        /// <summary>
//        /// send tcp message to server 
//        /// </summary>
//        /// <param name="obj"></param>
//        /// 
//        public override bool Send(byte[] buffer, int offset, int size)
//        {
//#if ENABLE_PROFILER
//            //BaseConnector.Log("Send :" + msgSend.mCommand + "/" + msgSend.mSubCommand + "_clientSock=" + _clientSock.Connected);
//#endif
//            if (_clientSock == null)
//            {
//                return false;
//            }
//            if (_clientSock.Connected == false)
//            {
//                Dispose();
//                return false;
//            }

//            //zzNote: tcp在ios和editor模式可能使用了异步模式，这里不能使用static内存...
//            //byte[] mBuffer = msgSend.packge(null, 0);

//            //#if !WIN_CLIENT
//            //            if (RemoteManager.GetInstance().needSavePack)
//            //                ServerPackInfo.GetInstance().AddPackage(msgSend);
//            //#if LOG_NET
//            //            ServerPackInfo.GetInstance().AddMsgRaw(2, mBuffer, msgSend.mPakLen);
//            //#endif
//            //#endif

//            //int Send(byte[] buf, int offset, int size, SocketFlags flags);

//            //IAsyncResult asyncSend0 = _clientSock.BeginSend(msgSend.buffer, 0, msgSend.buffer.Length, SocketFlags.None, new AsyncCallback(sendCallback), _clientSock);
//            IAsyncResult res = _clientSock.BeginSend(buffer, offset, size, SocketFlags.None, m_asyncCB, _clientSock);//new AsyncCallback(sendCallback)
//            //监测超时
//            //bool success0 = asyncSend0.AsyncWaitHandle.WaitOne(m_TimeoutDuration, true);
//            //if (!success0)
//            //{
//            //    //_clientSock.Close();
//            //    Dispose();
//            //    BaseConnector.Log("Time Out !");
//            //    return false;
//            //}

//            return true;
//        }
//        #endregion

//        #region Isconneted

//        /// <summary>
//        /// tcp connect status
//        /// </summary>
//        /// <returns></returns>
//        public override bool Isconnected()
//        {
//            if (_clientSock == null)
//            {
//                return false;
//            }
//            return _clientSock.Connected;

//        }

//        #endregion
//        /// <summary>
//        ///tcp send callback
//        /// </summary>
//        /// <param name="asyncSend"></param>
//        private void sendCallback(IAsyncResult asyncSend)
//        {
//            // 从state对象中获取socket  
//            Socket client = (Socket)asyncSend.AsyncState;
//            // 完成数据发送.  
//            int bytesSent = client.EndSend(asyncSend);

//            if (bytesSent == 0)
//            {
//                LogError("Send err bytesSent == 0!");
//                //if (e.SocketError != SocketError.WouldBlock)
//                {
//                    Dispose();
//                }

//                return;
//            }
//            //isSending = false;

//            if (mNeedSendMsg.Count > 0)
//            {
//                // Send(mNeedSendMsg[0]);
//                mNeedSendMsg.RemoveAt(0);
//            }
//        }

//        //
//        // Summary:
//        //     
//        public override void Close()
//        {
//            if (_clientSock != null)
//            {
//                //fix: 如果当前状态为CONNECTED，也需要往上抛出
//                if (_clientSock.Connected || (m_Status == SOCKET_STATUS.CONNECTED))
//                {
//                    try
//                    {
//                        //Disables sends and receives on a System.Net.Sockets.Socket.
//                        //_clientSock.Shutdown(SocketShutdown.Both);
//                        // Closes the socket connection and allows reuse of the socket.
//                        _clientSock.Disconnect(false); //
//                    }
//                    catch (Exception e)
//                    {
//                        BaseConnector.Log("Close:e=" + e);
//                    }
//                    OnClosed();
//                }
//            }
//        }

//        //关闭发送和接收
//        // Summary:
//        public override void Shutdown()
//        {
//            Close();
//            if (_clientSock != null)
//            {
//                //_clientSock.Shutdown(SocketShutdown.Both);
//                try
//                {
//                    //     Closes the System.Net.Sockets.Socket connection and releases all associated resources.
//                    _clientSock.Close();
//                    //_clientSock.Dispose();
//                }
//                catch (Exception e)
//                {
//                    BaseConnector.Log("Shutdown:e=" + e);
//                }
//            }
//        }

//        public override void Dispose()
//        {
//            if (_clientSock != null)
//            {
//                Close();
//                Shutdown();
//                if (mRevEvent != null)
//                {
//                    mRevEvent.Dispose();
//                    mRevEvent = null;
//                }
//                _clientSock = null;
//            }
//            m_asyncCB = null;
//            base.Dispose();
//        }

//        //Socket客户端对象
//        private Socket _clientSock;

//        //private bool isSending;

//        protected ArraySegment<byte> Buffer { get; set; }
//        private SocketAsyncEventArgs mRevEvent;

//    }
//}
