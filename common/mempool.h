/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _LW_MEMPOOL
#define _LW_MEMPOOL

#include <tvector.h>

class CMemChunk
{
public:
	void*							m_pMem;
	size_t							m_iSize;
	CMemChunk*						m_pNext;
};

class CMemPool
{
public:
	static void*					Alloc(size_t iSize, size_t iHandle = 0);
	static void						Free(void* pMem, size_t iHandle = 0);

	static size_t					GetMemPoolHandle();

	// A potentially dangerous function to call if the memory is still being used.
	static void						ClearPool(size_t iHandle);

private:
									CMemPool();

public:
									~CMemPool();

private:
	void*							Reserve(void* pLocation, size_t iSize, CMemChunk* pAfter = NULL);

	size_t							m_iHandle;
	void*							m_pMemPool;
	size_t							m_iMemPoolSize;
	size_t							m_iMemoryAllocated;
	CMemChunk*						m_pAllocMap;	// *Sorted* list of memory allocations.
	CMemChunk*						m_pAllocMapBack;

private:
	static CMemPool*				AddPool(size_t iSize, size_t iHandle);

	static size_t					s_iMemoryAllocated;
	static tvector<CMemPool*>		s_apMemPools;

	static size_t					s_iLastMemPoolHandle;
};

void*	mempool_alloc(size_t iSize, size_t iHandle = 0);
void	mempool_free(void* pMem, size_t iHandle = 0);
size_t	mempool_gethandle();
void	mempool_clearpool(size_t iHandle);

#endif