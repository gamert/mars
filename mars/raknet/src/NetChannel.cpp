#include "NetChannel.h"
#ifdef USE_AntiBot
#include "AntiBot.h"
#endif
#include "MyAutoBuffer.h"

Channel_t::Channel_t() :m_TcpClient(NULL), m_remaining(0), m_pINetEvent(0)
{
}

Channel_t::~Channel_t()
{
	m_pINetEvent = NULL;
	Close();
}


int		Channel_t::IsConnectTo(const char *szIP, int port)
{
	if (m_TcpClient)
	{
		if (strcmp(m_TcpClient->GetIP(), szIP) == 0 && port == m_TcpClient->GetPort())
			return true;
	}
	return 0;
	//return (Socket && port == iPort && szServerName == szIP);
};

int		Channel_t::Connect(const char *szIP, int port, INetEvent *p)
{
	m_pINetEvent = p;
	m_TcpClient = new TcpClient(szIP, port, *this);
	return m_TcpClient->Connect();
};

void	Channel_t::Close()
{
	if (m_TcpClient)
	{
		delete m_TcpClient;
		m_TcpClient = NULL;
	}
}

//主线程call
int		Channel_t::Send(const char *sendMsg, int sendLen, int nEncrytionParam)
{
	if (m_TcpClient && m_TcpClient->GetTcpStatus() == TcpClient::kTcpConnected)
	{
		return ProcessOutgoingPackage(sendMsg, sendLen);
	}
	{
		ScopedLock lock(m_mutex_);
		m_mes.push_back(new MES(sendMsg, sendLen));
	}
	return -1;
}
const char *Channel_t::GetIP()
{
	assert(m_TcpClient);
	return m_TcpClient->GetIP();
}
int Channel_t::GetPort()
{
	assert(m_TcpClient);
	return m_TcpClient->GetPort();
}



//发送暂存数据
int Channel_t::PurgeOutbuf()
{
	//size_t _remaining = GetOutputLength();
	//if (_remaining > 0)
	//{
	//	std::string strBuf;
	//	strBuf.resize(_remaining);
	//	if (_remaining)
	//		obuf.Read((char*)strBuf.data(), _remaining);
	//	return _Send(strBuf.data(), _remaining);

		//// move data from m_mes to immediate output buffer
		//obuf.Space() && 
		while (!m_mes.empty())
		{
			MES *p;
			{
				ScopedLock lock(m_mutex_);
				ucharp_v::iterator it = m_mes.begin();
				p = *it;
				m_mes.erase(it);
			}
			//if ( p -> left())
			assert(p && p->left());
			{
				ProcessOutgoingPackage(p->curbuf(), p->left());
				delete p;
			}
			//else
			//{
			//	size_t sz = obuf.Space();
			//	obuf.Write(p -> curbuf(),sz);
			//	p -> ptr += sz;
			//}
		}
	//}
	return 0;
};

void Channel_t::OnConnect()
{
	DevLog("%s[%d] : ", __FUNCTION__, __LINE__);
	//tc->Write("abcd", 4);

	//SetConnected(true);
	PurgeOutbuf();

	if (m_pINetEvent)
	{
		m_pINetEvent->OnConnect();
	}
};
void Channel_t::OnDisConnect(bool _isremote)
{
	DevLog("%s[%d] : ", __FUNCTION__, __LINE__);
	if (m_pINetEvent)
	{
		m_pINetEvent->OnDisConnect(_isremote);
	}
};
void Channel_t::OnError(int _status, int _errcode)
{
	const char *ppStatus[] = { "kTcpInit",
		"kTcpInitErr",
		"kSocketThreadStart",
		"kSocketThreadStartErr",
		"kTcpConnecting",
		"kTcpConnectIpErr",
		"kTcpConnectingErr",
		"kTcpConnectTimeoutErr",
		"kTcpConnected",
		"kTcpIOErr",
		"kTcpDisConnectedbyRemote",
		"kTcpDisConnected" };

	DevLog("%s[%d] : _status=%s,_errcode=%d", __FUNCTION__, __LINE__, ppStatus[_status], _errcode);
	if (m_pINetEvent)
	{
		m_pINetEvent->OnError(_status, _errcode);
	}
};

//...
void Channel_t::OnWrote(int _id, unsigned int _len)
{
	DevLog("%s[%d] : ", __FUNCTION__, __LINE__);
	//PurgeOutbuf();
	if (m_pINetEvent)
	{
		m_pINetEvent->OnWrote(_id, _len);
	}
};
void Channel_t::OnAllWrote()
{
	DevLog("%s[%d] : ", __FUNCTION__, __LINE__);
	if (m_pINetEvent)
	{
		m_pINetEvent->OnAllWrote();
	}
};

