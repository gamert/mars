#include "../../udpserver.h"



//server-client session:
class CServerClientSession :public udptask
{
public:
	/*
		@conv: client id
		@usocket:client sock
		@paddr: client addr
	*/
	CServerClientSession(IUINT32 conv, SOCKET usocket, struct sockaddr_in *paddr)
	{
		udpsock = new udpsocket(usocket, paddr);
		udptask::init(conv, udpsock);
		closeit = false;
		fp = NULL;
		nexttime = 0;
		recvsize = 0;
	}
	~CServerClientSession()
	{
		if (udpsock != NULL){
			delete udpsock;
		}
		if (fp != NULL){
			fclose(fp);
		}
	}
	virtual bool isalive()
	{
		if (closeit)
		{
			return false;
		}
		return udptask::isalive();
	}

	//
	virtual int udp_recv(const char  *buf, int len)
	{
		buf += 4;
		len -= 4;

		if (buf[0] == 0)
		{
			on_get_control_udp(buf, len);
		}
		
		mark_alivetime();

		return len;
	};

	virtual int udp_send(const char  * buf, int len)
	{
		//time_measure_t::MarkTime("==udp_send ");

		TUdpDatagram_t ab(false);
		IUINT32 conv = this->GetConv();
		ab.Append(conv);
		ab.Append(buf, len);
		return udpsock->SendTo(ab.data(), ab.size());
	};

	/*
		tcp message
	*/
	virtual int parsemsg(const char *buf, int len)
	{
		if (len < 2)
		{
			printf("数据错误 %s,%d\n", buf, len);
			return -1;
		}
		if (buf[0] == 0)
		{
			return on_get_control_tcp(buf, len);
		}
		else
		{

		}

		return 0;
	}

	//
	int on_get_control_tcp(const char *buf, int len)
	{
		switch (buf[1])
		{
		case TF_TYPE_BEGIN:
		{
			const char *filename = buf + 2;
			fp = fopen(filename, "wb");
			if (fp == NULL)
			{
				printf("创建文件失败 %s \n", filename);
				return -1;
			}
			printf("创建文件 %s \n", filename);
		}
		break;
		case TF_TYPE_DATA:
		{
			if (fp == NULL)
			{
				printf("未创建文件 %d \n", len);
				return -1;
			}
			while (fwrite(buf + 2, len - 2, 1, fp) != 1)
			{
			}
			recvsize += len;
			IUINT32 current = iclock();
			if (nexttime < current)
			{
				printf("%.2f M/S\r", float(recvsize) / (1024 * 1024));
				recvsize = 0;
				nexttime = current + 1000;
			}
		}
		break;
		case TF_TYPE_END:
		{
			if (fp != NULL)
			{
				fclose(fp);
			}
			printf("接收文件完成 \n");
			closeit = true;
		}
		break;
		case TF_TYPE_PING:
		{
			//time_measure_t::MarkTime("==tcp_recv TF_TYPE_PING");

			char buf2[64];
			memcpy(buf2, buf, len);
			buf2[1] = TF_TYPE_PONG;
			this->tcp_send(buf2, len);
			//printf("接收文件完成 \n");
		}
		break;
		case TF_TYPE_CONNECT_AUTH:
		{
			//time_measure_t::MarkTime("==tcp_recv TF_TYPE_PING");
			IUINT32 conv;
			memcpy(&conv, buf + 2, sizeof(IUINT32));

			KTime t2 = GetKTime();
			char buf2[64];
			//memcpy(buf2, buf, len);
			buf2[0] = 0;
			buf2[1] = TF_TYPE_CONNECT_AUTH_RES;
			memcpy(buf2 + 2, &conv, sizeof(conv));
			memcpy(buf2 + 6, &t2, sizeof(t2));
			this->tcp_send(buf2, 1 + sizeof(conv) + sizeof(t2));
			//printf("接收文件完成 \n");
		}
		break;
		default:
			printf("指令错误 %s,%d\n", buf, len);
		}
		return 0;
	}

	void on_get_control_udp(const char *buf, int len)
	{
		switch (buf[1])
		{
			case TF_TYPE_PING:
			{
				//time_measure_t::MarkTime("==udp_recv TF_TYPE_PING");
				int ping_index;
				KTime t1;
				memcpy(&ping_index, buf + 2, sizeof(int));
				memcpy(&t1, buf + 6, sizeof(t1));
				KTime t2 = GetKTime();
				KTimeDiff dt = GetKTimeDiffSecond(t2, t1);
				printf("[%s]收到PING[%d] %llf = %lld - %lld\n", "udp_recv", ping_index, dt, t2, t1);

				char buf2[64];
				memcpy(buf2, buf, len);
				buf2[1] = TF_TYPE_PONG;
				this->udp_send(buf2, len);
				//printf("接收文件完成 \n");
			}
		}

	}

private:
	udpsocket *udpsock;
	bool closeit;
	FILE *fp;
	IUINT32 nexttime;
	int recvsize;
};

int main(int argc, char *argv[])
{
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

	const char *ip = "0.0.0.0";
	int port = 9001;

	udpserver<CServerClientSession> s;
	s.bind(ip, port);

	printf("server: %s, %d\n",ip,port);

	std::chrono::milliseconds dura(10);
	for (;;)
	{
		std::this_thread::sleep_for(dura);
	}

	s.shutdown();
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}
