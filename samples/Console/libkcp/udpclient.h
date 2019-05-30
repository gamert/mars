#ifndef __UDPCLIENT_H__
#define __UDPCLIENT_H__

#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include <thread>
#include <mutex>
#include "udptask.h"

/*
	
*/
template<typename T /*udptask*/>
class udpclient 
{
public:
	udpclient() :utask(NULL){}
	~udpclient()
	{
		if (utask != NULL)
		{
			delete utask;
		}
	}

	/*
	@addr: ipv4? dns?
	@port:
	@conv: 会话ID，use ID?
	*/
	bool connect(const char *addr, unsigned short int port, IUINT32 conv)
	{
		if (!udpsock.connect(addr, port))
		{
			return false;
		}

		utask = new T;
		if (!utask->init(conv, &udpsock))
		{
			return false;
		}

		isstop = false;

		_thread = std::thread(std::bind(&udpclient::run, this));
		_threadtm = std::thread(std::bind(&udpclient::loop, this));
		return true;
	}

	int sendtcp(const char *buf, int size)
	{
		_mutex.lock();
		int ret = utask->tcp_send(buf, size);
		_mutex.unlock();
		return ret;
	}

	//send udp msg
	int sendUdp(const char  *buf, int len)
	{
		TUdpDatagram_t ab(false);
		IUINT32 conv = utask->GetConv();
		ab.Append(conv);
		ab.Append(buf, len);
		return udpsock.SendTo(ab.data(), ab.size());
	}

	void shutdown()
	{
		isstop = true;
		udpsock.shutdown();
		_thread.join();
		_threadtm.join();
	}

	//处理tcp proxy消息
	void loop()
	{
		//std::chrono::milliseconds dura(1);
		std::chrono::microseconds dura(1);
		for (; !isstop;)
		{
			_mutex.lock();
			utask->timerloop();
			_mutex.unlock();

			_time_measure.Update();

			std::this_thread::sleep_for(dura);//std::chrono::nanoseconds(1)
		}
	}

	//处理udp 网络消息
	void run()
	{
		char buff[65536] = { 0 };
		struct sockaddr_in seraddr;
		for (; !isstop;)
		{
			socklen_t len = sizeof(struct sockaddr_in);
			int size = udpsock.recvfrom(buff, sizeof(buff), (struct sockaddr*)&seraddr, &len);
			if (size == 0)
			{
				continue;
			}
			if (size < 0)
			{
				printf("接收失败 %d,%d \n", udpsock.getsocket(), size);
				continue;
			}
			//time_measure_t::MarkTime("recvfrom");

			const char *pBuf = (const char *)buff;
			if (pBuf[0] == 'U' && pBuf[1] == 'D' && pBuf[2] == 'G')
			{
				if (pBuf[3] == '1')
				{
					_mutex.lock();
					utask->tcp_recv(pBuf + 4, size - 4);
					_mutex.unlock();

				}
				else if (pBuf[3] == '0')
				{
					//_mutex_udp.lock();
					//_udpqueue.push_back(std::string(pBuf+4, _len-4));
					//_mutex_udp.unlock();
					utask->udp_recv(pBuf + 4, size - 4);
				}
				else
				{
					assert(false);
				}
			}
		}
	}

	bool isalive()
	{
		_mutex.lock();
		bool alive = utask->isalive();
		_mutex.unlock();
		return alive;
	}

	virtual int parsemsg(const char *buf, int len)
	{
		printf("收到数据 %s,%d\n", buf, len);
		return 0;
	}
private:
	udpsocket udpsock;
	udptask* utask;		//keep thread safe
	std::thread _thread;
	std::thread _threadtm;
	std::mutex _mutex;
	volatile bool isstop;

	time_measure_t _time_measure;
};

#endif
