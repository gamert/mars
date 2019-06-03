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



//平台定义:
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

//错误type
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
	//初始化:
	T_DLL void STDCALL _std_initialize(const char *dataPath, const char *persistentDataPath);
	//退出、清理系统
	T_DLL void STDCALL _std_uninitialize();
	
	/*
		创建Peer,type为udp|tcp|kcp
	*/
	T_DLL IntPtr STDCALL _std_create_session(const char *type);
	T_DLL void STDCALL _std_release_session(IntPtr handle);

	/*
		连接,-1失败,0 成功
	*/
	T_DLL int STDCALL _std_connect(IntPtr handle, const char* host, unsigned short port, uint uuid);
	/*
		关闭连接（不销毁）
	*/
	T_DLL void STDCALL _std_close_connect(IntPtr handle);

	/*
	接收数据，以分割、组合报文的形式，返回多个报文，降低call
	*/
	T_DLL void* STDCALL _std_receive_cycle(IntPtr handle, int *result);

	/*
	发送数据，返回字节数; -1表示发送失败...
	*/
	T_DLL int STDCALL _std_send(IntPtr handle, const char *data, const int length, int sendType);

	/*
	取得连接状态
	*/
	T_DLL int STDCALL _std_get_connection_state(IntPtr handle);

	/*
	取得Ping，ms
	*/
	T_DLL int STDCALL _std_get_average_ping(IntPtr handle);

	/*
	取得当前时刻，us
	*/
	T_DLL uint64_t STDCALL _std_get_timeUs();

	//helper
	T_DLL void STDCALL MemCopy(void *dst, void *src, int size);
	//T_DLL int STDCALL Compress(Bytef *dest, uLongf *destLen,const Bytef *source, uint64_t sourceLen);
	//T_DLL int STDCALL UnCompress(Bytef *dest, uLongf *destLen, const Bytef *source, uint64_t sourceLen);
	T_DLL void STDCALL Net_xLog(int level, const char* host);
	T_DLL void STDCALL Net_xLogFlush(int param);

}

#endif