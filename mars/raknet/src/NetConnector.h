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
	PlatformNotInstall = 10,
	PlatformNotSupportApi,
	UserCancel,
	UserDenied,
	LoginFailed,
	NoAccessToken,
	TokenInvalid,
	AccessTokenExpired = 16,
	RefreshTokenExpired,
	PayTokenExpired,
	NeedReset,
	NoPermission,
	CheckToken,
	NeedRealNameAuth,
	InitializeError = 50,
	MarketError,
	ItemInvalidate,
	Incomplete,
	NotYetConsume,
	TransactionIdNotExist,
	GooglePlayNotSupport,
	LbsNeedOpenLocation = 70,
	LbsLocateFail,
	WebviewClosed = 80,
	PluginEnd = 99,
	InnerError,
	NotInitialized,
	NoConnection,
	ConnectFailed,
	InvalidServiceId,
	IsConnecting,
	GcpError = 120,
	PeerCloseConnection,
	PeerStopSession,
	PkgNotCompleted,
	SendFailed,
	StayInQueue,
	SvrIsFull,
	TokenSvrError,
	AuthFailed,
	HttpOtherError = 270,
	HttpBadVersion,
	HttpBadURL,
	HttpBadMethod,
	HttpReqToLong,
	HttpBadHeader,
	Others = 10000
};

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


enum class kEncryptMethod
{
	None,
	Tea,
	Aes,
	Aes2
};

enum class kKeyMaking
{
	None,
	Auth,
	Server,
	RawDH,
	EncDH
};

