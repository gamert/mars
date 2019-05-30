/*
测试 kcp+mars UdpClient
*/

#include "kcpclient.h"
#include "TimeMeasure.h"

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
	clitask(): m_TimeMeasure(0)
	{

	}
	virtual int udp_recv(const char  *buf, int len)
	{
		buf += 4;
		len -= 4;
		if (buf[0] == TF_TYPE_PONG)
		{
			int ping_index;
			std::chrono::steady_clock::time_point t1;
			memcpy(&ping_index, buf + 1, sizeof(int));
			memcpy(&t1, buf + 5, sizeof(t1));
			std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
			std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
			//if (time_span.count() >= 1)
			double dd = time_span.count();
			printf("[UDP]收到PONG[%d] %llf\n", ping_index, dd);
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
			int ping_index;
			std::chrono::steady_clock::time_point t1;
			memcpy(&ping_index, buf + 1, sizeof(int));
			memcpy(&t1, buf + 5, sizeof(t1));
			std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
			std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
			//if (time_span.count() >= 1)
			double dd = time_span.count();
			printf("[TCP]收到PONG[%d] %llf\n", ping_index, dd);

			m_TimeMeasure.m_pings.push_back(dd);
			if ((m_TimeMeasure.m_pings.size() % 10) == 9)
			{
				m_TimeMeasure.Dump("clitask ping",true);
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


void send_ping(kcpclient<clitask> &c,int index,bool bTcp = true)
{
	printf("\n");

	std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
	char buf[64] = { 0 };
	buf[0] = TF_TYPE_PING;
	memcpy(buf + 1, &index, sizeof(index));
	memcpy(buf + 5, &t2, sizeof(t2));

	if(bTcp)
		c.sendtcp(buf, sizeof(t2) + 10);
	else
		c.sendUdp(buf, sizeof(t2) + 10);
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("%s ip 文件名\n", argv[0]);
		return -1;
	}
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	//timeBeginPeriod(1); //设置精度为1毫秒

	srand((unsigned)time(NULL));

	//boost::asio::io_context io_context;
	//std::shared_ptr<AsioClient> asioClient = InitAsioClient(io_context, "127.0.0.1", "3333");

	//rand
	kcpclient<clitask> c;
	c.connect(argv[1], 9001, rand());

	int ping_index = 1;
	//测试发送文件...
	//int res = test_send_file(argv[2],c);
	//if (res == -1)
	//	return -1;
	send_ping(c, ping_index++);

	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	std::chrono::milliseconds dura(1);
	while (c.isalive())
	{
		std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		if (time_span.count() >= 2)
		{
			send_ping(c, ping_index++, (ping_index % 2) == 0);

			t1 = t2;
		}

		std::this_thread::sleep_for(dura);
	}

	c.shutdown();
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}


