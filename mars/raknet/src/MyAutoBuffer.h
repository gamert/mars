#ifndef MyAutoBuffer_H
#define MyAutoBuffer_H

#include "plat/basetypes.h"
#include "plat/memalloc.h"


template <typename T = char ,int E_MAX_AB = 3800>
struct TTypedAutoBuffer_t
{
	enum 
	{
		E_MAX_AB_SPACE	= E_MAX_AB - 8,
	};
	T	fixbuf[E_MAX_AB];		//该固定缓存总是满足大部分stack存储需求
	T	*pNewBuf;

	T	*pBuf;
	int  nLen;					//缓冲长度(T)
	TTypedAutoBuffer_t():pNewBuf(NULL),pBuf(NULL),nLen(0)
	{
	}
	~TTypedAutoBuffer_t()
	{
		FreeBuffer();
	}
	inline int	 GetStackBufSize(){return E_MAX_AB_SPACE;}
	inline T	*EnsureBuffer(int nSize)
	{
		nLen = nSize;
		if(nLen > E_MAX_AB_SPACE)
		{
			pNewBuf = (T *)__REALLOC(pNewBuf,(nLen+4)*sizeof(T));
			pBuf	= pNewBuf;
		}
		else
		{
			pBuf	= fixbuf;
			if(pNewBuf)
			{
				__FREE(pNewBuf);
				pNewBuf = NULL;
			}
		}
		memset(&pBuf[nLen],0,sizeof(T));
		//pBuf[nLen]	= 0;	//假定其为字符串～
		return data();
	};
	inline void FreeBuffer()
	{
		if(pNewBuf)
		{
			__FREE(pNewBuf);
			pNewBuf = NULL;
		}
		pBuf = NULL;
		nLen = 0;
	}
	inline T *data(){return pBuf;}
	inline T *c_str(){return pBuf;}
	inline int size(){return nLen;}

	inline void HookSet(BYTE **ppbyGameProtoInitData, uint32& dwGameProtoInitSize)
	{
		EnsureBuffer(dwGameProtoInitSize);
		*ppbyGameProtoInitData = (BYTE *)data();
	};

};

template <int E_MAX_AB = 3800>
struct TAutoBuffer_t : public TTypedAutoBuffer_t<char ,E_MAX_AB>
{
};

typedef TAutoBuffer_t<3800> AutoBuffer_t;	//4096 will cause _chkstk()


#endif