
#pragma once

#include <stdint.h>

//类型定义.
typedef uint32_t OBJ_T;	//
typedef uint32_t uint;
typedef void*	IntPtr;

#define MAX_TCP_BUFF (64*1024)

//(异步)回调...
typedef void(*NS_OnConnect)(int userPtr);
typedef void(*NS_OnDisConnect)(int userPtr,int _isremote);
typedef void(*NS_OnError)(int userPtr, int _status, int _errcode);
typedef void(*NS_OnData)(int userPtr, unsigned char* buf, int len);

const static char *tcp_status_str[] = {
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


//a typed net session for cs:
class INetSession
{
protected:
	NS_OnConnect m_OnConnect;
	NS_OnDisConnect m_OnDisConnect;
	NS_OnError m_OnError;
	NS_OnData m_OnData;

	int		m_UserPtr;
public:
	INetSession()
	{
		m_OnConnect = NULL;
		m_OnDisConnect = NULL;
		m_OnError = NULL;
		m_OnData = NULL;
		m_UserPtr = -1;
	}

	virtual ~INetSession()
	{
	}

	//for tcp?
	virtual void SetCallBack(int userPtr,NS_OnConnect _OnConnect, NS_OnDisConnect _OnDisConnect, NS_OnError _OnError, NS_OnData _OnData)
	{
		m_UserPtr = userPtr;
		m_OnConnect = _OnConnect;
		m_OnDisConnect = _OnDisConnect;
		m_OnError = _OnError;
		m_OnData = _OnData;
	};

	//连接...
	virtual bool Connect(const char* ip, unsigned short port, unsigned uuid) = 0;
	//关闭连接...
	virtual void CloseConnect() = 0;
	//
	virtual void* ReceiveCycle2(int *result) = 0;
	
	/*不同的实现有不同的发送类型:
	**sendType: 0 tcp,1 kcp?
	*/
	virtual int Send(const char *data, const int length, int sendType) = 0;
	//
	virtual int GetConnectionState() = 0;
	//
	virtual int GetAveragePing() = 0;
};

INetSession *CreateSessionKCP();
INetSession *CreateSessionTCP();
