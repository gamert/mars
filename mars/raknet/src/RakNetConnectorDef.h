#ifndef _RAKNET_CONNECTOR_DEF_H_
#define _RAKNET_CONNECTOR_DEF_H_

extern "C"
{
	T_DLL void STDCALL RkN_Initialize2(_Rak_Settings_t* pSettings, const char *dataPath, const char *persistentDataPath);
	T_DLL int STDCALL RkN_CreatePeer();
	T_DLL int STDCALL RkN_Startup(int handle, const char* ip, unsigned short port, unsigned int maxConnections);
	T_DLL void* STDCALL RkN_ReceiveCycle2(int handle, int *result);
	T_DLL uint32_t STDCALL RkN_Send(int handle, const char *data, const int length, int priority, int reliability, char orderingChannel, uint64_t guid, bool broadcast, uint32_t forceReceiptNumber = 0);
	T_DLL void STDCALL RkN_Shutdown(int handle, int blockDuration = 0, unsigned char orderingChannel = 0, int disconnectionNotificationPriority = 3);
	T_DLL void STDCALL RkN_CloseConnection(int handle, uint64_t guid, bool sendDisconnectionNotification = true, unsigned char orderingChannel = 0, int disconnectionNotificationPriority = 3);
	T_DLL int STDCALL RkN_Connect(int handle, const char* host, unsigned short port, unsigned int sendConnectionAttemptCount = 4, unsigned int timeBetweenSendConnectionAttemptsMS = 1500, RakNet::TimeMS timeoutTime = 0);
	T_DLL int STDCALL RkN_GetConnectionState(int handle, uint64_t guid);
	T_DLL int STDCALL RkN_GetAveragePing(int handle, uint64_t guid);
	T_DLL void STDCALL RkN_Uninitialize();
	T_DLL void STDCALL RkN_ReleaseClient(int handle);

	T_DLL void STDCALL MemCopy(void *dst, void *src, int size);
	//T_DLL int STDCALL Compress(Bytef *dest, uLongf *destLen,const Bytef *source, uint64_t sourceLen);
	//T_DLL int STDCALL UnCompress(Bytef *dest, uLongf *destLen, const Bytef *source, uint64_t sourceLen);
	T_DLL void STDCALL RkN_xLog(int level, const char* host);
	T_DLL void STDCALL RkN_xLogFlush(int param);

}

#endif
