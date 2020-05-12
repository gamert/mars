
#include <malloc.h>

inline void* __REALLOC(void* pMem, size_t nSize)
{
#ifdef USE_MemAlloc
	return g_pMemAlloc->Realloc(pMem, nSize);
#else
	return ::realloc(pMem, nSize);
#endif
};
inline void __FREE(void* pMem)
{
#ifdef USE_MemAlloc
	g_pMemAlloc->Free(pMem);
#else
	return ::free(pMem);
#endif
};