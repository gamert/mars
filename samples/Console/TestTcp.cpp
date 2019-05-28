#include "stdlib.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/bootrun.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/dns/dns.h"

#include "mars/comm/socket/tcpclient.h"
#include "mars/comm/socket/tcpserver.h"

#define USE_LOOP_SERVER 1

static void __GetIP() {
	xverbose_function();
}

class MTcpServerImp:public MTcpServer {
public:
	virtual void OnCreate(TcpServer* _server)
	{
		printf("MTcpServerImp::OnCreate()\n");
	};
	virtual void OnAccept(TcpServer* _server, SOCKET _sock, const sockaddr_in& _addr)
	{
		printf("MTcpServerImp::OnAccept(%d)\n", _sock);
		int send_len = (int)send(_sock, (char*)"1234", 4, 0);
	};
	virtual void OnError(TcpServer* _server, int _error)
	{
		printf("MTcpServerImp::OnError(%d)\n", _error);
	};
};

const char *tcp_status_str[] = {
	"kTcpInit",
	"kTcpInitErr",
	"kSocketThreadStart",
	"kSocketThreadStartErr",
	"kTcpConnecting",
	"kTcpConnectIpErr",
	"kTcpConnectingErr",
	"kTcpConnectTimeoutErr",
	"kTcpConnected",
	"kTcpIOErr",
	"kTcpDisConnectedbyRemote",
	"kTcpDisConnected", };

class CTcpEvent :public MTcpEvent
{
public:
	TcpClient *tc;

	void Loop(const char *ip, int _port)
	{
		tc = new TcpClient(ip, _port, *this);
		tc->Connect();

		TcpClient::TTcpStatus status = tc->GetTcpStatus();
		uint64_t begin = gettickcount();
		uint64_t end;
		int seconds = 0;
		do
		{
			::sleep(1);
			end = gettickcount();
			status = tc->GetTcpStatus();
			printf("main::status = %s\n", tcp_status_str[status]);

			seconds++;
			if (seconds % 5 == 1)
			{
				tc->Write("abcd", 4);
			}
		} while (end - begin < 60000 || status == TcpClient::kTcpDisConnected);
	}

	virtual void OnConnect()
	{
		printf("CTcpEvent::OnConnect\n");

		tc->Write("abcd",4);
	};
	virtual void OnDisConnect(bool _isremote) 
	{
		printf("CTcpEvent::OnDisConnect (%d)\n", _isremote);
	};
	virtual void OnError(int _status, int _errcode) 
	{
		printf("CTcpEvent::OnError(%d,%d)\n", _status,  _errcode);
	};

	virtual void OnWrote(int _id, unsigned int _len) 
	{
		printf("CTcpEvent::OnWrote(%d,%d)\n", _id, _len);
	};
	virtual void OnAllWrote() 
	{
		printf("CTcpEvent::OnAllWrote\n");
	};

	virtual void OnRead() 
	{
		char buf[128];
		unsigned int _len = tc->Read(buf, sizeof(buf));
		printf("CTcpEvent::OnRead = %d\n", _len);
	};
};

static void InitSocket()
{

	WSADATA Data;
	SOCKADDR_IN serverSockAddr;
	SOCKADDR_IN clientSockAddr;
	SOCKET serverSocket;
	SOCKET clientSocket;
	int addrLen = sizeof(SOCKADDR_IN);
	int status;
	int numrcv;
	char buffer[256];

	/* initialize the Windows Socket DLL */
	status = WSAStartup(MAKEWORD(1, 1), &Data);
	if (status != 0)
	{
	}
}

void tcp_main(int argc, char **argv)
{
	InitSocket();

	AutoBuffer ab(1000);

	std::string _host_name = "www.baidu.com";

	//static DNS s_dns;
	//std::vector<std::string> ips;
	//s_dns.GetHostByName(_host_name, ips);
	const char *ip = "192.168.82.19";//

	//const char *ip = "192.168.82.201";//127.0.0.1
	//uint16_t _port = 6620;
	//const char *ip = "192.168.85.11";//127.0.0.1
	uint16_t _port = 8001;
	//timeMs();
	//xdebug2(TSF"curtime:%_ , @%_", gettickcount(), &s_dns);
#if USE_LOOP_SERVER
	MTcpServerImp *tsi = new MTcpServerImp();
	TcpServer *ts = new TcpServer(ip, _port, *tsi);

	bool _newone = 0;
	ts->StartAndWait(&_newone);

	uint64_t begin = gettickcount();
	uint64_t end;
	do
	{
		end = gettickcount();
	} while (end - begin < 5000 || ts->Socket()== INVALID_SOCKET);
#endif

	//Thread thread(&__GetIP, _host_name.c_str());
	//int startRet = thread.start();
	CTcpEvent *event = new CTcpEvent();
	event->Loop(ip, _port);


	printf("main::end\n");

	exit(0);
}