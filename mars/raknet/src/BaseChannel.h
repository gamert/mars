
#pragma once

#include <list>
#include "CircularBuffer.h"


#define DATA_BUFSIZE				1024*128	
#define SOCK_BUFFER_SIZE  			4000        
#define TCP_BUFSIZE_READ			16400*4		

//±£¥ÊTCP data...
class CBaseChannel
{
public:
	/** Dynamic output buffer storage struct. 
		\ingroup internal */
	struct MES {
		MES( const char *buf_in,size_t len_in)
		:buf(new  char[len_in])
		,len(len_in)
		,ptr(0)
		{
			memcpy(buf,buf_in,len);
		}
		~MES() { delete[] buf; }
		size_t left() { return len - ptr; }
		 char *curbuf() { return buf + ptr; }
		 char *buf;
		size_t len;
		size_t ptr;
	};
	/** Dynamic output buffer list. */
	typedef std::list<MES *> ucharp_v;
	static inline void ClearMES(ucharp_v &arg)
	{
		while (!arg.empty())
		{
			MES *p = arg.front();
			arg.pop_front();
			delete p;
		}
	}

public:
	CBaseChannel():m_b_input_buffer_disabled(false)
		, ibuf(TCP_BUFSIZE_READ * 2)
//		, obuf(32768)
		, m_bytes_sent(0)
		, m_bytes_received(0)
	{

	}
	size_t GetInputLength()
	{
		return ibuf.GetLength();
	}

	//size_t GetOutputLength()
	//{
	//	return obuf.GetLength();
	//}

	void ClearIObuf()
	{
		//obuf.Reset();
		ibuf.Reset();
		ClearMES(m_mes_in);
		ClearMES(m_mes);
	};

	bool			m_b_input_buffer_disabled;
	CircularBuffer  ibuf; ///< Circular input buffer
	//CircularBuffer  obuf; ///< Circular output buffer
	uint64_t		m_bytes_sent;
	uint64_t		m_bytes_received;
	ucharp_v		m_mes;		///< overflow protection, dynamic output buffer
	ucharp_v		m_mes_in;	///< overflow protection, dynamic input buffer
};