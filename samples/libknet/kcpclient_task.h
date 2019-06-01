#pragma once

#include "TimeMeasure.h"
#include "kcpclient.h"


#define TF_TYPE_BEGIN	1
#define TF_TYPE_DATA	2
#define TF_TYPE_END 	3

//计算ping-pong
#define TF_TYPE_PING 	127
#define TF_TYPE_PONG 	126

//client-server session:
class clitask :public kcptask
{
public:
	TimeMeasure_t m_TimeMeasure;
	clitask() : m_TimeMeasure(0)
	{

	}

	uint64_t handlePong(const char  *buf, int len,const char *prompt)
	{
		int ping_index;
		uint64_t t1;
		memcpy(&ping_index, buf + 1, sizeof(int));
		memcpy(&t1, buf + 5, sizeof(t1));
		uint64_t t2 = timeUs();
		//std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		//if (time_span.count() >= 1)
		//double dd = time_span.count();
		printf("[%s]收到PONG[%d] %lld\n", prompt, ping_index, t2 - t1);
		return t2 - t1;
	}

	virtual int udp_recv(const char  *buf, int len)
	{
		buf += 4;
		len -= 4;
		if (buf[0] == TF_TYPE_PONG)
		{
			handlePong(buf,len,"UDP");
		}

		return len;
	};
	virtual int udp_send(const char  * buf, int len)
	{
		return 0;
	};

	virtual int parsemsg(const char *buf, int len)
	{
		if (buf[0] == TF_TYPE_PONG)
		{
			uint64_t dd = handlePong(buf, len, "TCP");
			m_TimeMeasure.m_pings.push_back(dd*0.000001);
			if ((m_TimeMeasure.m_pings.size() % 10) == 9)
			{
				m_TimeMeasure.Dump("clitask ping", true);
			}

			//auto now = get_tick_count();
			//if (preTCPRecvTime == 0)
			//{
			//	preTCPRecvTime = now;
			//}
			//auto detal = now - preTCPRecvTime;
			//preTCPRecvTime = now;
			//char temp[256];
			//snprintf(temp, sizeof(temp), "1_%d;", detal);
			//asioClient->Write(temp);

			//logs.push_back(detal);
			//if (logs.size() % 200 == 0) {
			//	for (size_t i = 0; i < logs.size(); i++)
			//	{
			//		printf("%d ", logs[i]);
			//	}
			//	printf("\n");
			//}
		}
		else
		{
			printf("收到未知数据 %s,%d\n", buf, len);
		}
		return 0;
	}
};
