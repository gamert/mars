#ifndef NET_H
#define NET_H

//#include "plat/basetypes.h"

#include "plat/platform.h"

//-----------------------------------------------------------------------------
// dll export stuff
//-----------------------------------------------------------------------------
#ifdef NET_DLL_EXPORT
#define NET_INTERFACE	DLL_EXPORT
#else
#define NET_INTERFACE	DLL_IMPORT
#endif



class INetEvent {
public:
	virtual ~INetEvent() {}

	virtual void OnConnect() = 0;
	virtual void OnDisConnect(bool _isremote) = 0;
	virtual void OnError(int _status, int _errcode) = 0;

	virtual void OnWrote(int _id, unsigned int _len) = 0;
	virtual void OnAllWrote() = 0;

	virtual void OnRead() = 0;
	virtual void NET_Package(int cmd,const char *pMsg, int len) = 0;
};


class INetChannel
{
public:
	virtual ~INetChannel(){}
	//@return:
	virtual int		Connect(const char *szIP, int port, INetEvent *p) = 0;
	//@return:
	virtual int		IsConnectTo(const char *szIP, int port) = 0;
	//@return:
	virtual void	Close() = 0;
	//@return: writedbufid_
	virtual int Send(const char *sendMsg, int sendLen, int nEncrytionParam) = 0;
	//
	virtual const char *GetIP() = 0;
	virtual int GetPort() = 0;
};

class INet
{
public:
	virtual void Shutdown() = 0;

	virtual INetChannel *Create() = 0;
	virtual void Delete(INetChannel *) = 0;
};

NET_INTERFACE INet	*GetINet();

#endif