//[net thread]
void Channel_t::OnRead()
{
	//char buf[128];
	unsigned int _len = Read();
	DevLog("%s[%d] : _len=%d ", __FUNCTION__, __LINE__, _len);

	ProcessIncoming();
	if (m_pINetEvent)
	{
		m_pINetEvent->OnRead();
	}
};


//[net thread]: 读取时，一次性读空？
int Channel_t::Read()
{
#define READ_BUF_SIZE 3800

	char	buf[READ_BUF_SIZE];

	uint32 dwRecvBytes = m_TcpClient->Read(buf, sizeof(buf));
	if (dwRecvBytes > 0 && dwRecvBytes <= READ_BUF_SIZE)
	{

#ifdef CHANNEL_LOG
		sLog.HexLog("In:", buf, dwRecvBytes);
#endif
		m_bytes_received += dwRecvBytes;
		//if (GetTrafficMonitor())
		//{
		//	GetTrafficMonitor() -> fwrite(buf, 1, n);
		//}
		if (m_b_input_buffer_disabled || ibuf.Write(buf, dwRecvBytes) == false)
		{
#ifdef CHANNEL_LOG
			sLog.Log("[%s][%d] ibuf.Write error.", __FUNCTION__, __LINE__);
#endif		
			DevLog("%s[%d] :ibuf.Write error.", __FUNCTION__,__LINE__);

			assert(false && "ibuf.Write(buf,dwRecvBytes) == false");

			m_mes_in.push_back(new MES(buf, dwRecvBytes));
			//			Handler().LogError(this, "OnRead", 0, "ibuf overflow", LOG_LEVEL_WARNING);
		}
		return dwRecvBytes;
	}
	else
	{
		assert(false);
		PlatLog("E:[%d]:%d", __LINE__, dwRecvBytes);
	}

	return 0;
};

//处理发出报文；
//是否加密由低层自动跟据服务器是否设置了加密解密函数决定；切换连接的时候，自动清除加解密函数；
int	Channel_t::ProcessOutgoingPackage(const char * szPackuf, uint32 len)
{
#ifdef CHANNEL_LOG
	static int s_OutgoingPackages = 1;
	sLog.Log("[%s][Block=%d][IsConnected=%d][Seq=%d]len =%d\n", __FUNCTION__, m_iBlockChannel, IsConnected(), s_OutgoingPackages++, len);
	sLog.HexLog("Out:", szPackuf, len);
#endif

//	if (m_iBlockChannel)
//	{
//#ifdef  CHANNEL_LOG
//		std::string out;
//		int res = HexBuffer((char *)szPackuf, len, out);
//		if (res < 1000)
//		{
//			DevLog("[%s][Block=%d][IsConnected=%d] len =%u; %s\n", __FUNCTION__, m_iBlockChannel, IsConnected(), len, out.c_str());
//		}
//#endif
//		return 0;
//	}

	//if (m_fnHookSend)
	//{
	//	m_fnHookSend(0, szPackuf, len, 0);
	//}
	//if(bNeed)		// 需要加密的消息
	//{	
	//}
	//else
	// 不需要加密或者已经处理过加密的消息
	AutoBuffer_t strBufIn;//,strBufOut;
	strBufIn.EnsureBuffer(len);

	//ClientPktHeader pkt;
	//memcpy(&pkt,szPackuf,sizeof(ClientPktHeader));

//#ifdef CHANNEL_LOG
//	sLog.HexLog("OutgoingPackage0",szPackuf,HEADERSIZE);
//#endif	
#ifdef USE_NetCrypt
	_crypt.EncryptSend((unsigned char *)szPackuf, HEADERSIZE, (unsigned char *)strBufIn.data(), HEADERSIZE);
#endif
	//#ifdef CHANNEL_LOG
	//	sLog.HexLog("OutgoingPackage1",strBufIn.data(),HEADERSIZE);
	//#endif	
	if (len > HEADERSIZE)
	{
		memcpy(strBufIn.data() + HEADERSIZE, szPackuf + HEADERSIZE, len - HEADERSIZE);
		//		_crypt.EncryptMsgBody(strBufIn.data() + HEADERSIZE,len - HEADERSIZE);
#ifdef USE_NetCrypt
		_crypt.EncryptMsgBody_LoopGroup(strBufIn.data() + HEADERSIZE, len - HEADERSIZE, strBufIn.data(), HEADERSIZE);
#endif
	}

	//if(pkt.cmd == (0x0116| 0x4000))
	//{
	//	int nSend1 = len/2;
	//	int nSend2 = len - nSend1;
	//	const char *pSend1 = strBufIn.data();
	//	const char *pSend2 = strBufIn.data() + nSend1;
	//	IsConnected() ? _Send(pSend1,nSend1) : _AsynSend(pSend1,nSend1);
	//	IsConnected() ? _Send(pSend2,nSend2) : _AsynSend(pSend2,nSend2);
	//	return len;
	//}

	if (m_TcpClient)
	{
		return m_TcpClient->Write(strBufIn.data(), len);
	}
	//_Send() ;
	return -1;
}


