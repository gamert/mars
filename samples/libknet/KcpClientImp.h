#pragma once
#include "kcpclient_task.h"


class CKcpClientImp : public kcpclient
{
public:
	//
	//int Connect(const char* ip, unsigned short port, unsigned uuid)
	//{
	//	bool res = this->connect();
	//	return 0;
	//}
	void CloseConnect()
	{

	}
	void* ReceiveCycle2(int *result)
	{
		return NULL;
	}
	int Send(const char *data, const int length, int sendType)
	{
		if (sendType == 1)
		{
			return this->sendUdp(data, length);
		}
		return this->sendtcp(data, length);
	}

	int GetConnectionState()
	{

		return -1;
	}

	int GetAveragePing()
	{
		return 1000;
	}


};
