///////////////////////////////////////////////////////////////////////////////
//
//  Module: NetRpt.h
//
//    Desc: Defines the interface for the NetRpt.DLL.
//
// Copyright (c) 2007 Txz
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _NETRPTAPI_H_
#define _NETRPTAPI_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


// NetRpt.h
#ifdef NETRPTDLL
#define NETRPTAPI extern "C" __declspec(dllexport)
#else
#define NETRPTAPI
#endif


//½ÓÊÕ
typedef int ( * lpfnDNetRecv) (int ,const char *,int,int);
//·¢ËÍ
typedef int ( * lpfnDNetSend) (int ,const char *,int,int); 
//
typedef int ( * lpfnDNetShow) (int); 


/*
	SOCKET index
			DNetRecv(0,SocketInfo->Buffer,RecvBytes,0);

*/
NETRPTAPI int DNetRecv(int index,const char *szDataBuffer,int nDataLength,int reserve);
NETRPTAPI int DNetSend(int index,const char *szDataBuffer,int nDataLength,int reserve);
NETRPTAPI int DNetShow(int nShow);


#endif
