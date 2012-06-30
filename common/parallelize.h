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

#ifndef _LW_PARALLELIZE_H
#define _LW_PARALLELIZE_H

#include <tvector.h>
#include <pthread.h>

class CParallelizeThread
{
public:
	void							Process();

public:
	pthread_t						m_iThread;
	class CParallelizer*			m_pParallelizer;
	bool							m_bQuitWhenDone;
	bool							m_bDone;
	bool							m_bQuit;
};

class CParallelizeJob
{
public:
									CParallelizeJob() : m_pJobData(NULL), m_iExecuted(0) {};

public:
	void*							m_pJobData;
	size_t							m_iExecuted;
};

typedef void (*JobCallback)(void*);

class CParallelizer
{
public:
	friend class CParallelizeThread;

public:
									CParallelizer(JobCallback pfnCallback);
									~CParallelizer();

public:
	void							AddJob(void* pJobData, size_t iSize);
	void							FinishJobs();
	bool							AreAllJobsDone();
	bool							AreAllJobsQuit();

	void							Start() { m_bStopped = false; };
	void							Stop() { m_bStopped = true; };
	void							RestartJobs();

	void							LockData();
	void							UnlockData();

	size_t							GetJobsTotal() { return m_iJobsGiven; };
	size_t							GetJobsDone() { return m_iJobsDone; };	// GetJobsDone: What I like to say

private:
	void							DispatchJob(void* pJobData);

	tvector<CParallelizeThread>		m_aThreads;
	tvector<CParallelizeJob>		m_aJobs;
	size_t							m_iLastExecuted;
	size_t							m_iLastAssigned;
	pthread_mutex_t					m_iJobsMutex;
	size_t							m_iJobsGiven;
	size_t							m_iJobsDone;	// JobsDone: What I like to hear
	pthread_mutex_t					m_iDataMutex;
	size_t							m_iExecutions;

	size_t							m_iMemPool;

	bool							m_bStopped;
	bool							m_bShuttingDown;

	JobCallback						m_pfnCallback;
};

#endif
