


////zzAdd: TCP support
//
//#include "RakPeerInterface.h"
//#include "NetConnector.h"
//
//#ifdef _WIN32
//
//#ifdef _WIN32
//#pragma warning(disable:4786)
//#endif
//#include "../Sockets-2.3.9.9/StdoutLog.h"
//#include "../Sockets-2.3.9.9/ListenSocket.h"
//#include "../Sockets-2.3.9.9/SocketHandlerEp.h"
//#include "../Sockets-2.3.9.9/SocketHandlerThread.h"
//#include "../Sockets-2.3.9.9/TcpSocket.h"
//#include "../Sockets-2.3.9.9/Utility.h"
//#ifndef _WIN32
//#include <signal.h>
//#include <stdint.h>
//#else
//typedef __int64 int64_t;
//#endif
//#include "../Sockets-2.3.9.9/HttpGetSocket.h"
//#include <iostream>
//
//#ifdef SOCKETS_NAMESPACE
//using namespace SOCKETS_NAMESPACE;
//#endif
//
//NettySendMessageDelegate m_NettySendMessageDelegate = NULL;
//NettySendStructDelegate m_NettySendStructDelegate = NULL;
//
//
//
//
//
//static  int             g_ant = 0;			//总链接个数
//static  double          g_min_time = 10000;	//s,最小链接时间；最大链接时间
//static  double          g_max_time = 0;
//static  double          g_tot_time = 0;		//s,总共链接时间
//
//static  int             g_ant2 = 0;			//收发报文延时统计
//static  double          g_min_time2 = 10000;
//static  double          g_max_time2 = 0;
//static  double          g_tot_time2 = 0;
//
//static  int64_t         gBytesIn = 0;		//总入
//static  int64_t         gBytesOut = 0;		//总出
//
//static	int		samples = 0;
//
//static  int             g_tot_ant = 0;		//
//static  double          g_tot_min_t = 0;
//static  double          g_tot_max_t = 0;
//static  double          g_tot_tot_t = 0;
//
//static  int             g_tot_ant2 = 0;
//static  double          g_tot_min_t2 = 0;
//static  double          g_tot_max_t2 = 0;
//static  double          g_tot_tot_t2 = 0;
//
//static  int64_t         tot_gBytesIn = 0;
//static  int64_t         tot_gBytesOut = 0;
//
//static  double          tot_rt = 0;
//
//static  bool            gQuit = false;
//static  size_t          g_max_connections = 0;
//static  std::string     gHost = "localhost";
//static  port_t          gPort = 2222;
//static  bool            g_b_flood = false;
//static  bool            g_b_off = false;
//static  bool            g_b_limit = false;
//static  bool            g_b_repeat = false;
//static  std::string     g_data;
//static  size_t          g_data_size = 1024;
//static  bool            g_b_stop = false;
//#ifdef HAVE_OPENSSL
//static  bool            g_b_ssl = false;
//#endif
//static  bool            g_b_instant = false;
//static  struct timeval  g_t_start;
//
//
///**
//* Return time difference between two struct timeval's, in seconds
//* \param t0 start time
//* \param t  end time
//*/
//double Diff(struct timeval t0, struct timeval t)
//{
//	t.tv_sec -= t0.tv_sec;
//	t.tv_usec -= t0.tv_usec;
//	if (t.tv_usec < 0)
//	{
//		t.tv_usec += 1000000;
//		t.tv_sec -= 1;
//	}
//	return t.tv_sec + (double)t.tv_usec / 1000000;
//}
//
////取得当前时间:
//void gettime(struct timeval *p, struct timezone *)
//{
//#ifdef _WIN32
//	FILETIME ft; // Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
//	GetSystemTimeAsFileTime(&ft);
//	uint64_t tt;
//	memcpy(&tt, &ft, sizeof(tt));
//	tt /= 10;
//	p->tv_sec = tt / 1000000;
//	p->tv_usec = tt % 1000000;
//#else
//	gettimeofday(p, NULL);
//#endif
//}
//
////打印当前统计:
//void print_tot()
//{
//	printf("c -- %6d (%.4f/%.4f/%.4f)", g_tot_ant / samples, g_tot_min_t / samples, g_tot_max_t / samples, g_tot_tot_t / g_tot_ant);
//
//	printf(" r -- %7d (%.4f/%.4f/%.4f)", g_tot_ant2 / samples, g_tot_min_t2 / samples, g_tot_max_t2 / samples, g_tot_tot_t2 / g_tot_ant2);
//
//	double mbi = (double)tot_gBytesIn / 1024 / samples;
//	mbi /= 1024;
//	mbi /= tot_rt / samples;
//	printf(" -/in%8lld", tot_gBytesIn / samples);	//11
//	printf(" (%4.2f MB/s)", mbi);	//5
//
//	double mbo = (double)tot_gBytesOut / 1024 / samples;
//	mbo /= 1024;
//	mbo /= tot_rt / samples;
//	printf(" -/out%8lld", tot_gBytesOut / samples);
//	printf(" (%4.2f MB/s)", mbo);
//
//	printf(" %4.2f s\n", tot_rt / samples);
//
//}
//
////
//void printreport()
//{
//	struct timeval tv;
//	gettime(&tv, NULL);
//	double rt = Diff(g_t_start, tv);
//	g_t_start = tv;
//	//
//	//  printf("connect %6d (%.4f/%.4f/%.4f)", g_ant,  g_min_time,  g_max_time,  g_tot_time /  g_ant);
//	g_tot_ant += g_ant;
//	g_tot_min_t += g_min_time;
//	g_tot_max_t += g_max_time;
//	g_tot_tot_t += g_tot_time;
//
//	//  printf("  reply %7d (%.4f/%.4f/%.4f)", g_ant2, g_min_time2, g_max_time2, g_tot_time2 / g_ant2);
//	g_tot_ant2 += g_ant2;
//	g_tot_min_t2 += g_min_time2;
//	g_tot_max_t2 += g_max_time2;
//	g_tot_tot_t2 += g_tot_time2;
//
//	double mbi = (double)gBytesIn / 1024;
//	mbi /= 1024;
//	mbi /= rt;
//	//  printf("  b/in %11lld",  gBytesIn);
//	//  printf(" (%5.2f MB/s)",       mbi);
//	tot_gBytesIn += gBytesIn;
//
//	double mbo = (double)gBytesOut / 1024;
//	mbo /= 1024;
//	mbo /= rt;
//	//  printf("  b/out %11lld", gBytesOut);
//	//  printf(" (%5.2f MB/s)",       mbo);
//	tot_gBytesOut += gBytesOut;
//
//	//  printf("  %5.2f s\n",         rt);
//	tot_rt += rt;
//
//	++samples;
//	print_tot();
//}
//
//
//void printreport_reset()
//{
//	printreport();
//	//
//	g_min_time = 10000;
//	g_max_time = 0;
//	g_tot_time = 0;
//	g_ant = 0;
//	g_min_time2 = 10000;
//	g_max_time2 = 0;
//	g_tot_time2 = 0;
//	g_ant2 = 0;
//	gBytesIn = gBytesOut = 0;
//}
//
//
//class NettyStringParser
//{
//public:
//
//	static std::string   replace_all_distinct(std::string str, const   std::string&   old_value, const   std::string&   new_value)
//	{
//		for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length())
//		{
//			if ((pos = str.find(old_value, pos)) != std::string::npos)
//				str.replace(pos, old_value.length(), new_value);
//			else 
//				break;
//		}
//		return   str;
//	}
//
//	static std::string EncodeString(std::string src)
//	{
//		std::string text = replace_all_distinct(src, "%", "%25");
//		text = replace_all_distinct(text,"=", "%3d");
//		return replace_all_distinct(text,"&", "%26");
//	}
//	static std::string EncodeStringQuto(std::string src)
//	{
//		std::string text = replace_all_distinct(src,",", "%2c");
//		text = replace_all_distinct(text, "%", "%25");
//		text = replace_all_distinct(text, "=", "%3d");
//		return replace_all_distinct(text,"&", "%26");
//	}
//
//};
//
//
//class MySocket : public TcpSocket
//{
//public:
//	MySocket(ISocketHandler& h, bool one) : TcpSocket(h), m_b_client(false), m_b_one(one), m_b_created(false), m_b_active(false)
//	{
//		gettime(&m_create, NULL);
//		SetLineProtocol(false);
//#ifdef HAVE_OPENSSL
//		if (g_b_ssl)
//			EnableSSL();
//#endif
//		if (g_max_connections && !m_b_one && Handler().GetCount() >= g_max_connections)
//		{
//			fprintf(stderr, "\nConnection limit reached: %d, continuing in single connection stress mode\n", (int)g_max_connections);
//			if (g_b_off)
//				printreport_reset();
//			g_b_limit = true;
//			m_b_one = true;
//			//
//			g_b_flood = g_b_repeat;
//		}
//#ifndef USE_EPOLL
//		if (!m_b_one && Handler().GetCount() >= Handler().MaxCount() - 17)
//		{
//			fprintf(stderr, "\nFD_SETSIZE connection limit reached: %d, continuing in single connection stress mode\n", (int)Handler().GetCount());
//			if (g_b_off)
//				printreport_reset();
//			g_b_limit = true;
//			m_b_one = true;
//			//
//			g_b_flood = g_b_repeat;
//		}
//#endif
//	}
//	~MySocket()
//	{
//		printf("=======MySocket::~MySocket==========\n");
//	}
//
//	//
//	virtual void OnConnect()
//	{
//		printf("=======MySocket::OnConnect==========[TID=%d]\n", GetCurrentThreadId());
//
//		gettime(&m_connect, NULL);
//		m_b_active = true;
//		{
//			double tconnect = Diff(m_create, m_connect);
//			//
//			g_min_time = tconnect < g_min_time ? tconnect : g_min_time;
//			g_max_time = tconnect > g_max_time ? tconnect : g_max_time;
//			g_tot_time += tconnect;
//			g_ant += 1;
//		}
//		//SendBlock();
//		m_b_client = true;
//		if (m_NettySendMessageDelegate)
//		{
//
//			std::string AccountInfo = "Platform=1&OpenId=2&UserId=3&Uin=4&TokenList=1,2,3,4&Pf=dw&PfKey=key&STKey=st";
//			std::string WaitingInfo = "";
//			std::string ServerInfo = "";
//
//			std::string LoginInfo = "AccountInfo=" + NettyStringParser::EncodeString(AccountInfo)
//			    + "&WaitingInfo=" + NettyStringParser::EncodeString(WaitingInfo)
//			    + "&ServerInfo=" + NettyStringParser::EncodeString(ServerInfo);
//
//			////for test:
//			std::string ss = "Result=0&LoginInfo="+ NettyStringParser::EncodeString(LoginInfo);
//			m_NettySendMessageDelegate(objectId,"OnConnectProc",ss.c_str());
//		}
//	}
//
//
//	//Called on connect timeout (5s).
//	virtual void OnConnectFailed()
//	{
//		printf("[TID=%d]MySocket::OnConnectFailed:\n", GetCurrentThreadId());
//		if (m_NettySendMessageDelegate)
//		{
//			////for test:
//			std::string ss = "Result=103&LoginInfo=";
//			m_NettySendMessageDelegate(objectId, "OnConnectProc", ss.c_str());
//		}
//
//	}
//
//	virtual void OnDisconnect(short info, int code)
//	{
//		printf("[TID=%d]MySocket::OnDisconnect:info=%d,code=%d\n", GetCurrentThreadId(), info, code);
//
//		if (m_NettySendMessageDelegate)
//		{
//			std::string ss = "Result=0";
//			m_NettySendMessageDelegate(objectId, "OnDisconnectProc", ss.c_str());
//		}
//	}
//
//	//超时:
//	virtual void OnConnectTimeout()
//	{
//		printf("[TID=%d]MySocket::OnConnectTimeout:", GetCurrentThreadId());
//
//		if (m_NettySendMessageDelegate)
//		{
//			std::string ss = "Result=3";
//			m_NettySendMessageDelegate(objectId, "OnConnectorErrorProc", ss.c_str());
//		}
//	}
//
//#ifdef ENABLE_RECONNECT
//	/** a reconnect has been made */
//	virtual void OnReconnect()
//	{
//		printf("[TID=%d]MySocket::OnReconnect:", GetCurrentThreadId());
//
//		if (m_NettySendMessageDelegate)
//		{
//			std::string ss = "Result=0";
//			m_NettySendMessageDelegate(objectId, "OnReconnectProc", ss.c_str());
//		}
//	}
//#endif
//
//	//这个仅用于测试？
//	//
//	virtual void OnRawData(const char *buf, size_t len)
//	{
//		printf("[TID=%d]MySocket::OnRawData(%s,%d)\n", GetCurrentThreadId(), buf, len);
//	}
//
//
//	virtual void OnLine(const std::string& line)
//	{
//		gettime(&m_reply, NULL);
//		m_b_active = true;
//		{
//			double treply = Diff(m_send, m_reply);
//			//
//			g_min_time2 = treply < g_min_time2 ? treply : g_min_time2;
//			g_max_time2 = treply > g_max_time2 ? treply : g_max_time2;
//			g_tot_time2 += treply;
//			g_ant2 += 1;
//		}
//		//
//		if (line != g_data)
//		{
//			fprintf(stderr, "\n%s\n%s\n", line.c_str(), g_data.c_str());
//			fprintf(stderr, "(reply did not match data - exiting)\n");
//			//      exit(-1);
//		}
//		//
//		gBytesIn += GetBytesReceived(true);
//		gBytesOut += GetBytesSent(true);
//		if (m_b_one)
//		{
//			SetCloseAndDelete();
//		}
//		else
//			if (g_b_repeat && g_b_limit)
//			{
//				SendBlock();
//			}
//		// add another
//		if (!m_b_created && (!g_b_limit || !g_b_off) && !g_b_instant)
//		{
//			MySocket *p = new MySocket(Handler(), m_b_one);
//			p->SetDeleteByHandler();
//			p->Open(gHost, gPort);
//			Handler().Add(p);
//			m_b_created = true;
//		}
//	}
//
//	void SendBlock() {
//		gettime(&m_send, NULL);
//		Send(g_data + "\n");
//	}
//
//	bool IsActive() {
//		bool b = m_b_active;
//		m_b_active = false;
//		return b;
//	}
//
//	//pszSvrUrl: tcp://192.168.11.23:8888
//	void SetSvrUrl(const char *pszSvrUrl)
//	{
//		const char *sep1 = strstr(pszSvrUrl, "://");
//		if (sep1)
//		{
//			int len1 = int(sep1 - pszSvrUrl);
//			memcpy(proxy, pszSvrUrl, len1); proxy[len1] = 0; sep1 += 3;
//			const char *sep2 = strstr(sep1, ":");
//			if (sep2)
//			{
//				int len2 = int(sep2 - sep1);
//				memcpy(ip, sep1, len2); ip[len2] = 0; sep2 += 1;
//
//				port = (uint16_t)atoi(sep2);
//			}
//		}
//		else
//		{
//		}
//	};
//
//	/** Set timeout to use for connection attempt.
//	\param x Timeout in seconds */
//	bool doConnect(uint32_t timeout)
//	{
//		printf("MySocket::doConnect: \n");
//		this->SetConnectTimeout(timeout);
//		return this->Open(ip, port);
//	}
//
//
//private:
//	char proxy[8];
//	char ip[64];
//	uint16_t port;
//
//	int	objectId = 1;	//for 测试
//
//	bool m_b_client;
//	bool m_b_one;
//	bool m_b_created;
//	bool m_b_active;
//	struct timeval m_create;
//	struct timeval m_connect;
//	struct timeval m_send;
//	struct timeval m_reply;
//};
//
//
//
//#ifndef _WIN32
//void sigint(int)
//{
//	printreport();
//	gQuit = true;
//}
//
//
//void sigusr1(int)
//{
//	g_b_flood = true;
//}
//
//
//void sigusr2(int)
//{
//	printreport_reset();
//}
//#endif
//
//
//class MyHandler : public SocketHandlerEp
//{
//public:
//	MyHandler() : SocketHandlerEp() {
//	}
//	MyHandler(StdoutLog *p) : SocketHandlerEp(p) {
//	}
//	~MyHandler() {
//	}
//	void Flood() {
//		for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
//		{
//			Socket *p0 = it->second;
//			MySocket *p = dynamic_cast<MySocket *>(p0);
//			if (p)
//			{
//				p->SendBlock();
//			}
//		}
//	}
//	void Report() {
//		int ant = 0;
//		int act = 0;
//		for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
//		{
//			MySocket *p = dynamic_cast<MySocket *>(it->second);
//			if (p)
//			{
//				ant++;
//				if (p->IsActive())
//				{
//					act++;
//				}
//			}
//		}
//		//    printf("  Number of //stress// sockets: %d  Active: %d\n", ant, act);
//	}
//};
//
////
//#define MAX_SOCKS 128
//#define TO_SOCK_ID(n) (n % MAX_SOCKS)
//
//
//
//class NetSystem
//{
//public:
//	NetSystem()
//	{
//		Init();
//	}
//
//	void Init()
//	{
//#ifndef _WIN32
//		signal(SIGINT, sigint);
//		signal(SIGUSR1, sigusr1);
//		signal(SIGUSR2, sigusr2);
//		signal(SIGPIPE, SIG_IGN);
//#endif
//		printf("NetSystem::Init(sizeof(aSockets)=%d) \n", sizeof(aSockets));
//		
//		fprintf(stderr, "Using data size: %d bytes\n", (int)g_data_size);
//		std::string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
//		while (g_data.size() < g_data_size)
//		{
//			g_data += chars[rand() % chars.size()];
//		}
//
//		memset(aSockets, 0, sizeof(aSockets));
//
//		log = enableLog ? new StdoutLog() : NULL;
//
//		m_handler = new MyHandler(log);
//		_SHThread = new SocketHandlerThread(*m_handler);
//		_SHThread->Start();	//
//
//		//for (int i = 0; i < 100; ++i)
//		//{
//		//	if (_SHThread->Handler())
//		//	{
//		//		printf("Init SHThread->Handler: ttime = %d\n",i);
//		//		break;
//		//	}
//		//	Utility::Sleep(10);
//		//}
//
//		gettime(&g_t_start, NULL);
//	}
//
//	//
//	int Initialize(OBJ_T objId, kPlatform platform, uint32_t permission, const char *pszSvrUrl)
//	{
//		printf("Initialize(%d,%s)[TID%d]\n",objId, pszSvrUrl,GetCurrentThreadId());
//
//		MySocket *s = new MySocket(*_SHThread->Handler(), one);
//		s->SetDeleteByHandler(false);
//		s->SetSvrUrl(pszSvrUrl);
//		//s->Open(gHost, gPort);
//		//_SHThread->Handler().Add(s);
//		aSockets[TO_SOCK_ID(objId)] = s;
//		return 0;
//	}
//
//	//连接:
//	kNetResult connect(OBJ_T objId, uint timeout)
//	{
//		MySocket *s = aSockets[TO_SOCK_ID(objId)];
//		assert(s != nullptr);
//		bool b = s->doConnect(timeout);
//		_SHThread->Handler()->Add(s);
//		return b ? kNetResult::Success: kNetResult::Error;
//	}
//	//tcp数据:
//	kNetResult writeData(OBJ_T objId, byte *buff, int size)
//	{
//		MySocket *s = aSockets[TO_SOCK_ID(objId)];
//		assert(s != nullptr);
//
//		//构造一个发送报文？
//		//2BLen 1Bversion 1Btag [data]
//
//		return (kNetResult)s->SendBuf((const char *)buff, size);
//	}
//
//public:
//	bool many = false;
//	bool one = false;
//	bool enableLog = false;
//	bool http = false;
//	bool connector = false;
//	std::string url;
//	time_t report_period = 10;
//
//	StdoutLog *log = nullptr;
//	MyHandler *m_handler = nullptr;
//	SocketHandlerThread *_SHThread = nullptr;
//
//	MySocket *aSockets[MAX_SOCKS];	//够了吗？
//
//};
//
////for static  init...
//NetSystem s_NetSystem;
//
//
//
//extern "C"
//{
//
//	
//	//网络初始化:
//	//@objId: c#指定一个唯一ID，用于控制c端
//	//@pszSvrUrl: tcp://192.168.11.23:8888
//	T_DLL int netty_Initialize(OBJ_T objId, kPlatform platform, uint permission, const char *pszSvrUrl)
//	{
//		return s_NetSystem.Initialize(objId,  platform,  permission, pszSvrUrl);
//	}
//
//	//设置安全信息:
//	T_DLL kNetResult netty_setSecurityInfo(OBJ_T objId, kEncryptMethod encyptMethod, kKeyMaking keyMakingMethod, const char *pszDHP)
//	{
//		return kNetResult::Success;
//	}
//
//	//设置路由信息
//	T_DLL kNetResult netty_setRouteInfo(OBJ_T objId, byte *buff, int size)
//	{
//		return kNetResult::Success;
//	}
//
//	//连接:
//	T_DLL kNetResult netty_connect(OBJ_T objId, uint timeout)
//	{
//		return s_NetSystem.connect(objId, timeout);
//	}
//
//	//重连:
//	T_DLL kNetResult netty_reconnect(OBJ_T objId, uint timeout)
//	{
//		return kNetResult::Success;
//	}
//
//	//主动断开
//	T_DLL kNetResult netty_disconnect(OBJ_T objId)
//	{
//		return kNetResult::Success;
//	}
//
//	//写TCP数据
//	T_DLL kNetResult netty_writeData(OBJ_T objId, byte *buff, int size)
//	{
//		return s_NetSystem.writeData(objId, buff, size);
//	}
//
//	//写UDP数据
//	T_DLL kNetResult netty_writeUdpData(OBJ_T objId, byte *buff, int size)
//	{
//		return kNetResult::Success;
//	}
//
//	//带路由写
//	T_DLL kNetResult netty_writeData_with_route_info(OBJ_T objId, byte *buff, int size, byte *routeData, int routeDataLen, bool allowLost)
//	{
//		return kNetResult::Success;
//	}
//
//	//读TCP数据
//	T_DLL kNetResult netty_readData(OBJ_T objId, byte *buff, int *size)
//	{
//		return kNetResult::Success;
//	}
//
//	//读UDP数据
//	T_DLL kNetResult netty_readUdpData(OBJ_T objId, byte *buff, int *size)
//	{
//		return kNetResult::Success;
//	}
//
//	//
//	T_DLL kNetResult netty_getstopreason(OBJ_T objId, int *result, int *reason, int *excode)
//	{
//		return kNetResult::Success;
//	}
//
//	
//	T_DLL kNetResult netty_report_accesstoken(OBJ_T objId, const char *atk, uint expire)
//	{
//		return kNetResult::Success;
//	}
//
//	//
//	T_DLL kNetResult netty_set_clientType(OBJ_T objId, kClientType type)
//	{
//		return kNetResult::Success;
//	}
//
//	//设置协议版本:
//	T_DLL kNetResult netty_set_protocol_version(OBJ_T objId, int headVersion, int bodyVersion)
//	{
//		return kNetResult::Success;
//	}
//
//
//	//
//	T_DLL void addNettyObject(OBJ_T objectId, const char *objName)
//	{
//		printf("addNettyObject:objectId=%d,objName=%s\n", objectId, objName);
//	};
//
//	T_DLL void removeNettyObject(OBJ_T objectId)
//	{
//		printf("removeNettyObject:objectId=%d\n", objectId);
//	};
//
//
//	T_DLL void setNettySendMessageCallback( NettySendMessageDelegate callback)
//	{
//		printf("setNettySendMessageCallback:callback=%x\n", (uint32_t)callback);
//		m_NettySendMessageDelegate = callback;
//	};
//	T_DLL void setNettySendStructCallback( NettySendStructDelegate callback)
//	{
//		printf("setNettySendStructCallback:callback=%x\n", (uint32_t)callback);
//		m_NettySendStructDelegate = callback;
//	};
//
//	T_DLL void setNettySendResultCallback( NettySendResultDelegate callback)
//	{
//		printf("setNettySendResultCallback:callback=%x\n", (uint32_t)callback);
//	};
//
//	T_DLL void setNettySendBufferCallback( NettySendBufferDelegate callback)
//	{
//		printf("setNettySendBufferCallback:callback=%x\n", (uint32_t)callback);
//	};
//
//	T_DLL void setNettySendResultBufferCallback( NettySendResultBufferDelegate callback)
//	{
//		printf("setNettySendResultBufferCallback:callback=%x\n", (uint32_t)callback);
//	};
//
//	T_DLL void Netty_quit()
//	{
//		printf("Netty_quit:\n");
//	};
//}
//#endif
