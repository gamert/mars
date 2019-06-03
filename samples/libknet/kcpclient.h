#ifndef __KCPCLIENT_H__
#define __KCPCLIENT_H__


#include "mars/comm/thread/lock.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/bootrun.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/dns/dns.h"

#include "mars/comm/socket/udpclient.h"

#include <thread>
#include <mutex>
#include "udptask.h"

//
typedef TUdpTask<UdpClient> kcptask;
/*
	
*/
template<typename T /*udptask*/>
class kcpclient : IAsyncUdpClientEvent
{
public:
	kcpclient() :utask(NULL){}
	~kcpclient()
	{
		if (utask != NULL)
		{
			delete utask;
		}
		if (udpsock != NULL)
		{
			delete udpsock;
			udpsock = NULL;
		}
	}
	
	virtual void OnError(UdpClient* _this, int _errno)
	{
		char buf[256];
		sprintf(buf, "kcpclient::OnError(0x%x,%d)", _this, _errno);
		//time_measure_t::MarkTime(buf);
	};
	
	//raw message:
	virtual void OnDataGramRead(UdpClient* _this, void* _buf, size_t _len)
	{
		const char *pBuf = (const char *)_buf;

		//要判断是否为普通UDP?
		if (pBuf[0] == 'U' && pBuf[1] == 'D' && pBuf[2] == 'G')
		{
			if (pBuf[3] == '1')
			{
				_mutex.lock();
				utask->tcp_recv(pBuf + 4, _len - 4);
				_mutex.unlock();

			}
			else if (pBuf[3] == '0')
			{
				//_mutex_udp.lock();
				//_udpqueue.push_back(std::string(pBuf+4, _len-4));
				//_mutex_udp.unlock();
				utask->udp_recv(pBuf + 4, _len - 4);
			}
			else
			{
				assert(false);
			}
		}
	};

	virtual void OnDataSent(UdpClient* _this)
	{
		char buf[256];
		sprintf(buf, "kcpclient::OnDataSent(0x%x)", _this);
		//time_measure_t::MarkTime(buf);
	};


	/*
	@addr: ipv4? dns?
	@port:
	@conv: 会话ID，use ID?
	*/
	bool connect(const char *addr, unsigned short int port, IUINT32 conv)
	{
		udpsock = new UdpClient(addr, port, this);		
		//if (!udpsock.connect(addr, port))
		//{
		//	return false;
		//}

		utask = new T;
		if (!utask->init(conv, udpsock))
		{
			return false;
		}

		isstop = false;

		//_thread = std::thread(std::bind(&kcpclient::run, this));
		_threadtm = std::thread(std::bind(&kcpclient::proxy_loop, this));
		return true;
	}

	//send tcp msg
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
		char tb[256];
		sprintf(tb, "kcpclient::sendUdp(0x%x,,%d)", this, len);
		time_measure_t::MarkTime(tb);

		TUdpDatagram_t ab(false);
		IUINT32 conv = utask->GetConv();
		ab.Append(conv);
		ab.Append(buf, len);
		return udpsock->SendTo(ab.data(), ab.size());
	}

	void shutdown()
	{
		isstop = true;
		//udpsock->shutdown();
		//_thread.join();
		_threadtm.join();

		if (udpsock != NULL)
		{
			delete udpsock;
			udpsock = NULL;
		}
		//if (udpsock != NULL)
		//{
		//	udpsock->Break();
		//}
	}

	//处理tcp proxy消息
	void proxy_loop()
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
	//void run()
	//{
	//	char buff[65536] = { 0 };
	//	struct sockaddr_in seraddr;
	//	for (; !isstop;)
	//	{
	//		socklen_t len = sizeof(struct sockaddr_in);
	//		int size = udpsock.recvfrom(buff, sizeof(buff), (struct sockaddr*)&seraddr, &len);
	//		if (size == 0)
	//		{
	//			continue;
	//		}
	//		if (size < 0)
	//		{
	//			printf("接收失败 %d,%d \n", udpsock.getsocket(), size);
	//			continue;
	//		}
	//		time_measure_t::MarkTime("recvfrom");

	//		_mutex.lock();
	//		utask->recv(buff, size);
	//		_mutex.unlock();
	//	}
	//}

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
protected:
	UdpClient *udpsock;
	kcptask* utask;		//keep thread safe
	//std::thread _thread;
	std::thread _threadtm;
	std::mutex _mutex;
	volatile bool isstop;

	//
	std::mutex _mutex_udp;
	std::list<std::string> _udpqueue;	//

	time_measure_t _time_measure;
};

#endif
