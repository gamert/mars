namespace Comm.Network
{
    public delegate void VoidCallback();
    public delegate void ObjectCallback(object obj);
    public delegate void NetworkMsgErrorCallback (int errCode);

    public enum EMonitorType
    {
        E_MT_DISCONNECT = 0,
        E_MT_PING,
        E_MT_MSG_ERROR,

        num
    }
}