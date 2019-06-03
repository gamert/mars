#ifndef __UDPSERVER_H__
#define __UDPSERVER_H__

#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include <thread>
#include <mutex>
#include <map>
#include<vector>
#include "udptask.h"
#ifdef USE_STD_THREAD
#else
	#include "process.h"
#endif


/// Maximum (stack) size to use with _alloca before using new and delete instead.
#ifndef MAX_ALLOCA_STACK_ALLOCATION
#define MAX_ALLOCA_STACK_ALLOCATION 1048576
#endif

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
#ifdef USE_STD_THREAD
		_thread = std::thread(std::bind(&handlethread::loop, this));
#else
		threadHandle = (HANDLE)_beginthreadex(NULL, MAX_ALLOCA_STACK_ALLOCATION * 2, &loop_fun, this, 0, &threadID);
#endif

	}
	void shutdown()
	{
		isstop = true;
#ifdef USE_STD_THREAD
		_thread.join();
#else

#endif
		_mutex.lock();
		for (std::map<IUINT32, udptask*>::iterator iter = clients.begin();
			iter != clients.end(); ++iter)
		{
			delete iter->second;
		}
		_mutex.unlock();
	}
	static unsigned  loop_fun(void *param)
	{
		handlethread<T> *p = (handlethread<T> *)param;
		p->loop();
		return 0;
	}
	void loop()
	{
		//微秒
		std::chrono::microseconds dura(1);
		for (; !isstop;)
		{
			_mutex.lock();
			for (std::map<IUINT32, udptask*>::iterator iter = clients.begin();
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

			_time_measure.Update();

			std::this_thread::sleep_for(dura);
//			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		}
	}

	/*
		收到raw udp报文
	@udpsock:
	@paddr:
	@conv: the user id...
	@buff:
	@size:
	*/
	void recv(SOCKET udpsock, struct sockaddr_in *paddr, IUINT32 conv, const char *buff, int size, bool bTcp)
	{
		udptask *pclient = NULL;
		_mutex.lock();
		std::map<IUINT32, udptask*>::iterator iter = clients.find(conv);
		if (iter == clients.end())
		{
			pclient = new T(conv, udpsock, paddr);
			clients.insert(std::map<IUINT32, udptask*>::value_type(conv, pclient));
		}
		else
		{
			pclient = iter->second;
		}
		if(bTcp)
			pclient->tcp_recv(buff, size);
		else
		{
			pclient->udp_recv(buff, size);
		}

		_mutex.unlock();
	}
public:
	volatile bool isstop;
	std::mutex _mutex;
#ifdef USE_STD_THREAD
	std::thread _thread;
#else
	HANDLE threadHandle;
	unsigned threadID = 0;
#endif
	std::map<IUINT32, udptask*> clients;
	time_measure_t _time_measure;
};


#define MAX_TDNUM 1	//线程个数..

template<typename T>
class udpserver
{
public:

	/*

	@addr:
	@port:
	*/
	bool bind(const char *addr, unsigned int short port)
	{
		if (!udpsock.bind(addr, port))
		{
			return false;
		}

		isstop = false;
		_thread = std::thread(std::bind(&udpserver::run, this));

		maxtdnum = MAX_TDNUM;
		for (unsigned int i = 0; i < maxtdnum; i++)
		{
			handlethread<T>* hthread = new handlethread<T>;
			hthread->init();
			timerthreads.push_back(hthread);
		}

		return true;
	}

	void shutdown()
	{
		isstop = true;
		udpsock.shutdown();
		_thread.join();
		for (typename  std::vector<handlethread<T>*>::iterator iter = timerthreads.begin();
			iter != timerthreads.end(); ++iter)
		{
			(*iter)->shutdown();
			delete *iter;
		}
	}

	void run()
	{
		char buff[65536] = { 0 };
		struct sockaddr_in cliaddr;
		for (; !isstop;)
		{
			socklen_t len = sizeof(struct sockaddr_in);
			int size = udpsock.recvfrom(buff, sizeof(buff), (struct sockaddr*)&cliaddr, &len);
			if (size == 0)
			{
				continue;
			}
			if (size < 0)
			{
				//可能是worldblock?
				printf("接收失败 getsocket=%d,%d \n", udpsock.getsocket(), size);
				continue;
			}
			
			//time_measure_t::MarkTime("recvfrom");

			const char *pBuf = (const char *)buff;
			if (pBuf[0] == 'U' && pBuf[1] == 'D' && pBuf[2] == 'G')
			{
				char *ppbuff = (char *)(pBuf + 4);
				size -= 4;
				IUINT32 conv = ikcp_getconv(ppbuff);
				bool bTcp = pBuf[3] == '1';
				timerthreads[conv % maxtdnum]->recv(udpsock.getsocket(), &cliaddr, conv, ppbuff, size, bTcp);
				{
	//				assert(false);
				}
			}
		}
	}

private:

	udpsocket udpsock;
	std::thread _thread;
	volatile bool isstop;
	unsigned int maxtdnum;
	std::vector<handlethread<T>*> timerthreads;
};

#endif
