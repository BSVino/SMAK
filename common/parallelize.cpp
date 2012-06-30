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

#include "parallelize.h"

#include <time.h>
#include <tinker_platform.h>
#include <mempool.h>

void ThreadMain(void* pData)
{
	CParallelizeThread* pThread = (CParallelizeThread*)pData;
	pThread->Process();
}

void CParallelizeThread::Process()
{
	while (true)
	{
		if (m_pParallelizer->m_bShuttingDown)
		{
			m_bQuit = true;
			pthread_exit(NULL);
			return;
		}

		if (m_pParallelizer->m_bStopped)
		{
			SleepMS(100);
			continue;
		}

		pthread_mutex_lock(&m_pParallelizer->m_iJobsMutex);
		if (m_pParallelizer->m_iJobsDone == m_pParallelizer->m_iJobsGiven)
		{
			pthread_mutex_unlock(&m_pParallelizer->m_iJobsMutex);

			m_bDone = true;

			if (m_bQuitWhenDone)
			{
				m_bQuit = true;
				pthread_exit(NULL);
				return;
			}
			else
			{
				// Give the main thread a chance to render and whatnot.
				SleepMS(1);
				continue;
			}
		}
		else
			m_bDone = false;

		for (size_t i = m_pParallelizer->m_iLastExecuted; i < m_pParallelizer->m_aJobs.size(); i++)
		{
			if (!m_pParallelizer->m_aJobs[i].m_pJobData)
				continue;

			if (m_pParallelizer->m_aJobs[i].m_iExecuted >= m_pParallelizer->m_iExecutions)
				continue;

			if (i > m_pParallelizer->m_iLastAssigned)
				break;

			m_pParallelizer->m_iLastExecuted = i;
			break;
		}

		if (m_pParallelizer->m_iLastExecuted > m_pParallelizer->m_iLastAssigned)
			continue;

		CParallelizeJob* pJob = &m_pParallelizer->m_aJobs[m_pParallelizer->m_iLastExecuted];
		pJob->m_iExecuted = m_pParallelizer->m_iExecutions;

		m_pParallelizer->m_iJobsDone++;

		pthread_mutex_unlock(&m_pParallelizer->m_iJobsMutex);

		m_pParallelizer->DispatchJob(pJob->m_pJobData);
	}
}

CParallelizer::CParallelizer(JobCallback pfnCallback)
{
	pthread_mutex_init(&m_iDataMutex, NULL);
	pthread_mutex_init(&m_iJobsMutex, NULL);

	m_iJobsGiven = 0;
	m_iJobsDone = 0;

	// Insert all first so that reallocations are all done before we pass the pointers to the threads.
	m_aThreads.insert(m_aThreads.begin(), GetNumberOfProcessors(), CParallelizeThread());

	for (size_t i = 0; i < GetNumberOfProcessors(); i++)
	{
		CParallelizeThread* pThread = &m_aThreads[i];

		pThread->m_pParallelizer = this;
		pThread->m_bQuitWhenDone = false;
		pThread->m_bDone = false;
		pThread->m_bQuit = false;

		pthread_create(&pThread->m_iThread, NULL, (void *(*) (void *))&ThreadMain, (void*)pThread);
	}

	m_bStopped = true;
	m_bShuttingDown = false;

	m_pfnCallback = pfnCallback;

	m_iLastAssigned = -1;
	m_iLastExecuted = 0;
	m_aJobs.resize(100);

	m_iExecutions = 1;

	m_iMemPool = mempool_gethandle();
}

CParallelizer::~CParallelizer()
{
	m_bShuttingDown = true;

	while (!AreAllJobsQuit());

	// Clears all mempool_alloc'd job data
	mempool_clearpool(m_iMemPool);

	pthread_mutex_destroy(&m_iDataMutex);
	pthread_mutex_destroy(&m_iJobsMutex);

	m_aThreads.clear();
	m_aJobs.clear();
}

void CParallelizer::AddJob(void* pJobData, size_t iSize)
{
	pthread_mutex_lock(&m_iJobsMutex);

	size_t i = m_iLastAssigned+1;

	// Avoid memory allocations for every added job
	if (i >= m_aJobs.size())
		m_aJobs.resize(m_aJobs.size()*2);

	m_aJobs[i].m_pJobData = mempool_alloc(iSize, m_iMemPool);
	m_aJobs[i].m_iExecuted = 0;
	m_iLastAssigned = i;

	memcpy(m_aJobs[i].m_pJobData, pJobData, iSize);

	pthread_mutex_unlock(&m_iJobsMutex);

	m_iJobsGiven++;
}

void CParallelizer::FinishJobs()
{
	for (size_t i = 0; i < m_aThreads.size(); i++)
		m_aThreads[i].m_bQuitWhenDone = true;
}

bool CParallelizer::AreAllJobsDone()
{
	if (m_iJobsDone == m_iJobsGiven)
		return true;

	for (size_t t = 0; t < m_aThreads.size(); t++)
	{
		if (!m_aThreads[t].m_bDone)
		{
			// Give the job a chance to finish.
			SleepMS(1);
			return false;
		}
	}
	return true;
}

bool CParallelizer::AreAllJobsQuit()
{
	for (size_t t = 0; t < m_aThreads.size(); t++)
	{
		if (!m_aThreads[t].m_bQuit)
			return false;
	}
	return true;
}

void CParallelizer::RestartJobs()
{
	pthread_mutex_lock(&m_iJobsMutex);

	m_iLastExecuted = 0;
	m_iExecutions++;
	m_iJobsDone = 0;

	pthread_mutex_unlock(&m_iJobsMutex);

	Start();
}

void CParallelizer::LockData()
{
	pthread_mutex_lock(&m_iDataMutex);
}

void CParallelizer::UnlockData()
{
	pthread_mutex_unlock(&m_iDataMutex);
}

void CParallelizer::DispatchJob(void* pJobData)
{
	m_pfnCallback(pJobData);
}
