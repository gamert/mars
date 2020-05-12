/** \file CircularBuffer.cpp
 **	\date  
 **	\author 
**/

#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include "CircularBuffer.h"
#include <string.h>
#include <assert.h>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


CircularBuffer::CircularBuffer(size_t size):buf(new char[2 * size])
,m_max(size)
,m_q(0)
,m_b(0)
,m_t(0)
,m_count(0)
{
}


CircularBuffer::~CircularBuffer()
{
	delete[] buf;
}

void CircularBuffer::Reset()
{
	m_q = (0);		//current used
	m_b = (0);		//bottom
	m_t = (0);		//top
	m_count = (0);	//all handled bytes
};

bool CircularBuffer::Write(const char *s,size_t l)
{
	if (m_q + l > m_max)
	{
		assert(false && "m_q + l > m_max");
		//OutputDebugString();
//		m_owner.Handler().LogError(&m_owner, "CircularBuffer::Write", -1, "write buffer overflow");
		return false; // overflow
	}
	m_count += (unsigned long)l;
	if (m_t + l > m_max) // block crosses circular border
	{
		size_t l1 = m_max - m_t; // size left until circular border crossing
		// always copy full block to buffer(buf) + top pointer(m_t)
		// because we have doubled the buffer size for performance reasons
		memcpy(buf + m_t, s, l);
		memcpy(buf, s + l1, l - l1);
		m_t = l - l1;
		m_q += l;
	}
	else
	{
		memcpy(buf + m_t, s, l);
		memcpy(buf + m_max + m_t, s, l);
		m_t += l;
		if (m_t >= m_max)
			m_t -= m_max;
		m_q += l;
	}
	return true;
}


bool CircularBuffer::Read(char *s,size_t l)
{
	if (l > m_q)
	{
		assert(false && "l > m_q");
//		m_owner.Handler().LogError(&m_owner, s ? "CircularBuffer::Read" : "CircularBuffer::Write", -1, "attempt to read beyond buffer");
		return false; // not enough chars
	}
	if (m_b + l > m_max) // block crosses circular border
	{
		size_t l1 = m_max - m_b;
		if (s)
		{
			memcpy(s, buf + m_b, l1);
			memcpy(s + l1, buf, l - l1);
		}
		m_b = l - l1;
		m_q -= l;
	}
	else
	{
		if (s)
		{
			memcpy(s, buf + m_b, l);
		}
		m_b += l;
		if (m_b >= m_max)
			m_b -= m_max;
		m_q -= l;
	}
	if (!m_q)
	{
		m_b = m_t = 0;
	}
	return true;
}

bool CircularBuffer::SoftRead(char *s, size_t l)
{
    if (l > m_q)
    {
        return false;
    }
    if (m_b + l > m_max)                          // block crosses circular border
    {
        size_t l1 = m_max - m_b;
        if (s)
        {
            memcpy(s, buf + m_b, l1);
            memcpy(s + l1, buf, l - l1);
        }
    }
    else
    {
        if (s)
        {
            memcpy(s, buf + m_b, l);
        }
    }
    return true;
}

bool CircularBuffer::Remove(size_t l)
{
	return Read(NULL, l);
}


size_t CircularBuffer::GetLength()
{
	return m_q;
}


const char *CircularBuffer::GetStart()
{
	return buf + m_b;
}


size_t CircularBuffer::GetL()
{
	return (m_b + m_q > m_max) ? m_max - m_b : m_q;
}


size_t CircularBuffer::Space()
{
	return m_max - m_q;
}


unsigned long CircularBuffer::ByteCounter(bool clear)
{
	if (clear)
	{
		unsigned long x = m_count;
		m_count = 0;
		return x;
	}
	return m_count;
}



std::string CircularBuffer::ReadString(size_t l)
{
	char *sz = new char[l + 1];
	if (!Read(sz, l)) // failed, debug printout in Read() method
	{
		delete[] sz;
		return "";
	}
	sz[l] = 0;
	std::string tmp = sz;
	delete[] sz;
	return tmp;
}


#ifdef SOCKETS_NAMESPACE
}
#endif

