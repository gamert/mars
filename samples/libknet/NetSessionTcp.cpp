#pragma once

#include "mars/comm/autobuffer.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/bootrun.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/dns/dns.h"

#include "mars/comm/socket/tcpclient.h"
#include "INetSession.h"


#define LOG_DETAIL 1

#ifdef LOG_DETAIL
#define LLOG printf
#else
#define LLOG(a) (0)
#endif

//
class CTcpSession : public INetSession , public MTcpEvent
{
protected:
	TcpClient *tc;

public:
	CTcpSession():tc(NULL)
	{
	}
	virtual ~CTcpSession()
	{
		if (tc)
		{
			delete tc;
		}
	}

	//连接...
	virtual bool Connect(const char* ip, unsigned short port, unsigned uuid)
	{
		//检测ip的合法性...
		//DNS 解析?

		tc = new TcpClient(ip, port, *this);
		return tc->Connect();
	};

	//for 
	virtual void CloseConnect()
	{
		if (tc)
		{
			tc->Disconnect();
		}
	}
	virtual void* ReceiveCycle2(int *result)
	{
		return NULL;
	}

	///
	virtual int Send(const char *data, const int length, int sendType)
	{
		if (tc)
		{
			return tc->Write(data, length);
		}
		return -1;
	}

	virtual int GetConnectionState()
	{
		if (tc)
		{
			TcpClient::TTcpStatus st = tc->GetTcpStatus();
			LLOG("%s = [%d] %s\n", __FUNCTION__, st, tcp_status_str[st]);
			return st;
		}
		return -1;
	}

	virtual int GetAveragePing()
	{
		return 1000;
	}

public:

	virtual void OnConnect()
	{
		LLOG("%s\n",__FUNCTION__);
		//tc->Write("abcd", 4);
		if (m_OnConnect)
		{
			m_OnConnect(m_UserPtr);
		}
	};

	virtual void OnDisConnect(bool _isremote)
	{
		LLOG("%s (%d)\n", __FUNCTION__, _isremote);
		if (m_OnDisConnect)
		{
			m_OnDisConnect(m_UserPtr, _isremote);
		}
	};

	virtual void OnError(int _status, int _errcode)
	{
		LLOG("%s(%d,%d)\n", __FUNCTION__, _status, _errcode);
		if (m_OnError)
		{
			m_OnError(m_UserPtr, _status, _errcode);
		}
	};

	virtual void OnWrote(int _id, unsigned int _len)
	{
		LLOG("%s(%d,%d)\n", __FUNCTION__, _id, _len);
	};

	virtual void OnAllWrote()
	{
		LLOG("%s\n", __FUNCTION__);
	};

	virtual void OnRead()
	{
		const int R_LEN = 16 * 1024;
		BYTE buf[R_LEN+4];
		unsigned int _len = tc->Read(buf, sizeof(buf));
		LLOG("%s = %d\n", __FUNCTION__, _len);
		if (m_OnData)
		{
			m_OnData(m_UserPtr, buf, _len);
		}
	};
};

//
INetSession *CreateSessionTCP()
{
	return new CTcpSession();
}
