///zzTODO: 1. 去除复杂的内存管理；使用简单的string pack；
///2. 检查性能
///3. 日志系统
/*
printf("Packet Logger Test.\n");
printf("Displays all packets being sent or received.\n");
printf("Overwrite PacketLogger::Log to render output into your own program.\n");
printf("Difficulty: Intermediate\n\n");

printf("Comma delimited log format:\n");
printf("1. Send or receive,\n");
printf("2. Raw (direct socket send) OR Ack (Acknowledgement) OR\nTms (Timestamped packet),\n");
printf("3. Message number,\n");
printf("4. Packet Number (Independent for send & receive).\n(Each Packet may contain multiple messages),\n");
printf("5. Packet ID (or a string for RPC calls),\n");
printf("6. Bits used by the message (does not include 2-4 byte RakNet header),\n");
printf("7. Time the message is sent,\n");
printf("8. Local System (binary IP followed by port),\n");
printf("9. Remote System (binary IP followed by port)\n\n");
*/

#include <stdlib.h>
#include <cstring>
#include <cstdio>
#include <time.h>


#include "BitStream.h"
#include "Rand.h"
#include "RakNetStatistics.h"
#include "MessageIdentifiers.h"
#include "Kbhit.h"
#include "GetTime.h"
#include "RakAssert.h"
#include "RakSleep.h"
#include "Gets.h"
#include "RakNetTypes.h"
#include "RakNetConnector.h"

#include "comm/xlogger/xlogger.h"
//#include "mars/stn/stn_logic.h"
//#include "mars/sdt/sdt_logic.h"
#include "mars/log/appender.h"
//#include "Bussiness/NetworkService.h"
#include "mars/comm/verinfo.h"

#include "mars/comm/socket/local_ipstack.h"


//BOOST_NO_EXCEPTIONS
namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
#ifdef _WIN32	
	//void throw_exception(std::exception const & e) // user defined
	//{
	//	printf("throw_exception: ");
	//}
#endif
}


#ifdef __ANDROID__
#include <jni.h>
#include <string>
#endif

using namespace RakNet;

#ifdef _WIN32
	#include "zlib1_2_11.h"

	#include <Wininet.h>  
	#include <Sensapi.h>  
	#include <iostream>
	#pragma comment(lib, "Sensapi.lib") 
	//#pragma comment(lib,"boost.lib")
	//#pragma comment(lib,"comm.lib")
	//#pragma comment(lib,"log.lib")
	//#pragma comment(lib,"../win32sample/x64/Debug/stn.lib")
	//#pragma comment(lib,"../win32sample/x64/Debug/sdt.lib")
	//#pragma comment(lib,"../win32sample/x64/Debug/baseevent.lib")
	//#pragma comment(lib,"../win32sample/x64/Debug/app.lib")

	void Fun_IsNetworkAlive()
	{
		DWORD   flags;//上网方式  
		BOOL   m_bOnline = TRUE;//是否在线    
		m_bOnline = IsNetworkAlive(&flags);
		if (m_bOnline)//在线    
		{
			if ((flags & NETWORK_ALIVE_LAN) == NETWORK_ALIVE_LAN)
			{
				std::cout << "在线：NETWORK_ALIVE_LAN\n";
			}

			if ((flags & NETWORK_ALIVE_WAN) == NETWORK_ALIVE_WAN)
			{
				std::cout << "在线：NETWORK_ALIVE_WAN\n";
			}

			if ((flags & NETWORK_ALIVE_AOL) == NETWORK_ALIVE_AOL)
			{
				std::cout << "在线：NETWORK_ALIVE_AOL\n";
			}
		}
		else
		{
			std::cout << "不在线\n";
		}
	}


#if EDITOR_CONSOLE==1
	#include "TelnetTransport.h"
	#include "ConsoleServer.h"
	#include "LogCommandParser.h"
	#include "PacketConsoleLogger.h"
	ConsoleServer consoleServer;
	TelnetTransport tt;
	LogCommandParser lcp;
	PacketConsoleLogger pcl;
#endif
#else
	#include <zlib.h>
#endif


std::string g_dataPath;
std::string g_persistentDataPath;

int g_log = 0;


int CCLOG(int level, const char * _Format, ...)
{
	const int bufsize = 1024*2;
	char buffer[bufsize];

	va_list argptr;
	int cnt;
	va_start(argptr, _Format);
	cnt = vsnprintf(buffer, bufsize, _Format, argptr);
	va_end(argptr);
	if (level == 0)
	{
#ifdef _WIN32
		::OutputDebugStringA(buffer);
#else
		LOGD("%s",buffer);
#endif
	}
	else
	{
		//写日志文件..
		//LOGE("%s",buffer);
		//appender_flush();
		xinfo2(TSF"%_", buffer);
	}
	return(cnt);
}


