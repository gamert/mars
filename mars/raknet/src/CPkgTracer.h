/*
	功能: 打印指定的游戏报文: 用于追查是否接收到特定的序号报文:
	使用方法:
	1. fff1: 设定追踪的协议ID... sub ,服务器说fff1 有协议没有收到，证明其使用了非可靠发送...
	2. aotu: 服务器说战报报文没有收到...


*/


#pragma once

//总开关
//#define USE_PKGTRACER
//游戏开关...
//#define FILTER_FFF1
#define FILTER_FFF2


#ifdef USE_PKGTRACER

#ifdef _WIN32
#include "zlib1_2_11.h"
#endif


#ifdef FILTER_FFF2
#include "message_generated.h"

using namespace io::flat::SpaceX;

enum ESubProtcol
{
	E_MATCH_END = 9038,	//战报...
};

using namespace std;

enum E_CONSTRUCT
{
	E_LEN_PACKET = 1048576,
	E_LEN_COMPRESS = 20480,
};

typedef unsigned char      uint8_t;

#define ESTIMATE_COMPRESS_BOUND(sourceLen) (sourceLen + (sourceLen >> 12) + (sourceLen >> 14) + 11)

//压缩
void SparseArrayCompress(const string& strSource, string& strDest)
{
	bool bNew = false;
	char* indata;
	char* outdata;
	int size = strSource.size();
	if (size > E_LEN_PACKET)
	{
		bNew = true;
		//CLOG_INFO("SparseArrayCompress too long %d", size);
		indata = new char[size];
		unsigned long out_size = ESTIMATE_COMPRESS_BOUND(size) + 32;//压缩
		outdata = new char[out_size];
		//return;
	}
	else
	{
		static char _indata[E_LEN_PACKET] = { 0 };
		static char _outdata[E_LEN_PACKET] = { 0 };
		indata = _indata;
		outdata = _outdata;
	}

	memcpy(indata, strSource.data(), size);

	int j = 1;
	uint8_t padding = (uint8_t)(size % 8);
	//outdata[j++] = padding;
	//outdata[j++] = 0; // position of last tag

	for (int ii = 0; ii < size;)
	{
		uint8_t tag = 0;
		int tagId = j++;
		int limit = min(8, size - ii);
		for (int jj = 0; jj < limit; ++jj)
		{
			if (indata[ii + jj] != 0)
			{
				tag |= (uint8_t)(1 << jj);
				outdata[j++] = indata[ii + jj];
			}
		}

		outdata[tagId] = tag;
		ii += limit;
		if (ii >= size)
			outdata[0] = (uint8_t)((padding << 4) | (uint8_t)(j - tagId));
	}

	strDest.append(outdata, j);

	if (bNew)
	{
		delete[] indata;
		delete[] outdata;
	}
}


// 解压

// 解压
void SparseArray_Decompress(const string& strSource, string& strDest)
{
	bool bNew = false;
	char* indata;
	char* outdata;
	int size = strSource.size();
	if (size > E_LEN_PACKET)
	{
		bNew = true;
		//CLOG_INFO("SparseArray_Decompress too long %d", size);
		indata = new char[size];
		unsigned long out_size = ESTIMATE_COMPRESS_BOUND(size) + 32;//压缩
		outdata = new char[out_size];
		//return;
	}
	else
	{
		static char _indata[E_LEN_PACKET] = { 0 };
		static char _outdata[E_LEN_PACKET] = { 0 };
		indata = _indata;
		outdata = _outdata;
	}

	memcpy(indata, strSource.data(), size);

	int pos = 0;
	uint8_t padding = (uint8_t)((indata[0] & 0xf0) >> 4);
	uint8_t maskpos = (uint8_t)(indata[0] & 0x0f);
	int limit = 8;
	for (int ii = 1; ii < size;)
	{
		uint8_t tag = indata[ii++];
		for (int jj = 0; jj < limit; ++jj)
		{
			uint8_t mask = (uint8_t)(((1 << jj) & tag) >> jj);
			if (mask == 1)
				outdata[pos++] = indata[ii++];
			else
				outdata[pos++] = 0;
		}
		if (ii == size - maskpos)
			limit = padding > 0 ? padding : 8;
	}

	strDest.append(outdata, pos);

	if (bNew)
	{
		delete[] indata;
		delete[] outdata;
	}
}


