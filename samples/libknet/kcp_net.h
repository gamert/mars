#ifndef KCP_NET_H
#define KCP_NET_H

#pragma once

#include <stdint.h>

#ifdef _WIN32

#define STDCALL __stdcall
#else
#define STDCALL
#endif

#ifdef _WIN32
#define T_DLL	__declspec(dllexport)
#else
#define T_DLL	extern 
#endif

//CallingConvention = CallingConvention.Cdecl)


//
typedef uint32_t OBJ_T;	//
typedef uint32_t uint;
typedef void*	IntPtr;


typedef void(*NettySendMessageDelegate)(OBJ_T objectId, const char *function, const char *param);
typedef void(*NettySendStructDelegate)(OBJ_T objectId, const char * function, IntPtr param);
typedef void(*NettySendResultDelegate)(OBJ_T objectId, const char * function, int result);
typedef void(*NettySendBufferDelegate)(OBJ_T objectId, const char * function, IntPtr buffer, int size);
typedef void(*NettySendResultBufferDelegate)(OBJ_T objectId, const char * function, int result, IntPtr buffer, int size);



//ƽ̨����:
enum class kPlatform
{
	None = 0,

};

enum class kClientType
{
	None,
	Android,
	IOS,
	PC
};

//����type
enum class kNetResult
{
	Success = 0,
	Error,
	NetworkException,
	Timeout,
	InvalidArgument,
	LengthError,
	Unknown,
	Empty,
};



extern "C"
{
	//��ʼ��:
	T_DLL void STDCALL Net_Initialize(const char *dataPath, const char *persistentDataPath);
	//�˳�������ϵͳ
	T_DLL void STDCALL Net_Uninitialize();
	
	/*
		����Peer,typeΪudp|tcp|kcp
	*/
	T_DLL IntPtr STDCALL Net_CreateSession(const char *type);
	T_DLL void STDCALL Net_ReleaseSession(IntPtr handle);

	/*
		����,-1ʧ��,0 �ɹ�
	*/
	T_DLL int STDCALL Net_Connect(IntPtr handle, const char* host, unsigned short port, uint uuid);
	/*
		�ر����ӣ������٣�
	*/
	T_DLL void STDCALL Net_CloseConnect(IntPtr handle);

	/*
	�������ݣ��Էָ��ϱ��ĵ���ʽ�����ض�����ģ�����call
	*/
	T_DLL void* STDCALL Net_ReceiveCycle2(IntPtr handle, int *result);

	/*
	�������ݣ������ֽ���; -1��ʾ����ʧ��...
	*/
	T_DLL int STDCALL Net_Send(IntPtr handle, const char *data, const int length, int sendType);

	/*
	ȡ������״̬
	*/
	T_DLL int STDCALL Net_GetConnectionState(IntPtr handle);

	/*
	ȡ��Ping��ms
	*/
	T_DLL int STDCALL Net_GetAveragePing(IntPtr handle);

	/*
	ȡ�õ�ǰʱ�̣�us
	*/
	T_DLL uint64_t STDCALL Net_GetTimeUs();

	//helper
	T_DLL void STDCALL MemCopy(void *dst, void *src, int size);
	//T_DLL int STDCALL Compress(Bytef *dest, uLongf *destLen,const Bytef *source, uint64_t sourceLen);
	//T_DLL int STDCALL UnCompress(Bytef *dest, uLongf *destLen, const Bytef *source, uint64_t sourceLen);
	T_DLL void STDCALL Net_xLog(int level, const char* host);
	T_DLL void STDCALL Net_xLogFlush(int param);

}

#endif