#if RAKNET_SUPPORT_IPV6 == 1
TLocalIPStack GetLocalIPStack(bool bForce);
TLocalIPStack g_TLocalIPStack = GetLocalIPStack(true);
TLocalIPStack GetLocalIPStack(bool bForce)
{
    if(!bForce)return g_TLocalIPStack;
    std::string _log;
    g_TLocalIPStack = local_ipstack_detect_log(_log);
    xinfo2(TSF"%_",_log);
    return g_TLocalIPStack;
}
void FlushLocalIPStack()
{
    GetLocalIPStack(true);
}

//#if (defined(__GNUC__)  || defined(__GCCXML__)) && !defined(__WIN32__)
//#include <netdb.h>
//#include <ifaddrs.h>
//#endif
//
//#define INET_ADDRSTRLEN     16
//#define    INET6_ADDRSTRLEN    46
//
//int formatIPV4Address(struct in_addr ipv4Addr, char address[])
//{
//    char dstStr[INET_ADDRSTRLEN];
//    char srcStr[INET_ADDRSTRLEN];
//    memcpy(srcStr, &ipv4Addr, sizeof(struct in_addr));
//    if (inet_ntop(AF_INET, srcStr, dstStr, INET_ADDRSTRLEN) != NULL) {
//        //address = [NSString stringWithUTF8String:dstStr];
//        memcpy(address, dstStr, INET_ADDRSTRLEN);
//        return 1;
//    }
//    return 0;
//}
//
//int formatIPV6Address(struct in6_addr ipv6Addr, char address[])
//{
//    char dstStr[INET6_ADDRSTRLEN];
//    char srcStr[INET6_ADDRSTRLEN];
//    memcpy(srcStr, &ipv6Addr, sizeof(struct in6_addr));
//    if (inet_ntop(AF_INET6, srcStr, dstStr, INET6_ADDRSTRLEN) != NULL) {
//        memcpy(address, dstStr, INET6_ADDRSTRLEN);
//        return 1;
//    }
//    return 0;
//}
//
//char *strlower(char *s, int len)
//{
//    for (int i = 0; i < len; i++) {
//        s[i] = tolower(s[i]);
//    }
//    return s;
//}
//
//
//
//int IOS_GetIp(char address[], char sa[])
//{
//    int ipv6 = 0;
//    struct ifaddrs *interfaces = NULL;
//    struct ifaddrs *temp_addr = NULL;
//
//    int success = getifaddrs(&interfaces);
//    if (success == 0)
//    {  // 0 表示获取成功
//        temp_addr = interfaces;
//        while (temp_addr != NULL)
//        {
//            xinfo2(TSF" ifa_name===%_\n",temp_addr->ifa_name);
//            // Check if interface is en0 which is the wifi connection on the iPhone
//            if (strcasecmp(temp_addr->ifa_name, "en0") == 0 || strcasecmp(temp_addr->ifa_name, "pdp_ip0") == 0)
//            {
//                //如果是IPV4地址，直接转化
//                if (temp_addr->ifa_addr->sa_family == AF_INET) {
//                    // Get NSString from C String
//                    //address = [self formatIPV4Address:((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr];
//                    formatIPV4Address(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr, address);
//                    xinfo2(TSF" ifa_name===%_;ipv4=%_\n", temp_addr->ifa_name, address);
//                }
//                //如果是IPV6地址
//                else if (temp_addr->ifa_addr->sa_family == AF_INET6)
//                {
//                    //如果找到
//                    int res = formatIPV6Address(((struct sockaddr_in6 *)temp_addr->ifa_addr)->sin6_addr, address);
//                    xinfo2(TSF" ifa_name===%_;ipv6=%_\n", temp_addr->ifa_name, address);
//                    //address = [self formatIPV6Address:((struct sockaddr_in6 *)temp_addr->ifa_addr)->sin6_addr];
//                    if (res == 1)//![address.uppercaseString hasPrefix:@"FE80"]
//                    {
//                        char *pp = strlower(address, strlen(address));
//                        if (memcmp(pp, "2001", 4) != 0 || memcmp(pp, "2002", 4) != 0) {   //2001
//                            ipv6 = 1;
//                            break;
//                        }
//                    }
//                }
//            }
//            temp_addr = temp_addr->ifa_next;
//        }
//    }
//    else
//    {
//        xerror2(TSF"getifaddrs fail.");
//    }
//    freeifaddrs(interfaces);
//    return ipv6;
//}




#endif

void* _internal_malloc(size_t size, const char* file, unsigned int line)
{
    void* block = rakMalloc_Ex(size, file, line);
    return block;
}

void _internal_free(void* pblock, const char* file, unsigned int line)
{
    if (pblock != (void*)0)
    {
        rakFree_Ex(pblock, file, line);
    }
}