#endif

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#define FETCH(T,a) T a = *((T *)pData); pData += sizeof(T);
#define HEAD_LENGTH 4


#ifdef FILTER_FFF1
#include "RC4.h"
#include "BaseRC4.h"

struct PakHead_t
{
	uint16 len;
	uint8 mSubCommand;
	uint8 mCommand;
};

enum EMainProtcol
{
	PUBLIC_LOGIN_MAIN = 0,          /// publish server 相关命令
	LOGIN_MAIN_COMMAND = 1,          /// 游戏服务器主命令 EPlayerSubProtcol
	PVP_MAIN_COMMAND = 50,         /// 战斗相关命令 现在发到战斗服
};

//fff1: 

BOOL IsLittleEndian(void)
{
	WORD wValue = 0x5678;
	return (*((BYTE*)&wValue) == 0x78);
}

#endif
//fff2-







#define AutoBufferLen 512
class AutoBuffer_t
{
public:
	AutoBuffer_t(int len)
	{
		EnsureLen(len);
	}
	~AutoBuffer_t()
	{
		if(pBuf!= buf)
			delete []pBuf;
	}
	void EnsureLen(int len)
	{
		if (len > AutoBufferLen)
			pBuf = new char[len];
		else
			pBuf = buf;
		mLen = len;
	}

	int mLen;
	char *pBuf;
	char buf[AutoBufferLen];
};

class CPkgTracer
{
	//FILE *fp;
#ifdef FILTER_FFF1
	RC4 rc4;//<BYTE>
#endif
	int count;
public:
	CPkgTracer()
	{
		count = 0;
		//fp = fopen("PkgTracer.txt", "wb");

#ifdef FILTER_FFF1
		const char *password = "8jel5y";
		rc4.RC4_BeginDecode(password, strlen(password));
		BaseRC4::setPassword(password);
#endif
	}
	~CPkgTracer()
	{
		//fclose(fp);
	}

	//打印指定的游戏报文: 用于追查是否接收到特定的序号报文:
	//prefix: 发送，重发等等，用来标识io;
	void DumpLog(const char *prefix, const InternalPacket *internalPacket)
	{
		//测试用户报...
		if (!internalPacket->data)
		{
			return;
		}
#ifdef FILTER_FFF2
		DumpLog_fff2(prefix, internalPacket);

#elif defined( FILTER_FFF1)
		if(internalPacket->dataBitLength < 40
			|| internalPacket->data[0] != ID_USER_PACKET_ENUM)
		{
			return;
		}
		if (internalPacket->splitPacketCount > 0)
			return;
		//
		uint8 *pData = (uint8 *)(internalPacket->data+1);

		uLongf destLen = internalPacket->dataBitLength / 8 - 1;
		int index = 0;
		while (index < destLen)
		{
			int len = 0;
			len = handlePkg_fff1(prefix, internalPacket, pData);
			index += len;
			pData += len;
		}
#endif
	}

//
#ifdef FILTER_FFF2
	void DumpLog_fff2(const char *prefix, const InternalPacket *internalPacket)
	{
		//internalPacket->dataBitLength < 40 ||
		if ( internalPacket->data[0] != kCustomUncompressedPacketControlCode 
			&& internalPacket->data[0] != kCustomENCRYPT_MATCH_END)
		{
			return;
		}

		//只处理首分包: TODO: 是否能够根据首包解析NetMessageO？
		if (internalPacket->splitPacketCount > 0 && internalPacket->splitPacketIndex > 0)
		{
			CCLOG(1,"%s:非首包, split[%d/%d],MsgNo[%d],Index[%d],R[%d]\n", prefix, internalPacket->splitPacketIndex, internalPacket->splitPacketCount, internalPacket->reliableMessageNumber, internalPacket->orderingIndex, internalPacket->reliability);
			return;
		}
		//
		uint8 *pData = (uint8 *)(internalPacket->data + 1);
		uLongf destLen = internalPacket->dataBitLength / 8 - 1;


		string strInfo;
		string strResult;
		strInfo.append((char *)pData, destLen);
		SparseArray_Decompress(strInfo, strResult);

		const NetMessageI *msg = flatbuffers::GetRoot<NetMessageI>(strResult.c_str());
		//flatbuffers::Verifier verifier((uint8_t *)strResult.c_str(), strResult.size());
		if (msg )
		{
			//&& msg->Verify(verifier)
			CCLOG(1, "%s:channel=%d, split[%d/%d],MsgNo[%d],Index[%d],R[%d]\n", prefix, msg->channel(), internalPacket->splitPacketIndex, internalPacket->splitPacketCount, internalPacket->reliableMessageNumber, internalPacket->orderingIndex, internalPacket->reliability);
			//if (msg->channel() == E_MATCH_END)
			//{
			//}
		}
		else
		{
			CCLOG(1, "%s:Verify fail, split[%d/%d],MsgNo[%d],Index[%d],R[%d]\n", prefix, internalPacket->splitPacketIndex, internalPacket->splitPacketCount, internalPacket->reliableMessageNumber, internalPacket->orderingIndex, internalPacket->reliability);
		}
	}

