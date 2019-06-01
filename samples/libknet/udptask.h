#ifndef __UDPTASK_H__
#define __UDPTASK_H__

#include <stdio.h>
#include "ikcp.h"
#include "common.h"
#include "udpsocket.h"

#include "MyAutoBuffer.h"

#define TIME_MEASURE 1

//#define USE_chrono

#ifdef USE_chrono
	#define KTime		std::chrono::steady_clock::time_point
	#define GetKTime	std::chrono::steady_clock::now
	#define KTimeDiff	double
	#define GetKTimeDiffSecond(t2,t1) std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count()
#else
	#define KTime		uint64_t
	#define GetKTime	timeUs
	#define KTimeDiff	double
	#define GetKTimeDiffSecond(t2,t1) (t2-t1)*0.000001

#endif


class time_measure_t
{
public:
#define MEASURE_NUM 3000

	std::chrono::duration<double> time_span;
	int count;
	std::chrono::steady_clock::time_point t1;
	time_measure_t() :count(0)
	{
		t1 = std::chrono::steady_clock::now();
	}

	void Update()
	{
#if TIME_MEASURE
		//std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
		//time_span += std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		//t1 = t2;
		//count++;
		//if (count >= MEASURE_NUM)
		//{
		//	printf("time_measure_t::Update: %llf \n", time_span.count() / MEASURE_NUM);
		//	time_span = std::chrono::duration<double>(0);
		//	count = 0;
		//}
#endif
	}

	static void MarkTime(const char *prefix, char *buf = NULL)
	{
#if TIME_MEASURE
		//static 	std::chrono::steady_clock::time_point last_t1 = std::chrono::steady_clock::now();
		//std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
		////std::time_t tt;
		////tt = system_clock::to_time_t(today);
		//std::chrono::duration<double> dt = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - last_t1);
		//if (buf)
		//	sprintf(buf, "MarkTime:%s: %lld,  dt = %llf\n", prefix, t2, dt);
		//else
		//	printf("%s:MarkTime: %lld,  dt = %llf\n", prefix, t2, dt);
		//last_t1 = t2;
		//printf("%s:MarkTime:", prefix);
#endif
	}

};

/*
	inner proxy :
*/
struct TUdpDatagram_t:AutoBuffer_t
{
	TUdpDatagram_t(bool tcp = false)
	{
		if(tcp)
			Append("UDG1",4);
		else 
			Append("UDG0", 4);
	}
};


/*
	a control session:
*/
#define THINK_INTERVAL 2000			//microsecond def:10000 us
#define TIMEOUT_INTERVAL 1000000*5	//microsecond 
template<typename TSocket>
class TUdpTask
{
public:
	TUdpTask()
	{
		conv = 0;
		kcp = NULL;
		current = 0;
		nexttime = 0;
		alivetime = nexttime + TIMEOUT_INTERVAL;
		memset(buffer, 0, sizeof(buffer));
	}

	~TUdpTask()
	{
		if (kcp != NULL)
		{
			ikcp_flush(kcp);
			ikcp_release(kcp);
		}
		printf("关闭连接 %d\n", conv);
	}

	IUINT32 GetConv() {
		return conv;
	}

	/*
		@conv:
		@udpsock:
		*/
	bool init(IUINT32 conv, TSocket *udpsock, int mode = 2)
	{
		if (udpsock == NULL)
		{
			return false;
		}
		this->conv = conv;
		this->nexttime = 0;

		kcp = ikcp_create(conv, (void*)udpsock);

		kcp->output = &TUdpTask::udp_output;
		//kcp->logmask = 0xffff;
		//kcp->writelog = &TUdpTask::writelog;

		ikcp_wndsize(kcp, 128, 128);

		switch (mode)
		{
		case 0:
			// 默认模式
			ikcp_nodelay(kcp, 0, 10, 0, 0);
			break;
		case 1:
			// 普通模式，关闭流控等
			ikcp_nodelay(kcp, 0, 10, 0, 1);
			break;
		case 2:
			// 启动快速模式
			// 第二个参数 nodelay-启用以后若干常规加速将启动
			// 第三个参数 interval为内部处理时钟，默认设置为 10ms
			// 第四个参数 resend为快速重传指标，设置为2
			// 第五个参数 为是否禁用常规流控，这里禁止
			//ikcp_nodelay(kcp, 0, 10, 0, 0);
			//ikcp_nodelay(kcp, 0, 10, 0, 1);
			//ikcp_nodelay(kcp, 1, 10, 2, 1);
			ikcp_nodelay(kcp, 1, 1, 2, 1); // 设置成1次ACK跨越直接重传, 这样反应速度会更快. 内部时钟5毫秒.

			kcp->rx_minrto = 10;
			kcp->fastresend = 1;

			//kcp->interval = 1;
			break;
		default:
			printf("%d,%d 模式错误!\n", conv, mode);
		}

		printf("新建连接 %d\n", conv);
		return true;
	}

	/*
		接收，同时设定超时时间...
	*/
	int tcp_recv(const char  * buf, int len)
	{
		int nret = ikcp_input(kcp, buf, len);
		if (nret == 0)
		{
			nexttime = iclock();
			alivetime = nexttime + TIMEOUT_INTERVAL;
		}
		return nret;
	}

	//可靠报文...
	int tcp_send(const char  * buf, int len)
	{
		int nret = ikcp_send(kcp, buf, len);
		if (nret == 0)
		{
			nexttime = iclock();
			alivetime = nexttime + TIMEOUT_INTERVAL;
		}
		ikcp_flush(kcp);
		//printf("发送数据 %d,%d,%d\n", conv, len, nret);
		return nret;
	}

	/*
		for ikcp_update
	*/
	void timerloop()
	{
		current = iclock();
		if (nexttime > current)
		{
			return;
		}

		//1ms
		nexttime = ikcp_check(kcp, current);
		if (nexttime != current)
		{
			return;
		}

		ikcp_update(kcp, current);

		while (true) {
			int nrecv = ikcp_recv(kcp, buffer, sizeof(buffer));
			if (nrecv < 0)
			{
				if (nrecv == -3)
				{
					printf("buffer太小 %d,%d\n", conv, sizeof(buffer));
				}
				break;
			}
			parsemsg(buffer, nrecv);
		}
	}

public:
	virtual bool isalive()
	{
		return alivetime > current;
	}
	virtual int parsemsg(const char *buf, int len) = 0;

	//
	virtual int udp_recv(const char  * buf, int len) = 0;
	virtual int udp_send(const char  * buf, int len) = 0;
	
private:
	static int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
	{
		//time_measure_t::MarkTime("udp_output");
		TUdpDatagram_t ab(true);
		ab.Append(buf, len);
		return ((TSocket*)user)->SendTo(ab.data(), ab.size());
	}

	static void writelog(const char *log, struct IKCPCB *kcp, void *user)
	{
		printf("%s\n", log);
	}

protected:
	IUINT32 conv;
	ikcpcb *kcp;
	IUINT32 nexttime;
	IUINT32 current;
	IUINT32 alivetime;
	char buffer[65536];
};


//定义为
typedef TUdpTask<udpsocket> udptask;



#endif