void do_statistics(RakPeerInterface* peer)
{
    printf("Logging client statistics to ClientStats.txt\n");
    FILE *fp = fopen("ClientStats.txt", "wt");
    if (fp == NULL)
        return;
    char text[4096];
    float lostcount = 0;
    {
        RakNetStatistics *rssSender;
        rssSender = peer->GetStatistics(peer->GetSystemAddressFromIndex(0));
        if (rssSender->packetlossTotal > 0)
        {
            lostcount+= rssSender->packetlossTotal;
        }
        StatisticsToString(rssSender, text, 3);
        fprintf(fp, "==== Client %s ====\n", peer->GetMyGUID().ToString());
        fprintf(fp, "%s\n\n", text);
    }
    fprintf(fp, "==== Client lostcount=%f\n", lostcount);
    fclose(fp);
};

#if RAKNET_DEBUG_PAK_SEQ
#define HEAD_LEN 24+16
#else
#define HEAD_LEN 24
#endif
//如果一定要传ip，那么也应该使用整数，使用64,太奢侈。
//
int STDCALL _build_wrapped_packet(_Rak_Client_Info_t* pClient, Packet* pkt, unsigned char *buf, int pos)
{
    if (pkt && pkt->data)
    {
        assert((pos%4) == 0);
		unsigned short len = 0;
		if (pkt->length > sizeof(uint8_t))
		{
			len = (pkt->length - 1);
		}

        *(buf + pos) = HEAD_LEN; pos++;
        *(buf + pos) = pkt->data[0]; pos++;	//bugfix: #3635 SIGSEGV
        *(int16_t*)(buf + pos) = (pkt->systemAddress).GetPort(); pos += 2;
		*(int16_t*)(buf + pos) = len; pos += 2;
		*(int16_t*)(buf + pos) = 0; pos += 2;	//保留...

        *(int64_t*)(buf + pos) = (pkt->guid).g; pos += 8;
        *(uint64_t*)(buf + pos) = (pkt->systemAddress).address.addr4.sin_addr.s_addr; pos += 8;
#if RAKNET_DEBUG_PAK_SEQ
		*(int32_t*)(buf + pos) = pkt->orderingIndex; pos += 4;	//
		*(int32_t*)(buf + pos) = pkt->sequencingIndex; pos += 4;	//
		*(int32_t*)(buf + pos) = pkt->reliability; pos += 4;	//
		*(int32_t*)(buf + pos) = pkt->local_id; pos += 4;	//
#endif
        if (pkt->length > sizeof(uint8_t))
        {
            memcpy((buf + pos), pkt->data + sizeof(uint8_t), len); pos += len;
        }

        //每个报文对齐到4字节...
        pos = (pos + 3)&(~3);   //对齐

        // system address
        //in_addr in;
        //in.s_addr = (pkt->systemAddress).address.addr4.sin_addr.s_addr;
        //const char *ntoaStr = inet_ntoa(in);
        //size_t len = strlen(ntoaStr);

        //memcpy(out->source_ip, (void*)ntoaStr, len * sizeof(char));
        //out->source_ip[len] = '\0';
        //out->source_port = (pkt->systemAddress).GetPort();

        //if (pkt->length > sizeof(uint8_t))
        //{
        //    memcpy(out->data, pkt->data + sizeof(uint8_t), out->length * sizeof(uint8_t));
        //}
        //else
        //{
        //    out->length = 0;
        //}
    }
    return pos;
}


void StripSpace(char *&p)
{
	while (*p == ' ')
	{
		p++;
	}
}

//zzAdd for log:
void readCfg()
{
	if (g_persistentDataPath.length() == 0)
		return;

	std::string cfg = g_persistentDataPath + "/net_cfg.txt";
	FILE *fp = fopen(cfg.c_str(), "r");
	if (fp != NULL)
	{
		char buf[512];
		while (!feof(fp))
		{
			char *p = fgets(buf, 512, fp);
			if (p)
			{
				StripSpace(p);
				if (p[0] && p[0] != '#' && p[0] != '/')
				{
					char *e = strchr(p, '=');
					if (e)
					{
						*e = 0;
						std::string key;
						key.assign(p, (int)(e - p));

						++e;
						StripSpace(e);
						if (key == "log")
						{
							g_log = atoi(e);
						}
					}
				}
			}
		}

		fclose(fp);
	}
}


 _Rak_Settings_t g_Settings;
 _Rak_Client_Info_t** g_clientList = NULL;
 bool g_initialized = false;
 unsigned int g_nativeMemoryAllocated = 0;

inline _Rak_Client_Info_t *handle2Client(int handle)
{
    if (handle >= 0 && handle < (int)g_Settings.maxNumberOfPeers)
    {
        return g_clientList[handle];
    }
    return NULL;
}
inline RakPeerInterface *handle2peer(int handle)
{
    if (_Rak_Client_Info_t *pClient = handle2Client(handle))
    {
        return pClient->peer;
    }
    return NULL;
}


///
//void RkN_DumpPacket(int index, RakNet::Packet *pkg)
//{
//    CCLOG(2,"RkN_DumpPacket[%d][%x] guid=%lld;length=%d;bitSize=%d;guard=%x;data=%x;deleteData=%d;wasGeneratedLocally=%d\n", index,pkg,pkg->guid.g,pkg->length,pkg->bitSize,0,pkg->data,pkg->deleteData,pkg->wasGeneratedLocally);
//}



