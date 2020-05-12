/** \file CircularBuffer.h
 **	\date 
 **	\author 
**/
#ifndef _SOCKETS_CircularBuffer_H
#define _SOCKETS_CircularBuffer_H

/* Define NO_GETADDRINFO if your operating system does not support
   the "getaddrinfo" and "getnameinfo" function calls. */
#ifdef _WIN32
#define NO_GETADDRINFO
#endif

//#define SOCKETS_NAMESPACE

#include <string>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


/** \defgroup internal Internal utility */

/** Buffer class containing one read/write circular buffer. 
	\ingroup internal */
class CircularBuffer
{
public:
	CircularBuffer(size_t size);
	~CircularBuffer();
	void Reset();

	/** append l bytes from p to buffer */
	bool Write(const char *p,size_t l);
	/** copy l bytes from buffer to dest */
	bool Read(char *dest,size_t l);
	/** copy l bytes from buffer to dest, dont touch buffer pointers */
    bool SoftRead(char *dest, size_t l);
	/** skip l bytes from buffer */
	bool Remove(size_t l);
	/** read l bytes from buffer, returns as string. */
	std::string ReadString(size_t l);

	size_t GetMax(){return m_max;};
	/** total buffer length */
	size_t GetLength();
	/** pointer to circular buffer beginning */
	const char *GetStart();
	/** return number of bytes from circular buffer beginning to buffer physical end */
	size_t GetL();
	/** return free space in buffer, number of bytes until buffer overrun */
	size_t Space();

	/** return total number of bytes written to this buffer, ever */
	unsigned long ByteCounter(bool clear = false);

private:

//	CircularBuffer(const CircularBuffer& s)  {s;}
//	CircularBuffer& operator=(const CircularBuffer& ) { return *this; }

	char *buf;
	size_t m_max;
	size_t m_q;
	size_t m_b;
	size_t m_t;
	unsigned long m_count;
};


#ifdef SOCKETS_NAMESPACE
}
#endif



#endif // _SOCKETS_CircularBuffer_H
