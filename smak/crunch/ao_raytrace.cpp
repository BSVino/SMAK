/*
SMAK - The Super Model Army Knife
Copyright (C) 2012, Lunar Workshop, Inc

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "ao.h"

#include <raytracer/raytracer.h>
#include <tinker_platform.h>

typedef struct
{
	CAOGenerator*				pGenerator;
	raytrace::CRaytracer*		pTracer;
	Vector						vecUVPosition;
	Vector						vecNormal;
	CConversionMeshInstance*	pMeshInstance;
	CConversionFace*			pFace;
	size_t						iTexel;
} thread_job_t;

void RaytraceSceneFromPosition(void* pVoidData)
{
	thread_job_t* pJobData = (thread_job_t*)pVoidData;
	pJobData->pGenerator->RaytraceSceneFromPosition(pJobData->pTracer, pJobData->vecUVPosition, pJobData->vecNormal, pJobData->pMeshInstance, pJobData->pFace, pJobData->iTexel);
}

void CAOGenerator::RaytraceSceneFromPosition(raytrace::CRaytracer* pTracer, Vector vecUVPosition, Vector vecNormal, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t iTexel)
{
	// Build rotation matrix
	Matrix4x4 m;
	m.SetOrientation(vecNormal);

	// Turn it sideways so that pitch 90 is up
	Matrix4x4 m2;
	m2.SetAngles(EAngle(-90, 0, 0));

	m *= m2;

	//SMAKWindow()->AddDebugLine(vecUVPosition + pFace->GetNormal()*0.01f, vecUVPosition + vecNormal*0.5f, Color(0, 0, 255));

	float flHits = 0;
	float flTotalHits = 0;

	for (size_t x = 0; x < m_iSamples/2; x++)
	{
		float flRandom = 0;
		if (m_bRandomize)
			flRandom = RemapVal((float)(rand()%10000), 0, 10000.0f, -0.5f, 0.5f);

		float flPitch = RemapVal(cos(RemapVal((float)x+flRandom, 0, (float)m_iSamples/2, 0, (float)M_PI/2)), 0, 1, 90, 0);

		float flWeight = sin(flPitch * M_PI/180);

		for (size_t y = 0; y <= m_iSamples; y++)
		{
			flRandom = 0;
			if (m_bRandomize)
				flRandom = RemapVal((float)(rand()%10000), 0, 10000.0f, -0.5f, 0.5f);

			float flYaw = RemapVal((float)y+flRandom, 0, (float)m_iSamples, -180, 180);

			Vector vecDir = AngleVector(EAngle(flPitch, flYaw, 0));

			// Transform relative to the triangle's normal
			Vector vecRay = m * vecDir;

			//RenderSceneFromPosition(vecUVPosition, vecRay, pFace);

			flTotalHits += flWeight;

			//SMAKWindow()->AddDebugLine(vecUVPosition + pFace->GetNormal()*0.01f, vecUVPosition + vecRay.Normalized()*0.1f, vecDir);

			raytrace::CTraceResult tr;
			if (pTracer->Raytrace(Ray(vecUVPosition + pFace->GetNormal()*0.01f, vecRay), &tr))
			{
				float flDistance = (tr.m_vecHit - vecUVPosition).Length();
				if (m_flRayFalloff < 0)
					flHits += flWeight;
				else
					flHits += flWeight * (1/pow(2, flDistance/m_flRayFalloff));
			}
			else if (m_bGroundOcclusion && vecRay.y < 0)
			{
				// The following math is basically a plane-ray intersection algorithm,
				// with shortcuts made for the assumption of an infinite plane facing straight up.

				Vector n = Vector(0,1,0);

				float a = -(vecUVPosition.y - pMeshInstance->m_pParent->m_oExtends.m_vecMins.y);
				float b = vecRay.y;

				float flDistance = a/b;

				if (flDistance < 1e-4f || m_flRayFalloff < 0)
					flHits += flWeight;
				else
					flHits += flWeight * (1/pow(2, flDistance/m_flRayFalloff));
			}
		}
	}

	// One last ray directly up, it is skipped in the above loop so it's not done 10 times.
	Vector vecDir = AngleVector(EAngle(90, 0, 0));

	// Transform relative to the triangle's normal
	Vector vecRay = m * vecDir;

	//RenderSceneFromPosition(vecUVPosition, vecRay, pFace);

	flTotalHits++;

	//SMAKWindow()->AddDebugLine(vecUVPosition + pFace->GetNormal()*0.01f, vecUVPosition + vecRay.Normalized()*0.2f, vecDir);

	raytrace::CTraceResult tr;
	if (pTracer->Raytrace(Ray(vecUVPosition + pFace->GetNormal()*0.01f, vecRay), &tr))
	{
		float flDistance = (tr.m_vecHit - vecUVPosition).Length();
		if (m_flRayFalloff < 0)
			flHits += 1;
		else
			flHits += (1/pow(2, flDistance/m_flRayFalloff));
	}
	else if (m_bGroundOcclusion && vecRay.y < 0)
	{
		// The following math is basically a plane-ray intersection algorithm,
		// with shortcuts made for the assumption of an infinite plane facing straight up.

		Vector n = Vector(0,1,0);

		float a = -(vecUVPosition.y - pMeshInstance->m_pParent->m_oExtends.m_vecMins.y);
		float b = vecRay.y;

		float flDistance = a/b;

		if (flDistance < 1e-4f || m_flRayFalloff < 0)
			flHits += 1;
		else
			flHits += (1/pow(2, flDistance/m_flRayFalloff));
	}

	float flShadowValue = 1 - ((float)flHits / (float)flTotalHits);

	// Mutex may be dead, try to bail before.
	if (m_bStopGenerating)
		return;

	if (GetNumberOfProcessors() > 1)
	{
		// Keep all locking and unlocking in one branch to prevent processor prediction miss problems.
		// I saw it in some presentation somewhere.
		m_pRaytraceParallelizer->LockData();
		m_avecShadowValues[iTexel] += Vector(flShadowValue, flShadowValue, flShadowValue);
		m_pRaytraceParallelizer->UnlockData();
	}
	else
		m_avecShadowValues[iTexel] += Vector(flShadowValue, flShadowValue, flShadowValue);
}

void CAOGenerator::RaytraceSetupThreads()
{
	if (GetNumberOfProcessors() == 1)
		return;

	RaytraceCleanupThreads();

	m_pRaytraceParallelizer = new CParallelizer((JobCallback)::RaytraceSceneFromPosition);
	m_pRaytraceParallelizer->Start();
}

void CAOGenerator::RaytraceCleanupThreads()
{
	if (m_pRaytraceParallelizer)
		delete m_pRaytraceParallelizer;

	m_pRaytraceParallelizer = NULL;
}

void CAOGenerator::RaytraceSceneMultithreaded(raytrace::CRaytracer* pTracer, Vector vecUVPosition, Vector vecNormal, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t iTexel)
{
	if (!m_pRaytraceParallelizer)
	{
		RaytraceSceneFromPosition(pTracer, vecUVPosition, vecNormal, pMeshInstance, pFace, iTexel);
		return;
	}

	thread_job_t oJob;
	oJob.pGenerator = this;
	oJob.pTracer = pTracer;
	oJob.vecUVPosition = vecUVPosition;
	oJob.vecNormal = vecNormal;
	oJob.pMeshInstance = pMeshInstance;
	oJob.pFace = pFace;
	oJob.iTexel = iTexel;

	m_pRaytraceParallelizer->AddJob(&oJob, sizeof(oJob));
}

void CAOGenerator::RaytraceJoinThreads()
{
	if (!m_pRaytraceParallelizer)
		return;

	// Doesn't really join the threads per se, just signals for them to quit and then waits for them to be done
	// while calling work progress updates so the user can see what's happening.

	m_pRaytraceParallelizer->FinishJobs();

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Rendering", m_pRaytraceParallelizer->GetJobsTotal());

	while (true)
	{
		if (m_pRaytraceParallelizer->AreAllJobsDone())
			return;

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(m_pRaytraceParallelizer->GetJobsDone());

		if (m_bStopGenerating)
		{
			RaytraceCleanupThreads();
			return;
		}
	}
}