//[net thread]:
void	Channel_t::ProcessIncoming()
{
Channel_BEGIN:
	while (1)
	{
		//- Read the packet header and decipher it (if needed)
		if (!m_remaining)
		{
			if (ibuf.GetLength() < HEADERSIZE)
				break;

			ibuf.Read((char *)&m_hdr, HEADERSIZE);

#ifdef CHANNEL_LOG
			sLog.HexLog("IncomingPackage0", (char *)&m_hdr, HEADERSIZE);
#endif
			ServerPktHeader hdr;
#ifdef USE_NetCrypt
			_crypt.DecryptRecv((uint8 *)&m_hdr, HEADERSIZE, (uint8 *)&hdr, HEADERSIZE);
#endif

#ifdef CHANNEL_LOG
			sLog.HexLog("IncomingPackage1", (char *)&hdr, HEADERSIZE);
#endif
			if (hdr.size == 0)
			{
				PlatLog("E:%d", __LINE__);
				assert(false && "hdr.size == 0");
				ibuf.Reset();
				continue;
			}
			m_remaining = (hdr.size) - HEADERSIZE;
			m_cmd = hdr.cmd;
			m_subCmd = hdr.cmd2;
		}
		if (ibuf.GetLength() < m_remaining)
		{
			//缓冲区错误
			if (m_remaining > ibuf.GetMax())
			{
				PlatLog("E:%d", __LINE__);
				assert(false && "m_remaining > ibuf.GetMax()");
				m_remaining = 0;
				ibuf.Reset();
			}
			break;
		}

		int nRawSize = m_remaining + HEADERSIZE;
		//原始报文--
		AutoBuffer_t strBufIn;//,strBufOut;
		char *pBuf = strBufIn.EnsureBuffer(nRawSize);

		{
			//还原完整报文--
			*((uint16*)pBuf) = nRawSize;
			*((uint16*)(pBuf + 2)) = m_cmd;
			*((uint16*)(pBuf + 4)) = m_subCmd;
			if (m_remaining)
			{
				ibuf.Read(pBuf + HEADERSIZE, m_remaining);
#ifdef USE_NetCrypt
				//				_crypt.DecryptMsgBody(pBuf+HEADERSIZE, m_remaining);
				_crypt.DecryptMsgBody_LoopGroup(pBuf + HEADERSIZE, m_remaining, (char *)&m_hdr, HEADERSIZE);
#endif
			}
#ifdef CHANNEL_LOG
			sLog.HexLog("IncomingPackage2", (char *)pBuf, nRawSize);
#endif
			OnPackage(pBuf, nRawSize);
		}
		m_remaining = 0;
	}
	while (m_mes_in.empty() == false)
	{
		while (ibuf.Space() > 0)
		{
			ucharp_v::iterator it = m_mes_in.begin();
			MES *p = *it;
			if (ibuf.Space() > p->left())
			{
				ibuf.Write(p->curbuf(), p->left());
				delete p;
				m_mes_in.erase(it);
			}
			else
			{
				size_t sz = ibuf.Space();
				ibuf.Write(p->curbuf(), sz);
				p->ptr += sz;
			}
		}
		goto Channel_BEGIN;
	}
};

//报文
void Channel_t::OnPackage(char *pPack, int nPackLen)
{

#ifdef USE_AntiBot
	GetAntiBot().HookPackage(pPack, nPackLen);
#endif
	if (!pPack || nPackLen == 0)
	{
		return;
	}

	ServerPktHeader *pHead = (ServerPktHeader *)pPack;
	char *pMsg = pPack + (HEADERSIZE - 2);
	int len = (pHead->size) - (HEADERSIZE - 2);

	switch (pHead->cmd)
	{
#ifdef USE_AntiBot
	case 0x0030:	//E_CMD_ANTI_BOT
		//必须是去掉6Bytes报文头的部分
		GetAntiBot().OnRcvAntiData((BYTE *)pPack + HEADERSIZE, nPackLen - HEADERSIZE);
		break;
	case 0x0031:	//E_CMD_DPROTO
		break;
		//case 0xAB:
		//	break;
#endif
	case 0x0A:
#ifdef USE_NetCrypt
		_crypt.Msg_InitCrypt(pMsg, len);
#endif
		//BlockChannel(0);				//只要收到加密报文
		break;

	default:
	{
		if (m_pINetEvent)
		{
			m_pINetEvent->NET_Package(pHead->cmd, pMsg, len);
		}
		//NET_Package(pHead->cmd, pMsg, len);
		break;
	}
	}
};
//void	Channel_t::ProcessOutgoing()
//{
//	int n = send(GetSocket(),obuf.GetStart(),(int)obuf.GetLength(),MSG_NOSIGNAL);
//
//}