//void RkN_Dump(_Rak_Client_Info_t *pClient,int limit = 2)
//{
//    int index =pClient->lastIndex;
//    for (; index < pClient->count && (limit-- > 0); ++index)
//    {
//        RakNet::Packet *pkg = pClient->receiveSwapPool[index];
//        if (pkg)
//        {
//            RkN_DumpPacket(index,pkg);
//        }
//    }
//}



static int nGuad = 'DAUG';

static int g_max_count = 4;
#define RBUF_SIZE 1024 * 32
static unsigned char g_rbuf[RBUF_SIZE];
//循环取报文:
//#215 SIGSEGV. 3	DW-JailbreakGaea RkN_ReceiveCycle2(tagRakClientInfo*, unsigned char*, int) (RakNetConnector.cpp:199)
//1. pClient->lastIndex可能已经错误？
//2. RakNet::Packet可能已经错误？
int RkN_ReceiveCycle2(_Rak_Client_Info_t *pClient, unsigned char *buf, int buf_size)
{
    int handled = 0;
    int pos = 0;
    for (pClient->lastIndex; pClient->lastIndex < pClient->count; ++pClient->lastIndex)
    {
        RakNet::Packet *pkg = pClient->receiveSwapPool[pClient->lastIndex];
        if (pkg)
        {
			int space = buf_size - HEAD_LEN;
			if ((space < (int)pkg->length) || ((int)pkg->length<0))	//防止负值？或者大报文，直接丢弃..
			{
				xerror2(TSF"[error]RkN_ReceiveCycle2:: buf_size(%_)- HEAD_LEN < length(%_) \n", buf_size, pkg->length);
			}
			else
			{
				//缓存不够？
				if (space - pos < (int)pkg->length )
				{
					xwarn2(TSF"RkN_ReceiveCycle2(buf_size=%_ - pos=%_) < HEAD_LEN+pkg->length=%_\n", buf_size, pos, pkg->length);
					//RkN_Dump(pClient,2);
					break;
				}

				//#ifdef RAKNET_LOG_ZZ
				//        RAKNET_LOG("RkN_ReceiveCycle2 pClient=%d;lastIndex=%d;count=%d;pkg=%d\n", pClient->peer->GetMyGUID().ToString(), pClient->lastIndex, pClient->count, pkg->length);
				//#endif

				int pos2 = _build_wrapped_packet(pClient, pkg, buf, pos);	//+148  3	DW-Normal RkN_ReceiveCycle2(tagRakClientInfo*, unsigned char*, int) (RakNetConnector.cpp:198)
				if (pos2 > pos)
				{
					pos = pos2;
					if (buf_size < pos)
					{
						xerror2(TSF"[error]RkN_ReceiveCycle2: buf_size(%_)<pos(%_) ,pkg->data=%_\n", buf_size, pos, pkg->data);
					}
					handled++;
				}
				else
				{
					xerror2(TSF"[error]RkN_ReceiveCycle2:: pos2(%_)==pos(%_) \n", pos2, pos);
				}
			}
            pClient->peer->DeallocatePacket(pkg);
        }
        else
        {
			xerror2(TSF"[error]RkN_ReceiveCycle2:: [%_]pkg==null \n", pClient->lastIndex);
        }
        pClient->receiveSwapPool[pClient->lastIndex] = NULL;
    }
    
    int left = pClient->count - pClient->lastIndex;
    return left << 16 | handled;
}


