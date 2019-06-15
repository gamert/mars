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

typedef TUdpTask<UdpClient> kcptask;

/*
	派生kcp client
	1. ctrl proxy: 直接使用cb函数...
	2. data proxy: 直接使用cb函数...

*/
//template<typename T /*UdpClient*/>
class kcpclient :public kcptask, IAsyncUdpClientEvent
{
public:
	TimeMeasure_t m_TimeMeasure;
	kcpclient() :connect_state(0), m_TimeMeasure(0)
	{
	}
	~kcpclient()
	{
		if (udpsock != NULL)
		{
			delete udpsock;
			udpsock = NULL;
		}
	}

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

		if (!kcptask::init(conv, udpsock))
		{
			return false;
		}

		isstop = false;

		//_thread = std::thread(std::bind(&kcpclient::run, this));
		_threadtm = std::thread(std::bind(&kcpclient::proxy_loop, this));

		//send auth..
		send_connect_auth();

		return true;
	}


	//send tcp msg : control_code
	int sendtcp(const char *buf, int size)
	{
		_mutex.lock();
		int ret = kcptask::tcp_send(buf, size);
		_mutex.unlock();
		return ret;
	}
	
	//send udp msg
	int sendUdp(const char  *buf, int len)
	{
		//未连接成功，不发送..
		if (connect_state != 1)
		{
			return -1;
		};

		char tb[256];
		sprintf(tb, "kcpclient::sendUdp(0x%x,,%d)", this, len);
		time_measure_t::MarkTime(tb);

		TUdpDatagram_t ab(false);
		IUINT32 conv = kcptask::GetConv();
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
			kcptask::timerloop();
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
	//		kcptask::recv(buff, size);
	//		_mutex.unlock();
	//	}
	//}

	bool isalive()
	{
		_mutex.lock();
		bool alive = kcptask::isalive();
		_mutex.unlock();
		return alive;
	}

	//virtual int parsemsg(const char *buf, int len)
	//{
	//	printf("收到数据 %s,%d\n", buf, len);
	//	return 0;
	//}
protected:
	void send_connect_auth()
	{
		IUINT32 conv = kcptask::GetConv();

		KTime t2 = GetKTime();
		char buf[64] = { 0 };
		buf[0] = 0;						//main..
		buf[1] = TF_TYPE_CONNECT_AUTH;	//sub
		memcpy(buf + 2, &conv, sizeof(conv));
		memcpy(buf + 6, &t2, sizeof(t2));
		sendtcp(buf, 2+sizeof(conv) + sizeof(t2));
	}

	//implement: TUdpTask
	KTimeDiff handlePong(const char  *buf, int len, const char *prompt)
	{
		int ping_index;
		KTime t1;
		memcpy(&ping_index, buf + 1, sizeof(int));
		memcpy(&t1, buf + 5, sizeof(t1));
		KTime t2 = GetKTime();
		KTimeDiff dt = GetKTimeDiffSecond(t2, t1);
		printf("[%s]收到PONG[%d] %llf = %lld - %lld\n", prompt, ping_index, dt, t2, t1);
		return dt;
	}
	virtual int udp_recv(const char  *buf, int len)
	{
		buf += 4;
		len -= 4;
		if (buf[0] == 0 && buf[1] == TF_TYPE_PONG)
		{
			handlePong(buf+1, len-1, "UDP");
		}

		return len;
	};
	virtual int udp_send(const char  * buf, int len)
	{
		return 0;
	};

	//处理控制层协议...
	virtual int parsemsg(const char *buf, int len)
	{
		if (buf[0] == 0)
		{
			on_get_control(buf + 1, len - 1);
		}
		else
		{
			//处理数据协议..
			on_get_data(buf, len);
		}
		//if (buf[0] == TF_TYPE_PONG)
		//{

		//	//auto now = get_tick_count();
		//	//if (preTCPRecvTime == 0)
		//	//{
		//	//	preTCPRecvTime = now;
		//	//}
		//	//auto detal = now - preTCPRecvTime;
		//	//preTCPRecvTime = now;
		//	//char temp[256];
		//	//snprintf(temp, sizeof(temp), "1_%d;", detal);
		//	//asioClient->Write(temp);

		//	//logs.push_back(detal);
		//	//if (logs.size() % 200 == 0) {
		//	//	for (size_t i = 0; i < logs.size(); i++)
		//	//	{
		//	//		printf("%d ", logs[i]);
		//	//	}
		//	//	printf("\n");
		//	//}
		//}
		//else
		//{
		//	printf("收到未知数据 %s,%d\n", buf, len);
		//}
		return 0;
	}

	void on_get_control(const char *buf, int len)
	{
		switch (buf[0])
		{
		case TF_TYPE_CONNECT_AUTH:
		{
			IUINT32 conv;
			memcpy(&conv, buf + 1, sizeof(IUINT32));
			KTime t1;
			memcpy(&t1, buf + 5, sizeof(t1));
			KTime t2 = GetKTime();
			KTimeDiff dt = GetKTimeDiffSecond(t2, t1);
			printf("[%s]收到TF_TYPE_CONNECT_AUTH[%d] %llf = %lld - %lld\n", "parsemsg", conv, dt, t2, t1);

		}

		break;
		case TF_TYPE_CONNECT_AUTH_RES:
		{
			IUINT32 conv;
			memcpy(&conv, buf + 1, sizeof(IUINT32));
			KTime t1;
			memcpy(&t1, buf + 5, sizeof(t1));
			KTime t2 = GetKTime();
			KTimeDiff dt = GetKTimeDiffSecond(t2, t1);
			printf("[%s]收到TF_TYPE_CONNECT_AUTH_RES[%d] %llf = %lld - %lld\n", "parsemsg", conv, dt, t2, t1);

			set_connect_state(1);
		}
		break;

		case TF_TYPE_PONG:
		{
			KTimeDiff dd = handlePong(buf, len, "TCP");
			m_TimeMeasure.m_pings.push_back(dd);
			if ((m_TimeMeasure.m_pings.size() % 10) == 9)
			{
				m_TimeMeasure.Dump("clitask ping", true);
			}
		}
		break;

		default:
			break;
		}
	}

	void on_get_data(const char *buf, int len)
	{
		_mutex_udp.lock();
		_udpqueue.push_back(std::string(buf, len));
		_mutex_udp.unlock();
	}

protected:
	//
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
				kcptask::tcp_recv(pBuf + 4, _len - 4);
				_mutex.unlock();

			}
			else if (pBuf[3] == '0')
			{
				udp_recv(pBuf + 4, _len - 4);
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

protected:
	UdpClient *udpsock;
	//std::thread _thread;
	std::thread _threadtm;
	std::mutex _mutex;
	volatile bool isstop;
	volatile int connect_state;	//链接状态
	void set_connect_state(int new_state)
	{
		connect_state = new_state;
	}

	//
	std::mutex _mutex_udp;
	std::list<std::string> _udpqueue;	//

	time_measure_t _time_measure;
};

#endif
