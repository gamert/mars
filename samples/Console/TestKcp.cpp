/*
测试 kcp+mars UdpClient
*/

#include "../libknet/kcpclient_task.h"


void send_ping(kcpclient<clitask> &c,int index,bool bTcp = true)
{
	printf("\n");

	KTime t2 = GetKTime();
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
		//Sleep(0);
		std::this_thread::sleep_for(dura);
	}

	c.shutdown();
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}


