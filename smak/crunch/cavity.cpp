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

#include "cavity.h"

#include <maths.h>

#include <raytracer/raytracer.h>
#include <textures/materiallibrary.h>
#include <renderer/shaders.h>

CCavityGenerator::CCavityGenerator(CConversionScene* pScene)
{
	m_pScene = pScene;

	m_bPixelMask = NULL;

	m_bIsGenerating = false;
	m_bDoneGenerating = false;
	m_bStopGenerating = false;

	m_pWorkListener = nullptr;
	m_pCavityParallelizer = nullptr;

	m_iNormalWidth = 0;
}

CCavityGenerator::~CCavityGenerator()
{
	free(m_bPixelMask);
}

void CCavityGenerator::Think()
{
}

void CCavityGenerator::SaveToFile(const tchar *pszFilename)
{
	if (!pszFilename)
		return;

	tvector<Color> aclrTexels;
	aclrTexels.resize(m_avecCavityTexels.size());

	for (size_t i = 0; i < aclrTexels.size(); i++)
		aclrTexels[i] = Color(m_avecCavityTexels[i]);

	CRenderer::WriteTextureToFile(aclrTexels.data(), m_iNormalWidth, m_iNormalHeight, pszFilename);
}

bool CCavityGenerator::Texel(size_t w, size_t h, size_t& iTexel, size_t tw, size_t th, bool* abMask)
{
	if (w < 0 || h < 0 || w >= tw || h >= th)
		return false;

	TAssert(tw == th); // This formula is actually wrong unless the thing is a perfect square
	iTexel = th*(th-h-1) + w;

	TAssert(iTexel >= 0 && iTexel < tw * th);

	if (abMask && !abMask[iTexel])
		return false;

	return true;
}

typedef struct
{
	CCavityGenerator*	pGenerator;
	size_t				x;
	size_t				y;
} cavity_data_t;

void CavityValue(void* pVoidData)
{
	cavity_data_t* pJobData = (cavity_data_t*)pVoidData;

	pJobData->pGenerator->FindCavityValue(pJobData->x, pJobData->y);
}

void CCavityGenerator::FindCavityValue(size_t x, size_t y)
{
	if (!m_iNormalWidth)
		return;

	if (!m_avecNormalTexels.size())
		return;

	size_t iCenterTexel;
	Texel(x, y, iCenterTexel, m_iNormalWidth, m_iNormalHeight, false);

	float flConcavity = 0;
	float flSamples = 0;

	size_t iTexel;
	if (Texel(x+1, y, iTexel, m_iNormalWidth, m_iNormalHeight, false))
	{
		Vector vecPoint = m_avecNormalTexels[iTexel];
		flConcavity += Vector(1, 0, 0).Dot(vecPoint)/2+0.5f;
		flSamples += 1;
	}

	if (Texel(x-1, y, iTexel, m_iNormalWidth, m_iNormalHeight, false))
	{
		Vector vecPoint = m_avecNormalTexels[iTexel];
		flConcavity += Vector(-1, 0, 0).Dot(vecPoint)/2+0.5f;
		flSamples += 1;
	}

	if (Texel(x, y+1, iTexel, m_iNormalWidth, m_iNormalHeight, false))
	{
		Vector vecPoint = m_avecNormalTexels[iTexel];
		flConcavity += Vector(0, 1, 0).Dot(vecPoint)/2+0.5f;
		flSamples += 1;
	}

	if (Texel(x, y-1, iTexel, m_iNormalWidth, m_iNormalHeight, false))
	{
		Vector vecPoint = m_avecNormalTexels[iTexel];
		flConcavity += Vector(0, -1, 0).Dot(vecPoint)/2+0.5f;
		flSamples += 1;
	}

	if (Texel(x+1, y+1, iTexel, m_iNormalWidth, m_iNormalHeight, false))
	{
		Vector vecPoint = m_avecNormalTexels[iTexel];
		flConcavity += (Vector(0.707f, 0.707f, 0).Dot(vecPoint)/2+0.5f)*0.707f;
		flSamples += 0.707f;	// sqrt(2)/2, or the x distance to a point one unit away at 45 degrees
	}

	if (Texel(x-1, y+1, iTexel, m_iNormalWidth, m_iNormalHeight, false))
	{
		Vector vecPoint = m_avecNormalTexels[iTexel];
		flConcavity += (Vector(-0.707f, 0.707f, 0).Dot(vecPoint)/2+0.5f)*0.707f;
		flSamples += 0.707f;	// sqrt(2)/2, or the x distance to a point one unit away at 45 degrees
	}

	if (Texel(x-1, y-1, iTexel, m_iNormalWidth, m_iNormalHeight, false))
	{
		Vector vecPoint = m_avecNormalTexels[iTexel];
		flConcavity += (Vector(-0.707f, -0.707f, 0).Dot(vecPoint)/2+0.5f)*0.707f;
		flSamples += 0.707f;	// sqrt(2)/2, or the x distance to a point one unit away at 45 degrees
	}

	if (Texel(x+1, y-1, iTexel, m_iNormalWidth, m_iNormalHeight, false))
	{
		Vector vecPoint = m_avecNormalTexels[iTexel];
		flConcavity += (Vector(0.707f, -0.707f, 0).Dot(vecPoint)/2+0.5f)*0.707f;
		flSamples += 0.707f;	// sqrt(2)/2, or the x distance to a point one unit away at 45 degrees
	}

	if (flSamples < 0.1f)
		return;

	flConcavity /= flSamples;

	// Don't need to lock the data because we're guaranteed never to access the same texel twice because of the generation method.
	m_avecCavityTexels[iCenterTexel] = Vector(flConcavity, flConcavity, flConcavity);
}