	int handlePkg_fff2(const char *prefix, const InternalPacket *internalPacket, uint8 *pData)
	{
		return 0;
	}
#endif

#ifdef FILTER_FFF1
	//fff1 的单个报文...
	//fff2-spacex-aotu:
	int handlePkg_fff1(const char *prefix, const InternalPacket *internalPacket, uint8 *pData)
	{
		FETCH(PakHead_t, head);

		//我们要的报文...
		if (head.mCommand == PVP_MAIN_COMMAND && head.mSubCommand == 180)
		{
			FETCH(int, Seq);
			BaseRC4::Decrypt((char *)&Seq,4);
			int seq2 = Seq;
			rc4.RC4_decode((RC4_TYPE*)&seq2, 4, (RC4_TYPE*)&seq2, 4);
			CCLOG(1, "%s:[%d] ,[%d][%d]R[%d]\n", prefix, Seq, internalPacket->reliableMessageNumber, internalPacket->orderingIndex, internalPacket->reliability);
		}
		else if (head.mCommand == 0x39 && head.mSubCommand == 0x00)
		{
			uLongf sourceLen = head.len - 4;
			AutoBuffer_t ab1(sourceLen);
			memcpy(ab1.pBuf, pData,sourceLen);
			BaseRC4::Decrypt((char *)ab1.pBuf, sourceLen);
			//rc4.RC4_decode((RC4_TYPE*)pData, sourceLen, (RC4_TYPE*)src, sourceLen);

			//解压一下...
			uLongf destLen = 32 * head.len + 1024;
			AutoBuffer_t ab2(destLen);
			int res = uncompress((Bytef *)ab2.pBuf, &destLen, (Bytef *)ab1.pBuf, sourceLen);
			if (res == 0)
			{
				int index = 0;
				uint8 *pData = (uint8 *)ab2.pBuf;
				while (index < destLen)
				{
					int len = handlePkg_fff1(prefix, internalPacket, pData);
					index += len;
					pData += len;
				}
			}
			else
			{
				//assert//
				int i = 0;
				i++;
			}
		}
		return head.len;
	}
#endif

	//
	//void Log(const char *_Format, ...)
	//{
	//	if (fp)
	//	{
	//		const int bufsize = 1024 * 2;
	//		char buffer[bufsize];

	//		va_list argptr;
	//		int cnt;
	//		va_start(argptr, _Format);
	//		cnt = vsnprintf(buffer, bufsize, _Format, argptr);
	//		va_end(argptr);

	//		//vstart()

	//		fwrite(buffer,1, cnt,fp);
	//		count++;
	//		if ((count % 9) == 8)
	//		{
	//			fflush(fp);
	//		}
	//	}
	//}
};

CPkgTracer g_CPkgTracer;

#endif