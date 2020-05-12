#ifndef NETCHANNEL_T
#define NETCHANNEL_T

#include "plat/dbg.h"

#include "comm/autobuffer.h"
//#include "comm/xlogger/xlogger.h"
//#include "comm/bootrun.h"
//#include "comm/thread/mutex.h"
//#include "comm/thread/lock.h"
//#include "comm/thread/thread.h"
//#include "comm/time_utils.h"
//#include "comm/dns/dns.h"
#include "comm/socket/tcpclient.h"

#ifdef USE_NetCrypt
	#include "NetCrypt.h"
#endif
#include "BaseChannel.h"
#include "Net.h"

#define MAX_SOCKETS       			1                //同时连接的套接字数
#define MAXSERVER		  			1
#define SOCK_ERROR_CONNECTFAIL		0x0002
#define SOCK_ERROR_DISCONNECTED		0x0003



// WM_SOCKET_NOTIFY and WM_SOCKET_DEAD are used internally by MFC's
// Windows sockets implementation.  For more information, see sockcore.cpp

#pragma pack(1)
/// Client Packet Header
struct ClientPktHeader
{
    uint16 size;
    uint16 cmd;
};

/// Server Packet Header
struct ServerPktHeader
{
    uint16 size;
    uint16 cmd;
    uint16 cmd2;
};
#pragma pack()

const size_t HEADERSIZE = sizeof(ServerPktHeader);

class Channel_t :public CBaseChannel,public INetChannel,public MTcpEvent
{
public:
	Channel_t();
	~Channel_t();
public:
	//INetChannel
	virtual int		Connect(const char *szIP,int port, INetEvent *p);
	virtual int		IsConnectTo(const char *szIP,int port);
	virtual void	Close();
	virtual int		Send(const char *sendMsg, int sendLen, int nEncrytionParam);
	virtual const char *GetIP();
	virtual int		GetPort();
public:
	//MTcpEvent
	virtual void OnConnect();
	virtual void OnDisConnect(bool _isremote);
	virtual void OnError(int _status, int _errcode);

	virtual void OnWrote(int _id, unsigned int _len);
	virtual void OnAllWrote();

	virtual void OnRead();
protected:

	void	ProcessIncoming();
	int		ProcessOutgoingPackage(const char * szPackuf, uint32 len);
	int		PurgeOutbuf();
	int		Read();
	void	OnPackage(char *pPack, int nPackLen);

protected:
	INetEvent *m_pINetEvent;
	TcpClient *m_TcpClient;
	ServerPktHeader m_hdr;
	uint16	m_remaining;
	uint16	m_subCmd;
	uint32	m_cmd;
#ifdef USE_NetCrypt
	CNetCrypt _crypt;
#endif

	Mutex m_mutex_;
};


//外部--
//void NET_Package(int cmd, const char *pMsg, int len);
//void NET_Error(int cmd, const char *pMsg, int len);


#endif