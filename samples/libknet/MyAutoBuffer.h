/*
	for fast mem use by sized stack/heap 
	txz2007 
*/

#ifndef MyAutoBuffer_H
#define MyAutoBuffer_H

#ifndef FORCEINLINE
	#if (_MSC_VER >= 1200)
		#define FORCEINLINE __forceinline
	#else
		#define FORCEINLINE __inline
	#endif
#endif

#ifdef CL_MEMALLOC
	#include "plat/memalloc.h"
	FORCEINLINE void *Mem_Realloc(void *pMem, size_t nSize)
	{
		return g_pMemAlloc->Realloc(pMem, nSize);
	}
	FORCEINLINE void Mem_Free(void *pMem)
	{
		g_pMemAlloc->Free(pMem);
	}

#else
	#include <stdio.h>
	#include <malloc.h>
	#include <assert.h>

	FORCEINLINE void *Mem_Realloc(void *pMem, size_t nSize)
	{
		return ::realloc(pMem, nSize);
	}

	FORCEINLINE void Mem_Free(void *pMem)
	{
		::free(pMem);
	}

#endif

typedef unsigned long       DWORD;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;


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
	int  nPos;
	TTypedAutoBuffer_t():pNewBuf(NULL), nPos(0)
	{
		pBuf = fixbuf;
		nLen = E_MAX_AB;
	}

	~TTypedAutoBuffer_t()
	{
		FreeBuffer();
	}
	FORCEINLINE int	 GetStackBufSize(){return E_MAX_AB_SPACE;}
	FORCEINLINE T	*EnsureBuffer(int nSize)
	{
		nLen = nSize;
		if(nLen > E_MAX_AB_SPACE)
		{
			pNewBuf = (T *)Mem_Realloc(pNewBuf,(nLen+4)*sizeof(T));
			pBuf	= pNewBuf;
		}
		else
		{
			pBuf	= fixbuf;
			if(pNewBuf)
			{
				Mem_Free(pNewBuf);
				pNewBuf = NULL;
			}
		}
		memset(&pBuf[nLen],0,sizeof(T));
		//pBuf[nLen]	= 0;	//假定其为字符串～
		return data();
	};
	FORCEINLINE void FreeBuffer()
	{
		if(pNewBuf)
		{
			Mem_Free(pNewBuf);
			pNewBuf = NULL;
		}
		pBuf = NULL;
		nLen = 0;
	}
	FORCEINLINE T *data(){return pBuf;}
	FORCEINLINE T *c_str(){return pBuf;}
	FORCEINLINE int size(){return nPos;}


	FORCEINLINE void HookSet(BYTE **ppbyGameProtoInitData, DWORD& dwGameProtoInitSize)
	{
		EnsureBuffer(dwGameProtoInitSize);
		*ppbyGameProtoInitData = (BYTE *)data();
	};


	//copy data into buf..
	FORCEINLINE void Append(const char *p, int len)
	{
		assert(nPos + len <= nLen);
		memcpy(pBuf + nPos, p, len); nPos += len;
	}
	//simple type 
	template<typename TypeEle>
	FORCEINLINE void Append(TypeEle p)
	{
		assert(nPos + sizeof(TypeEle) <= nLen);
		*((TypeEle*)(pBuf + nPos)) = p; nPos += sizeof(TypeEle);
	}

};

template <int E_MAX_AB = 3800>
struct TAutoBuffer_t : public TTypedAutoBuffer_t<char ,E_MAX_AB>
{
};

typedef TAutoBuffer_t<3800> AutoBuffer_t;	//4096 will cause win32 _chkstk() 


#endif