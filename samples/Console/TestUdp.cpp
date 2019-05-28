#include "stdlib.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/bootrun.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/dns/dns.h"

#include "mars/comm/socket/udpclient.h"
#include "mars/comm/socket/udpserver.h"

#include <thread>
#include <mutex>
#include <map>

//是否使用server
#define USE_LOOP_SERVER 1
typedef std::chrono::steady_clock::time_point CTimePoint;
typedef std::chrono::duration<double> CTimeDuration;

#define GetClock std::chrono::steady_clock::now

#define LOG_DETAIL		0
#define MAX_THREADS		1
#define MAX_TEST_TIME	5000

inline void ThreadSleepMS(int ms)
{
	//::Sleep(ms);
	//::sleep();
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

static void __GetIP() {
	xverbose_function();
}

//时间测量
struct TimeMeasure_t
{
public:
	int m_id;
	std::vector<double> m_pings;	//second
	TimeMeasure_t(int id):m_id(id)
	{
		m_pings.reserve(1024);
	}
	void Clear()
	{
		m_pings.clear();
	}
	//返回
	double Dump(bool blog = false)
	{
		//now() 获取当前时钟
		double dmin = 9999999;
		double dmax = 0; //dmax.zero();
		double dd = 0; //dd.zero();
		for (int i = 0; i < m_pings.size(); ++i)
		{
			if (dmin > m_pings[i])
				dmin = m_pings[i];

			if (dmax < m_pings[i])
				dmax = m_pings[i];

			dd += m_pings[i];
		}
		int count = max(1, m_pings.size());
		double d2 = dd / count;
		//计算方差:用来度量随机变量和其数学期望（即均值）之间的偏离程度
		double Variance = 0;
		for (int i = 0; i < m_pings.size(); ++i)
		{
			double dd = m_pings[i] - d2;
			Variance += dd * dd;
		}
		Variance = Variance / count;
		//标准差
		double SD = sqrtf(Variance);
		if(blog)
			printf("TimeMeasure[%d]:count=%d, avg=%llf,Variance=%llf,SD=%llf, [%llf,%llf] \n", m_id, count, d2, Variance, SD, dmin, dmax);
		return d2;
	}
};

TimeMeasure_t s_TimeMeasure(0);

class TimeMeasureGroup_t
{
public:
	std::map<int, TimeMeasure_t*> m_Measures;
	void Add(int id, double dura)
	{
		TimeMeasure_t* p;
		std::map<int, TimeMeasure_t*>::iterator itr = m_Measures.find(id);
		if (itr == m_Measures.end())
		{
			p = new TimeMeasure_t(id);
			m_Measures[id] = p;
		}
		else
		{
			p = itr->second;
		}
		p->m_pings.push_back(dura);
	}
	void Dump()
	{
		for each (auto var in m_Measures)
		{
			var.second->Dump();
		}
	}


};


class MUdpServerImp:public IAsyncUdpServerEvent {
public:
	//TimeMeasureGroup_t m_TimeMeasureGroup;
	~MUdpServerImp()
	{
	}

	virtual void OnError(UdpServer* _this, int _errno)
	{
		printf("MUdpServerImp:OnError\n");
	};
	//
	virtual void OnDataGramRead(UdpServer* _this, struct sockaddr_in* _addr, void* _buf, size_t _len)
	{
		const char *pBuf = (const char *)_buf;

		int *id = (int *)pBuf;
		int *seconds = (int *)(pBuf + 4);

#if LOG_DETAIL
//		printf("MUdpServerImp:OnDataGramRead(%d,%d,%lld) dt=%lld(%llf), len=%d\n\n", *id, *seconds, *t2 , t3 - *t2, time_span.count(), _len);
#endif
		_this->SendAsync(_addr, _buf, _len);
	};

	void doStatistcs()
	{
		//m_TimeMeasureGroup.Dump();
	}
};


class CAsynUdpClient :public IAsyncUdpClientEvent
{
public:
	int m_id;
	UdpClient *tc;

	uint64_t _lastTP;
	int seconds;

	TimeMeasure_t _tm;

	CAsynUdpClient(int id,const char *ip,int _port):m_id(id), _tm(id)
	{
		tc = new UdpClient(ip, _port, this);
		_lastTP = gettickcount();
		seconds = 0;
	}
	~CAsynUdpClient()
	{
		delete tc;
		double d = _tm.Dump();
		if(d>0)
			s_TimeMeasure.m_pings.push_back(d);
		else
		{
			printf("~CAsynUdpClient[%d]: no valid ping \n", m_id);
		}
	}

	bool isalive() { return true; }
	//
	//static void CreateAsynUdpClient(int id,const char* ip, int port)
	//{
	//	CAsynUdpClient *p = new CAsynUdpClient(id);
	//	p->Loop(ip, port);
	//};

	void SendPing(int seconds)
	{
		char buf[64];
		char *pBuf = buf;
		*((int *)pBuf) = m_id; pBuf += sizeof(int);
		*((int *)pBuf) = seconds; pBuf += sizeof(int);
		CTimePoint t2 = GetClock();
		*((CTimePoint *)pBuf) = t2; pBuf += sizeof(CTimePoint);
		int len = pBuf - buf;
		tc->SendAsync(buf, len);
	}
	//
	void timerloop()
	{
		uint64_t t3 = gettickcount();
		uint64_t dmin = t3 - _lastTP;
		if (dmin >= 1000)
		{
			seconds++;
			SendPing(seconds);
			_lastTP = t3;
		}
	}

	//void loop()
	//{
	//	//tc->Connect();

	//	//UdpClient::TUdpStatus status = tc->GetUdpStatus();
	//	uint64_t begin = gettickcount();
	//	uint64_t end;
	//	int seconds = 0;
	//	do
	//	{
	//		ThreadSleepMS(1000);
	//		seconds++;
	//		if (seconds % 5 == 1)
	//		{
	//			SendPing(seconds);
	//		}
	//		end = gettickcount();
	//	} while (end - begin < MAX_TEST_TIME);
	//}

	virtual void OnError(UdpClient* _this, int _errno)
	{
		printf("CAsynUdpClient:OnError[%d] :_errno=%d,pings = %d\n", m_id, _errno, _tm.m_pings.size());
	};
	virtual void OnDataGramRead(UdpClient* _this, void* _buf, size_t _len)
	{
		const char *pBuf = (const char *)_buf;

		int *id = (int *)pBuf;
		int *seconds = (int *)(pBuf +4);

		CTimePoint *t2 = (CTimePoint *)(pBuf + 8);
		CTimePoint t3 = GetClock();

		CTimeDuration time_span = std::chrono::duration_cast<CTimeDuration>(t3 - *t2);

		double dd = time_span.count();
		if (dd >= 0)
		{
			_tm.m_pings.push_back(dd);
		}
		else
		{
			printf("!!!!CAsynUdpClient:OnDataGramRead error:(%d,%d) len=%d;dd=%llf\n\n", *id, *seconds, _len, dd);
		}
		//m_TimeMeasureGroup.Add(*id, time_span);

#if LOG_DETAIL
		printf("CAsynUdpClient:OnDataGramRead(%d,%d) len=%d\n\n", *id, *seconds, _len);
#endif
	};
	virtual void OnDataSent(UdpClient* _this)
	{
#if LOG_DETAIL
		printf("CAsynUdpClient:OnDataSent\n");
#endif
	};

	//void Dump();

};

#define IUINT32 int
/*
job thread:
*/
template<typename T>
class handlethread
{
public:
	void init()
	{
		isstop = false;
		_thread = std::thread(std::bind(&handlethread::loop, this));
	}
	void shutdown()
	{
		isstop = true;
		_thread.join();
		_mutex.lock();
		for (std::map<IUINT32, T*>::iterator iter = clients.begin();
			iter != clients.end(); ++iter)
		{
			delete iter->second;
		}
		_mutex.unlock();
	}

	//线程think...
	void loop()
	{
		//微秒
		std::chrono::microseconds dura(10);
		//std::chrono::seconds dura(1);
		for (; !isstop;)
		{
			_mutex.lock();
			for (std::map<IUINT32, T*>::iterator iter = clients.begin();
				iter != clients.end();)
			{
				if (!iter->second->isalive())
				{
					delete iter->second;
					iter = clients.erase(iter);
				}
				else
				{
					iter->second->timerloop();
					++iter;
				}
			}
			_mutex.unlock();

			//_time_measure.Update();

			std::this_thread::sleep_for(dura);
			//			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		}
	}

	T* AddClient(IUINT32 conv, const char *ip, int _port)
	{
		T *pclient = NULL;
		_mutex.lock();
		std::map<IUINT32, T*>::iterator iter = clients.find(conv);
		if (iter == clients.end())
		{
			pclient = new T(conv,ip, _port);
			clients.insert(std::map<IUINT32, T*>::value_type(conv, pclient));
		}
		else
		{
			pclient = iter->second;
		}
		_mutex.unlock();

		return pclient;
	}

public:
	volatile bool isstop;
	std::mutex _mutex;
	std::thread _thread;
	std::map<IUINT32, T*> clients;
	//time_measure_t _time_measure;
};

//
template<typename T>
class ClientHandler
{
public:
	ClientHandler()
	{
		maxtdnum = MAX_THREADS;
		start_index = 1;
	}
	int start_index;

	void CreateClients(int count, const char *ip, int _port)
	{
		for (int i = 0; i < count; ++i)
		{
			IUINT32 conv = start_index++;
			timerthreads[conv % maxtdnum]-> AddClient(conv, ip, _port);
		}
		//std::thread thrd1(CAsynUdpClient::CreateAsynUdpClient, 1, ip, _port);
		//std::thread thrd2(CAsynUdpClient::CreateAsynUdpClient, 2, ip, _port);
		//std::thread thrd3(CAsynUdpClient::CreateAsynUdpClient, 3, ip, _port);
	}


	void init()
	{
		//_slot = slot;
		for (unsigned int i = 0; i < maxtdnum; i++)
		{
			handlethread<T>* hthread = new handlethread<T>;
			hthread->init();
			timerthreads.push_back(hthread);
		}
	}
	void shutdown()
	{
		for (typename  std::vector<handlethread<T>*>::iterator iter = timerthreads.begin();
			iter != timerthreads.end(); ++iter)
		{
			(*iter)->shutdown();
			delete *iter;
		}
	}
	int _slot;
	int maxtdnum;
	std::vector<handlethread<T>*> timerthreads;
};



static void InitSocket()
{

	WSADATA Data;
	SOCKADDR_IN serverSockAddr;
	SOCKADDR_IN clientSockAddr;
	SOCKET serverSocket;
	SOCKET clientSocket;
	int addrLen = sizeof(SOCKADDR_IN);
	int status;
	int numrcv;
	char buffer[256];

	/* initialize the Windows Socket DLL */
	status = WSAStartup(MAKEWORD(1, 1), &Data);
	if (status != 0)
	{
	}
}

void DoTest(int nClints)
{
	const char *ip = "192.168.82.19";//

									 //const char *ip = "192.168.82.201";//127.0.0.1
									 //uint16_t _port = 6620;
									 //const char *ip = "192.168.85.11";//127.0.0.1
	uint16_t _port = 8001;
	//timeMs();
	//xdebug2(TSF"curtime:%_ , @%_", gettickcount(), &s_dns);
#if USE_LOOP_SERVER
	MUdpServerImp *tsi = new MUdpServerImp();
	UdpServer *ts = new UdpServer(_port, tsi);

	uint64_t begin = gettickcount();
	uint64_t end;
	do
	{
		ThreadSleepMS(100);
		end = gettickcount();
	} while (end - begin < 1000);
#endif

	ClientHandler<CAsynUdpClient> ch;
	ch.init();
	//1 Thread 16 ,avg:0.00645 0.009595 0.002736
	//1 Thread 64 ,avg:0.00408 0.003893
	//1 Thread 256 ,avg:0.00534 0.004753
	//1 Thread 512 ,avg:0.004194 0.004234
	//1 Thread 1024 ,avg:0.003456 0.003636

	//2 Thread 128 ,avg:0.012 0.006771 0.009470
	//2 Thread 256 ,avg:0.033 0.006492 0.00777

	//4 Thread 128 ,avg:0.01068 0.01554
	ch.CreateClients(nClints, ip, _port);

	{
		uint64_t begin = gettickcount();
		uint64_t end;
		do
		{
			ThreadSleepMS(1000);
			end = gettickcount();
		} while (end - begin < MAX_TEST_TIME);
	}
	ch.shutdown();

	tsi->doStatistcs();
	//Thread thread(&CreateAsynUdpClient, 1, ip, _port);
	//int startRet = thread.start();

	double ddd = s_TimeMeasure.Dump();
	s_TimeMeasure.Clear();

	printf("main::end ....MAX_THREADS=%d,nClints=%d, TimeMeasure avg = %llf\n", MAX_THREADS, nClints, ddd);

#if USE_LOOP_SERVER
	delete ts;
	delete tsi;
#endif
	ThreadSleepMS(1000);
}

///
void udp_main(int argc, char **argv)
{
	InitSocket();

	AutoBuffer ab(1000);

	//std::string _host_name = "www.baidu.com";

	//static DNS s_dns;
	//std::vector<std::string> ips;
	//s_dns.GetHostByName(_host_name, ips);

	//随着线程数的增多，pingpong明显增加
	const int round = 2;
	for (int i = 0; i < round; ++i)
	{
		DoTest(1);
		DoTest(4);
		DoTest(8);
		//DoTest(16);
		//DoTest(64);
		//DoTest(256);
		//DoTest(512);
	}

	exit(0);
}