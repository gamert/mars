#pragma once
#include "kcpclient_task.h"
#include "INetSession.h"

//
class CKcpSession : public kcpclient, public INetSession
{
public:
	//
	virtual bool Connect(const char* ip, unsigned short port, unsigned uuid)
	{
		return kcpclient::connect(ip, port, uuid);
	}

	//for 
	virtual void CloseConnect()
	{

	}

	virtual void* ReceiveCycle2(int *result)
	{
		return NULL;
	}

	///
	virtual int Send(const char *data, const int length, int sendType)
	{
		if (sendType == 1)
		{
			return this->sendUdp(data, length);
		}
		return this->sendtcp(data, length);
	}

	virtual int GetConnectionState()
	{
		return connect_state;
	}

	virtual int GetAveragePing()
	{
		return 1000;
	}
};

INetSession *CreateSessionKCP()
{
	return new CKcpSession();
}
