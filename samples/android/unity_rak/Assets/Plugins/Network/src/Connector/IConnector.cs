using System;

namespace Comm.Network
{
    public enum EConnectorState
    {
		E_CON_INVALID_PARAMETER = -1,
        E_CON_ST_CLOSED = 0,
        E_CON_ST_CLOSING,
        E_CON_ST_CONNECTING,
        E_CON_ST_CONNECTED
    }    

    public class AddressInfo
    {
        private string _host = "";
        private int _port = 0;
        private string _origin = "";
        private string _protocol = "";

        public string ip { get { return _host; } }
        public int port { get { return _port; } }
        public string origin { get { return _origin; } }
        public string protocol { get { return _protocol; } }

        public AddressInfo(string ip, int port, string origin, string protocol)
        {
            _host = ip;
            _port = port;
            _origin = origin;
            _protocol = protocol;
        }
    }
}