extern "C"
{
    //返回一个整数，高16位表示，剩余报文，低16位表示已经处理的报文
    T_DLL void* STDCALL RkN_ReceiveCycle2(int handle, int *result)
    {
        
#if EDITOR_CONSOLE==1
        consoleServer.Update();
#endif
        
        if (_Rak_Client_Info_t *pClient = handle2Client(handle))
        {
            if (RakPeerInterface* peer = pClient->peer)
            {
                int limit = g_Settings.packetSwapPoolSize;
                
                //检查
                //如果上一次循环没有处理完？
                if (pClient->count > pClient->lastIndex)
                {
                    //RkN_Dump(pClient);                    
                    if (pClient->guard == nGuad && pClient->lastIndex < limit && pClient->count <= limit)
                    {
                        //返回0，则c#退出循环
                        *result = RkN_ReceiveCycle2(pClient, g_rbuf, RBUF_SIZE);	//+104 （4	DW-Normal RkN_ReceiveCycle2 (RakNetConnector.cpp:590）
                        return g_rbuf;
                    }
                    else
                    {
						xerror2(TSF"RkN_ReceiveCycle2: check guard(%_/%_) lastIndex(%_/%_) count(%_/%_) pClient(%_) peer(%_)\n", pClient->guard, nGuad, pClient->lastIndex , limit , pClient->count , limit,pClient,peer);
                    }
                }
                //一轮新的开始...
                //pClient->count = 0;
                pClient->lastIndex = 0;
                pClient->count = peer->Receive(pClient->receiveSwapPool, limit);
                if (pClient->count>0)
                {
                    if(g_max_count<pClient->count)
                    {
						xinfo2(TSF"RkN_ReceiveCycle2: g_max_count(%_)<pClient->count(%_) \n", g_max_count,pClient->count);
                        g_max_count=pClient->count;
                    }                    
                    *result = RkN_ReceiveCycle2(pClient, g_rbuf, RBUF_SIZE);	//
                }
                else
                {
                    *result = 0;
                }
                return g_rbuf;
            }
            *result = -1;
            return 0;
        }
        *result = -2;
        return 0;
    }
    
    
    T_DLL void STDCALL RkN_Initialize2(_Rak_Settings_t* pSettings,const char *dataPath,const char *persistentDataPath)
    {
        if (!g_initialized && pSettings != (_Rak_Settings_t*)0)
        {
			if(dataPath)
				g_dataPath = dataPath;
			if(persistentDataPath)
				g_persistentDataPath = persistentDataPath;

			std::string dir = g_persistentDataPath + "/xLog";
			CCLOG(0,"RkN_Initialize2(%s) MARS_BUILD_TIME: " MARS_BUILD_TIME, dir.c_str());
			appender_open(kAppednerAsync, dir.c_str(), "zz",NULL);
			xlogger_SetLevel(kLevelAll);

			xinfo2(TSF"start log:%_", 0);

            g_Settings.maxNumberOfPeers = pSettings->maxNumberOfPeers > 0 ? pSettings->maxNumberOfPeers : DEFAULT_MAX_PEERS;
            g_Settings.packetSwapPoolSize = pSettings->packetSwapPoolSize > 0 ? pSettings->packetSwapPoolSize : DEFAULT_PACKET_SWAP_POOL_SIZE;
            g_nativeMemoryAllocated = 0;

            g_clientList = (_Rak_Client_Info_t**)/*rakMalloc_Ex*/_internal_malloc(sizeof(_Rak_Client_Info_t*) *g_Settings.maxNumberOfPeers, _FILE_AND_LINE_);
            memset(g_clientList, 0, sizeof(_Rak_Client_Info_t*)*g_Settings.maxNumberOfPeers);
#if EDITOR_CONSOLE==1
			pcl.SetLogCommandParser(&lcp);
			consoleServer.AddCommandParser(&lcp);
			consoleServer.SetTransportProvider(&tt, 23);
#endif
			readCfg();

            xinfo2(TSF"[w]RkN_Initialize2:MTU_SIZE=%_;%_;%_\n",MAXIMUM_MTU_SIZE,g_dataPath.c_str(),g_persistentDataPath.c_str());
            g_initialized = true;

			appender_flush();
        }
    }

    T_DLL void STDCALL RkN_Initialize()
    {
        if (!g_initialized)
        {
            _Rak_Settings_t conf;
            conf.maxNumberOfPeers = DEFAULT_MAX_PEERS;
            conf.packetSwapPoolSize = DEFAULT_PACKET_SWAP_POOL_SIZE;
            RkN_Initialize2(&conf,NULL,NULL);
        }
    }

    T_DLL void STDCALL RkN_ReleaseClient(int handle)
    {
        if (handle >= 0 && handle < (int)g_Settings.maxNumberOfPeers)
        {
            _Rak_Client_Info_t* pClient = g_clientList[handle];
            if (pClient != NULL)
            {
				xinfo2(TSF"[w]RkN_ReleaseClient(%_)\n", handle);
                /*rakFree_Ex*/_internal_free((void*)pClient->receiveSwapPool, _FILE_AND_LINE_);
                RakPeerInterface::DestroyInstance(pClient->peer);
                pClient->peer = NULL;
				pClient->lastIndex = 0;
				pClient->count = 0;

				if (pClient->messageHandler)
				{
					RakNet::PacketLogger::DestroyInstance(pClient->messageHandler);
					pClient->messageHandler = NULL;
				}
                /*rakFree_Ex*/_internal_free((void*)pClient, _FILE_AND_LINE_);
                g_clientList[handle] = NULL;

				appender_flush();
            }
        }
    }

    T_DLL int STDCALL RkN_CreatePeer()
    {
        if (g_initialized)
        {
            for (int ii = 0; ii < (int)g_Settings.maxNumberOfPeers; ++ii)
            {
                if (g_clientList[ii] == NULL)
                {
                    _Rak_Client_Info_t* client = (_Rak_Client_Info_t*)/*rakMalloc_Ex*/_internal_malloc(sizeof(_Rak_Client_Info_t), _FILE_AND_LINE_);
                    client->swapIndex = 0;

                    client->peer = RakPeerInterface::GetInstance();
                    client->receiveSwapPool = (Packet**)/*rakMalloc_Ex*/_internal_malloc(sizeof(Packet*) * g_Settings.packetSwapPoolSize, _FILE_AND_LINE_);
                    memset(client->receiveSwapPool, 0, sizeof(Packet*) * g_Settings.packetSwapPoolSize);
					client->count = 0;	//bugfix: 未初始化变量，导致崩溃.
					client->lastIndex = 0;
					client->messageHandler = 0;

					client->guard = nGuad;	//设定

					//if (g_log)
					//{
					//	client->messageHandler = RakNet::PacketLogger::GetInstance();
					//	if (client->messageHandler)
					//	{
					//		client->peer->AttachPlugin(client->messageHandler);
					//		client->messageHandler->LogHeader();
					//	}
					//}


#if EDITOR_CONSOLE==1
					client->peer->AttachPlugin(&pcl);
#endif

					g_clientList[ii] = client;

                    if (client->peer == NULL)
                    {
                        RkN_ReleaseClient(ii);
                        return -1; // out of memory
                    }

					//appender_flush();
                    return ii;
                }
            }
        }

        return -1;
    }
    

    T_DLL int STDCALL RkN_Startup(int handle, const char* ip, unsigned short port, unsigned int maxConnections)
    {
        int result = (int)STARTUP_OTHER_FAILURE;
        if (RakPeerInterface* peer = handle2peer(handle))
        {
            // auto ping/pong
            //peer->SetOccasionalPing(true);
            int ipv6 = 0;
#if RAKNET_SUPPORT_IPV6 == 1
            if(ELocalIPStack_IPv6 == g_TLocalIPStack)
                ipv6 = true;
//            char address[INET6_ADDRSTRLEN];
//            ipv6 = IOS_GetIp(address, NULL);
#endif
            if(ipv6)
                ip = "";
            SocketDescriptor desc(port, ip);
            if (ipv6)
            {
                desc.socketFamily = AF_INET6;      //可以指定链接的socket...
            }
            result = (int)peer->Startup(maxConnections, &desc, 1);

			appender_flush();
        }
        return result;
    }

    T_DLL void STDCALL RkN_SetMaximumIncomingConnections(int handle, unsigned short numberAllowed)
    {
        if (RakPeerInterface* peer = handle2peer(handle))
        {
            peer->SetMaximumIncomingConnections(numberAllowed);
        }
    }

    //TODO:
    T_DLL int STDCALL RkN_Receive(int handle)
    {
        //_Packet_t* out = (_Packet_t *)0;
        if (_Rak_Client_Info_t *pClient = handle2Client(handle))
        {
            RakPeerInterface* peer = pClient->peer;
            Packet* pkt = peer->Receive();
            if (pkt != (Packet*)0)
            {
                //out = _build_wrapped_packet(pClient, pkt);
                peer->DeallocatePacket(pkt);
            }
        }

        return 0;
    }

    T_DLL uint32_t STDCALL RkN_Send(int handle, const char *data, const int length, int priority, int reliability, char orderingChannel, uint64_t guid, bool broadcast, uint32_t forceReceiptNumber )
    {
        if (RakPeerInterface* peer = handle2peer(handle))
        {
            RakNetGUID uid(guid);
            AddressOrGUID identifier(uid);
			
			if (length > MAXIMUM_MTU_SIZE)
			{
				xinfo2(TSF"[w]RkN_Send: handle(%_) length =%_ priority=%_\n", handle, length, priority);
			}

            return peer->Send(data, length, (PacketPriority)priority, (PacketReliability)reliability, orderingChannel, identifier, broadcast, forceReceiptNumber);
        }

        return 0;
    }

    T_DLL void STDCALL RkN_Broadcast(int handle, const char *data, const int length, int priority, int reliability, char orderingChannel, void* identifierPtr, int identifierAmount, uint32_t forceReceiptNumber = 0)
    {
        if (RakPeerInterface* peer = handle2peer(handle))
        {
            uint64_t* pGuidList = (uint64_t*)identifierPtr;
            if (pGuidList != (uint64_t*)0)
            {
                for (int ii = 0; ii < identifierAmount; ++ii)
                {
                    RakNetGUID guid(pGuidList[ii]);
                    AddressOrGUID identifier(guid);
                    peer->Send(data, length, (PacketPriority)priority, (PacketReliability)reliability, orderingChannel, identifier, false, forceReceiptNumber);
                }
            }
        }
    }

    T_DLL void STDCALL RkN_Broadcast2(int handle, const char *data, const int length, int priority, int reliability, char orderingChannel, uint32_t forceReceiptNumber = 0)
    {
        if (RakPeerInterface* peer = handle2peer(handle))
        {
            DataStructures::List<SystemAddress> addresses;
            DataStructures::List<RakNetGUID> guids;
            peer->GetSystemList(addresses, guids);
            if (guids.Size() > 0)
            {
                for (int ii = 0; ii < (int)guids.Size(); ++ii)
                {
                    AddressOrGUID identifier(guids.Get(ii));
                    peer->Send(data, length, (PacketPriority)priority, (PacketReliability)reliability, orderingChannel, identifier, false, forceReceiptNumber);
                }
            }
        }
    }

    T_DLL void STDCALL RkN_Shutdown(int handle, int blockDuration , unsigned char orderingChannel , int disconnectionNotificationPriority )
    {
        if (_Rak_Client_Info_t *pClient = handle2Client(handle))
        {
            if (RakPeerInterface* peer = pClient->peer)
            {

				xinfo2(TSF"[w]RkN_Shutdown: handle(%_) peer(%_) pClient->count=%_ pClient->lastIndex=%_\n", handle,peer,pClient->count,pClient->lastIndex);
                peer->Shutdown(blockDuration, orderingChannel, (PacketPriority)disconnectionNotificationPriority);
            
                if(pClient->count!=pClient->lastIndex)
                {
                    memset(pClient->receiveSwapPool, 0, sizeof(Packet*) * g_Settings.packetSwapPoolSize);
                    pClient->count = 0;	//bugfix: 未初始化变量，导致崩溃.
                    pClient->lastIndex = 0;
                }
				appender_flush();
            }
        }
    }

    T_DLL void STDCALL RkN_CloseConnection(int handle, uint64_t guid, bool sendDisconnectionNotification , unsigned char orderingChannel , int disconnectionNotificationPriority )
    {
        if (RakPeerInterface* peer = handle2peer(handle))
        {
			xinfo2(TSF"handle(%_) peer(%_) sendDisconnectionNotification=%_ \n", handle, peer, sendDisconnectionNotification);
			RakNetGUID uid(guid);
            AddressOrGUID identifier(uid);

			//RakNetStatistics *rns = peer->GetStatistics(peer->GetSystemAddressFromIndex(0), NULL);
			//if (rns)
			//{
			//	if (rns->runningTotal[ACTUAL_BYTES_RECEIVED])
			//	{
			//		char text[4096]; text[0] = 0;
			//		StatisticsToString(rns, text, 3);
			//		if (text[0])
			//		{
			//			xinfo2(TSF"Statistics\n%_\n\n", text);
			//		}
			//	}
			//}

            //do_statistics(peer);
            peer->CloseConnection(identifier, sendDisconnectionNotification, orderingChannel, (PacketPriority)disconnectionNotificationPriority);
        }
    }

	//@timeBetweenSendConnectionAttemptsMS: 1500，用于多个MTU测试连接的发出间隔
	//@sendConnectionAttemptCount:6,用于控制总发的连接请求数。
    T_DLL int STDCALL RkN_Connect(int handle, const char* host, unsigned short port, unsigned int sendConnectionAttemptCount , unsigned int timeBetweenSendConnectionAttemptsMS , RakNet::TimeMS timeoutTime )
    {
        int result = 1;
        if (RakPeerInterface* peer = handle2peer(handle))
        {
            SystemAddress addr(host, port);
            AddressOrGUID identifier(addr);

            ConnectionState peerSt = peer->GetConnectionState(identifier);
			xinfo2(TSF"RkN_Connect(%_): ConnectionState=%_ ;AttemptCount=%_;peer=%_\n", addr.ToString(), peerSt, sendConnectionAttemptCount,peer);

			if (peerSt == IS_CONNECTED)
			{
				peer->CloseConnection(identifier, false);

				ConnectionState peerSt = peer->GetConnectionState(identifier);
				xinfo2(TSF"RkN_Connect(%_): ConnectionState2=%_ ;AttemptCount=%_\n", addr.ToString(), peerSt, sendConnectionAttemptCount);
			}

            if (peerSt == IS_DISCONNECTED || peerSt == IS_NOT_CONNECTED)
            {
                result = (int)peer->Connect(host, port, (const char*)0, 0, (PublicKey*)0, 0u, sendConnectionAttemptCount, timeBetweenSendConnectionAttemptsMS, timeoutTime);
            }
        }
		else
		{
			xerror2(TSF"RkN_Connect: handle2peer(%_) fail\n", handle);
		}
		//appender_flush();
        return result;
    }

	///
	T_DLL int STDCALL RkN_Reconnect(int handle, RakNet::TimeMS timeoutTime = 0)
	{
		int result = 1;
		if (RakPeerInterface* peer = handle2peer(handle))
		{
			result = (int)peer->Reconnect(timeoutTime);
		}

		return result;
	}


    T_DLL int STDCALL RkN_GetConnectionState(int handle, uint64_t guid)
    {
        if (RakPeerInterface* peer = handle2peer(handle))
        {
            RakNetGUID addr(guid);
            AddressOrGUID identifier(addr);

            return (int)peer->GetConnectionState(identifier);
        }
        return -1;
    }

    T_DLL int STDCALL RkN_GetAveragePing(int handle, uint64_t guid)
    {
        if (RakPeerInterface* peer = handle2peer(handle))
        {
            RakNetGUID uid(guid);
            AddressOrGUID identifier(uid);

            return peer->GetAveragePing(identifier);
        }

        return 1000;
    }




    T_DLL void STDCALL RkN_ReleasePacketList(void* packetList)
    {
    }

    //接收消息循环...
    //返回剩余消息个数..
    T_DLL int STDCALL RkN_ReceiveCycle(int handle, int packetRetrieveLimit, unsigned char *buf, int buf_size)
    {
        unsigned int limit = packetRetrieveLimit > DEFAULT_PACKET_SWAP_POOL_SIZE || packetRetrieveLimit <= 0 ? DEFAULT_PACKET_SWAP_POOL_SIZE : packetRetrieveLimit;
        unsigned int count = 0;

        if (limit > g_Settings.packetSwapPoolSize)
            limit = g_Settings.packetSwapPoolSize;

        //if (handle >= 0 && handle < g_Settings.maxNumberOfPeers && g_clientList[handle] != NULL)
        //{
        //    _Rak_Client_Info_t* pClient = g_clientList[handle];
        //    RakPeerInterface* peer = pClient->peer;
        //    if (peer != (RakPeerInterface *)0)
        //    {
        //        count = peer->Receive(pClient->receiveSwapPool, limit);

        //        //因为大部分
        //        pClient->container->amount = count;

        //        for (unsigned int i = 0; i < count; ++i)
        //        {
        //            pClient->container->ppPktList[i] = _build_wrapped_packet(pClient, pClient->receiveSwapPool[i]);
        //            peer->DeallocatePacket(pClient->receiveSwapPool[i]);
        //            pClient->receiveSwapPool[i] = (Packet*)0;
        //        }

        //        return (void*)(pClient->container);
        //    }
        //}
        return 0;
    }

    T_DLL void STDCALL RkN_Uninitialize()
    {
        if (g_initialized)
        {
            for (int ii = 0; ii < (int)g_Settings.maxNumberOfPeers; ++ii)
            {
                RkN_ReleaseClient(ii);
            }

            /*rakFree_Ex*/_internal_free(g_clientList, _FILE_AND_LINE_);
            g_clientList = (_Rak_Client_Info_t**)0;
            g_nativeMemoryAllocated = 0;
            g_initialized = false;
        }
    }

    T_DLL void STDCALL RkN_KeepConnectionAndRest(int handle, uint64_t guid)
    {
#if _MODULE_KEEP_CONN_AND_RESET == 1
        if (RakPeerInterface* peer = handle2peer(handle))
        {
            RakNetGUID addr(guid);
            AddressOrGUID identifier(addr);
            peer->Reset(identifier);
        }
#endif
    }

    T_DLL unsigned int STDCALL RkN_GetNativeMemoryUsage()
    {
        return g_nativeMemoryAllocated;
    }

    //新加的。。。
    T_DLL void STDCALL MemCopy(void *dst,void *src,int size)
    {
        memcpy(dst,src,size);
    }

    T_DLL int STDCALL Compress(Bytef *dest, uLongf *destLen,const Bytef *source, uint64_t sourceLen)
    {
		//uLongf destLen = *destLen0;
        int res = compress2(dest, destLen, source, sourceLen, Z_BEST_SPEED);
		//*destLen0 = destLen;
		return res;
	}

    T_DLL int STDCALL UnCompress(Bytef *dest, uLongf *destLen, const Bytef *source, uint64_t sourceLen)
    {
		//uLongf destLen = *destLen0;
		int res = uncompress(dest, destLen, source, sourceLen);
		//*destLen0 = destLen;
		return res;
	}

	//typedef enum {
	//	kLevelAll = 0,
	//	kLevelVerbose = 0,
	//	kLevelDebug,    // Detailed information on the flow through the system.
	//	kLevelInfo,     // Interesting runtime events (startup/shutdown), should be conservative and keep to a minimum.
	//	kLevelWarn,     // Other runtime situations that are undesirable or unexpected, but not necessarily "wrong".
	//	kLevelError,    // Other runtime errors or unexpected conditions.
	//	kLevelFatal,    // Severe errors that cause premature termination.
	//	kLevelNone,     // Special level used to disable all log messages.
	//} TLogLevel;
	//0722add: log
	T_DLL void STDCALL RkN_xLog(int level, const char* host)
	{
		xlog2((TLogLevel)level, TSF"%_\n", host);
	}
	//1012add: flush log
	T_DLL void STDCALL RkN_xLogFlush(int param)
	{
		appender_flush_sync();
	}

	//2019/1/18: 增加网络事件通知，用于更新IPStack
	T_DLL int STDCALL RkN_NetEvent(int param)
	{
#if RAKNET_SUPPORT_IPV6 == 1
		FlushLocalIPStack();
		//必须异步...
		//std::string GetDetailNetInfo()
		return g_TLocalIPStack;
#endif
		return 0;
	}





	//T_DLL int STDCALL RkN_GetIpStack(Bytef *dest, uLongf *destLen, const Bytef *source, uint64_t sourceLen)
	//{
	//}
}
#ifdef __ANDROID__
extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_test360pay_MainActivity_testNewString(
        JNIEnv* env,
        jobject /* this */) {

    int use = RkN_GetNativeMemoryUsage();
    const char *hello = "Hello from raknet";
    return env->NewStringUTF(hello);
}
#endif


void ExportRAKNET() {

}