void CCavityGenerator::Generate()
{
	if (m_pWorkListener)
	{
		m_pWorkListener->BeginProgress();
		m_pWorkListener->SetAction("Dispatching jobs", m_iNormalWidth);
	}

	if (m_pCavityParallelizer)
		delete m_pCavityParallelizer;
	m_pCavityParallelizer = new CParallelizer((JobCallback)::CavityValue);

	cavity_data_t oJob;
	oJob.pGenerator = this;

	for (size_t x = 0; x < m_iNormalWidth; x++)
	{
		for (size_t y = 0; y < m_iNormalHeight; y++)
		{
			oJob.x = x;
			oJob.y = y;
			m_pCavityParallelizer->AddJob(&oJob, sizeof(oJob));
		}

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(x);
	}

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Calculating cavities", m_pCavityParallelizer->GetJobsTotal());

	m_pCavityParallelizer->Start();

	m_pCavityParallelizer->FinishJobs();

	while (!m_pCavityParallelizer->AreAllJobsDone())
	{
		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(m_pCavityParallelizer->GetJobsDone());
	}

	m_bDoneGenerating = true;

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}

CTextureHandle CCavityGenerator::GenerateTexture(bool bInMedias)
{
	return CTextureLibrary::AddTexture(m_avecCavityTexels.data(), m_iNormalWidth, m_iNormalHeight);
}

void CCavityGenerator::SetNormalTexture(CMaterialHandle hMaterial)
{
	if (!hMaterial.IsValid())
		return;

	tvector<Color> aclrNormal;
	size_t iWidth, iHeight;
	if (!SMAKWindow()->GetCombinedNormal(hMaterial, aclrNormal, iWidth, iHeight))
		return;

	m_avecNormalTexels.resize(aclrNormal.size());
	m_avecCavityTexels.resize(aclrNormal.size());

	for (size_t i = 0; i < aclrNormal.size(); i++)
		m_avecNormalTexels[i] = Vector(aclrNormal[i])*2-Vector(1, 1, 1);

	m_iNormalWidth = iWidth;
	m_iNormalHeight = iHeight;
}
