namespace MobaGo.Network
{
    //public interface IMHInitializer
    //{
    //    IConnector CreateConnector();
    //    IMonitor[] CreateMonitors();
    //    IConnector RenewConnector(IConnector currentConn);
    //}

  //  public interface IMessageHandler
  //  {
  //      int pingTime
		//{
		//	get;
  //      }

  //      bool fastConnectionLossChk
  //      {
  //          get;
  //      }

		//IConnector conn { get; }
        
  //      void SetKeepAliveInterval(float interval);
  //      void Connect(string ip, int port, float KAInterval = 1f, string origin = "", string protocol = "", int uid = 0, byte[] token = null);
  //      void Update();
  //      void LateUpdate();
  //      void CloseConnector(bool shutdown,string reason);
  //      void Reconnect(string reason);
        
  //      bool IsConnected ();

  //      void AddDelegateOnConnected(VoidCallback cb);
  //      void AddDelegateOnClosed(VoidCallback cb);
  //      void AddDelegateOnTimeout(VoidCallback cb);
  //      void AddDelegateOnReachabilityChange(NetworkMsgErrorCallback cb);

  //      void RemoveDelegateOnConnected(VoidCallback cb);
  //      void RemoveDelegateOnClosed(VoidCallback cb);
  //      void RemoveDelegateOnTimeout(VoidCallback cb);
  //      void RemoveDelegateOnReachabilityChange(NetworkMsgErrorCallback cb);

  //      void AddDelegateMessageError(NetworkMsgErrorCallback cb);
  //      void RemoveDelegateMessageError(NetworkMsgErrorCallback cb);
  //      void NotifyMessageError(int errCode);

  //      IMonitor GetMonitor(int index);
  //      void RenewConnection(bool immediate = true);
  //      void RenewRawConnection(bool immediate = true);

  //      void AddPlugin<T>() where T : IMessagePlugin, new();
  //      void RemovePlugin<T>() where T : IMessagePlugin;
  //      T FindPlugin<T>() where T : IMessagePlugin;
  //  }
}
