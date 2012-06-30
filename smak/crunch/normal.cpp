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

#include "normal.h"

#include <maths.h>
#include <stb_image_write.h>

#include <raytracer/raytracer.h>
#include <textures/materiallibrary.h>
#include <renderer/shaders.h>

#if 0
#ifdef _DEBUG
#define NORMAL_DEBUG
#endif
#endif

#ifdef NORMAL_DEBUG
#include "ui/modelwindow.h"
#endif

CNormalGenerator::CNormalGenerator(CConversionScene* pScene)
{
	m_pScene = pScene;

	m_bPixelMask = NULL;

	m_bIsGenerating = false;
	m_bDoneGenerating = false;
	m_bStopGenerating = false;

	m_iMaterial = 0;
	m_avecTextureTexels = NULL;
	m_aflMidPassTexels = NULL;
	m_aflLowPassTexels = NULL;
	m_avecNormal2Texels = NULL;
	m_bNewNormal2Available = false;
	m_bNormal2Generated = false;

	m_pGenerationParallelizer = NULL;
	m_pNormal2Parallelizer = NULL;
}

CNormalGenerator::~CNormalGenerator()
{
	free(m_bPixelMask);

	if (m_avecNormal2Texels)
	{
		delete[] m_avecTextureTexels;
		delete[] m_aflMidPassTexels;
		delete[] m_aflLowPassTexels;
		delete[] m_avecNormal2Texels;
	}
}

void CNormalGenerator::Think()
{
	if (IsSetupComplete() && m_bNormal2Changed)
	{
		StartGenerationJobs();
	}
}

void CNormalGenerator::SaveToFile(const tchar *pszFilename)
{
	if (!pszFilename)
		return;

	tstring sFilename = pszFilename;

	tvector<Color> aclrTexels;
	aclrTexels.resize(m_iNormal2Width*m_iNormal2Height);

	for (size_t i = 0; i < aclrTexels.size(); i++)
		aclrTexels[i] = m_avecNormal2Texels[i];

	if (sFilename.endswith(".png"))
		stbi_write_png(sFilename.c_str(), m_iNormal2Width, m_iNormal2Height, 3, aclrTexels.data(), 0);
	else if (sFilename.endswith(".tga"))
		stbi_write_tga(sFilename.c_str(), m_iNormal2Width, m_iNormal2Height, 3, aclrTexels.data());
	else if (sFilename.endswith(".bmp"))
		stbi_write_bmp(sFilename.c_str(), m_iNormal2Width, m_iNormal2Height, 3, aclrTexels.data());
	else
		TUnimplemented();
}

bool CNormalGenerator::Texel(size_t w, size_t h, size_t& iTexel, size_t tw, size_t th, bool* abMask)
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
	CNormalGenerator*	pGenerator;
	size_t				x;
	size_t				y;
} normal2_data_t;

void NormalizeHeightValue(void* pVoidData)
{
	normal2_data_t* pJobData = (normal2_data_t*)pVoidData;

	pJobData->pGenerator->NormalizeHeightValue(pJobData->x, pJobData->y);
}

void CNormalGenerator::NormalizeHeightValue(size_t x, size_t y)
{
	if (!m_avecTextureTexels)
		return;

	float flHiScale = ((m_iNormal2Width+m_iNormal2Height)/2.0f)/200.0f * m_flNormalTextureDepth;
	float flMidScale = ((m_iNormal2Width+m_iNormal2Height)/2.0f)/100.0f * m_flNormalTextureDepth;
	float flLowScale = ((m_iNormal2Width+m_iNormal2Height)/2.0f)/50.0f * m_flNormalTextureDepth;

	size_t iTexel;
	Texel(x, y, iTexel, m_iNormal2Width, m_iNormal2Height, false);

	tvector<Vector> avecHeights;

	float flHeight = m_avecTextureTexels[iTexel].Average() * flHiScale;
	float flMidPass = m_aflMidPassTexels[iTexel] * flMidScale;
	float flLowPass = m_aflLowPassTexels[iTexel] * flLowScale;

	Vector vecCenter((float)x, (float)y, flHeight*m_flNormalTextureHiDepth + flMidPass*m_flNormalTextureMidDepth + flLowPass*m_flNormalTextureLoDepth);
	Vector vecNormal(0,0,0);

	if (Texel(x+1, y, iTexel, m_iNormal2Width, m_iNormal2Height, false))
	{
		flHeight = m_avecTextureTexels[iTexel].Average() * flHiScale;
		flMidPass = m_aflMidPassTexels[iTexel] * flMidScale;
		flLowPass = m_aflLowPassTexels[iTexel] * flLowScale;
		Vector vecNeighbor(x+1.0f, (float)y, flHeight*m_flNormalTextureHiDepth + flMidPass*m_flNormalTextureMidDepth + flLowPass*m_flNormalTextureLoDepth);
		vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(0, 1, 0));
	}

	if (Texel(x-1, y, iTexel, m_iNormal2Width, m_iNormal2Height, false))
	{
		flHeight = m_avecTextureTexels[iTexel].Average() * flHiScale;
		flMidPass = m_aflMidPassTexels[iTexel] * flMidScale;
		flLowPass = m_aflLowPassTexels[iTexel] * flLowScale;
		Vector vecNeighbor(x-1.0f, (float)y, flHeight*m_flNormalTextureHiDepth + flMidPass*m_flNormalTextureMidDepth + flLowPass*m_flNormalTextureLoDepth);
		vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(0, -1, 0));
	}

	if (Texel(x, y+1, iTexel, m_iNormal2Width, m_iNormal2Height, false))
	{
		flHeight = m_avecTextureTexels[iTexel].Average() * flHiScale;
		flMidPass = m_aflMidPassTexels[iTexel] * flMidScale;
		flLowPass = m_aflLowPassTexels[iTexel] * flLowScale;
		Vector vecNeighbor((float)x, y+1.0f, flHeight*m_flNormalTextureHiDepth + flMidPass*m_flNormalTextureMidDepth + flLowPass*m_flNormalTextureLoDepth);
		vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(-1, 0, 0));
	}

	if (Texel(x, y-1, iTexel, m_iNormal2Width, m_iNormal2Height, false))
	{
		flHeight = m_avecTextureTexels[iTexel].Average() * flHiScale;
		flMidPass = m_aflMidPassTexels[iTexel] * flMidScale;
		flLowPass = m_aflLowPassTexels[iTexel] * flLowScale;
		Vector vecNeighbor((float)x, y-1.0f, flHeight*m_flNormalTextureHiDepth + flMidPass*m_flNormalTextureMidDepth + flLowPass*m_flNormalTextureLoDepth);
		vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(1, 0, 0));
	}

	vecNormal.Normalize();

	for (size_t i = 0; i < 3; i++)
		vecNormal[i] = RemapVal(vecNormal[i], -1.0f, 1.0f, 0.0f, 0.99f);	// Don't use 1.0 because of integer overflow.

	// Don't need to lock the data because we're guaranteed never to access the same texel twice due to the generation method.
	m_avecNormal2Texels[iTexel] = vecNormal;
}

typedef struct
{
	CNormalGenerator*	pGenerator;
	size_t				x;
	size_t				y;
} generatepass_data_t;

void GeneratePass(void* pVoidData)
{
	generatepass_data_t* pJobData = (generatepass_data_t*)pVoidData;

	pJobData->pGenerator->GeneratePass(pJobData->x, pJobData->y);
}

void CNormalGenerator::GeneratePass(int x, int y)
{
	// Generate the low/mid pass values from a fast gaussian filter.
	static const float flWeightTable[11][11] = {
		{ 0.004f, 0.004f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.004f, 0.004f },
		{ 0.004f, 0.005f, 0.005f, 0.006f, 0.007f, 0.007f, 0.007f, 0.006f, 0.005f, 0.005f, 0.004f },
		{ 0.005f, 0.005f, 0.006f, 0.008f, 0.009f, 0.009f, 0.009f, 0.008f, 0.006f, 0.005f, 0.005f },
		{ 0.005f, 0.006f, 0.008f, 0.010f, 0.012f, 0.014f, 0.012f, 0.010f, 0.008f, 0.006f, 0.005f },
		{ 0.005f, 0.007f, 0.009f, 0.012f, 0.019f, 0.027f, 0.019f, 0.012f, 0.009f, 0.007f, 0.005f },
		{ 0.005f, 0.007f, 0.009f, 0.014f, 0.027f, 0.054f, 0.027f, 0.014f, 0.009f, 0.007f, 0.005f },
		{ 0.005f, 0.007f, 0.009f, 0.012f, 0.019f, 0.027f, 0.019f, 0.012f, 0.009f, 0.007f, 0.005f },
		{ 0.005f, 0.006f, 0.008f, 0.010f, 0.012f, 0.014f, 0.012f, 0.010f, 0.008f, 0.006f, 0.005f },
		{ 0.005f, 0.005f, 0.006f, 0.008f, 0.009f, 0.009f, 0.009f, 0.008f, 0.006f, 0.005f, 0.005f },
		{ 0.004f, 0.005f, 0.005f, 0.006f, 0.007f, 0.007f, 0.007f, 0.006f, 0.005f, 0.005f, 0.004f },
		{ 0.004f, 0.004f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.004f, 0.004f },
	};

	float flLowHeight = 0;
	float flTotalLowHeight = 0;
	float flMidHeight = 0;
	float flTotalMidHeight = 0;

	for (int i = -10; i <= 10; i++)
	{
		for (int j = -10; j <= 10; j++)
		{
			size_t iTexel2;
			if (Texel(x+i, y+j, iTexel2, m_iNormal2Width, m_iNormal2Height))
			{
				size_t iTexelOffset = iTexel2;
				float flWeight = flWeightTable[(i/2)+5][(j/2)+5];
				flLowHeight += m_avecTextureTexels[iTexelOffset].Average() * flWeight;
				flTotalLowHeight += flWeight;

				if (i >= -5 && i <= 5 && j >= -5 && j <= 5)
				{
					flWeight = flWeightTable[i+5][j+5];
					flMidHeight += m_avecTextureTexels[iTexelOffset].Average() * flWeight;
					flTotalMidHeight += flWeight;
				}
			}
		}
	}

	size_t iTexel;
	Texel(x, y, iTexel, m_iNormal2Width, m_iNormal2Height);

	m_aflLowPassTexels[iTexel] = flLowHeight/flTotalLowHeight;
	m_aflMidPassTexels[iTexel] = flMidHeight/flTotalMidHeight;
}

void CNormalGenerator::Setup()
{
	if (m_pGenerationParallelizer)
		delete m_pGenerationParallelizer;
	m_pGenerationParallelizer = new CParallelizer((JobCallback)::GeneratePass);

	generatepass_data_t oJob;
	oJob.pGenerator = this;

	for (size_t x = 0; x < m_iNormal2Width; x++)
	{
		for (size_t y = 0; y < m_iNormal2Height; y++)
		{
			oJob.x = x;
			oJob.y = y;
			m_pGenerationParallelizer->AddJob(&oJob, sizeof(oJob));
		}
	}

	m_pGenerationParallelizer->Start();
}

bool CNormalGenerator::IsSettingUp()
{
	if (!m_pGenerationParallelizer)
		return false;

	if (m_pGenerationParallelizer->AreAllJobsDone())
		return false;

	return true;
}

bool CNormalGenerator::IsSetupComplete()
{
	if (IsSettingUp())
		return false;

	if (!m_pGenerationParallelizer)
		return false;

	return m_pGenerationParallelizer->AreAllJobsDone();
}

float CNormalGenerator::GetSetupProgress()
{
	if (!m_pGenerationParallelizer)
		return 0;

	if (m_pGenerationParallelizer->GetJobsTotal() == 0)
		return 0;

	return (float)m_pGenerationParallelizer->GetJobsDone() / (float)m_pGenerationParallelizer->GetJobsTotal();
}

void CNormalGenerator::SetNormalTexture(size_t iMaterial)
{
	// Materials not loaded yet?
	if (iMaterial >= SMAKWindow()->GetMaterials().size())
		return;

	CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[iMaterial];

	CShader* pShader = CShaderLibrary::GetShader(hMaterial->m_sShader);
	size_t iDiffuse = pShader->FindTextureByUniform("iDiffuse");
	if (iDiffuse >= hMaterial->m_ahTextures.size())
		return;

	CTextureHandle hDiffuseTexture = hMaterial->m_ahTextures[iDiffuse];

	if (!hDiffuseTexture.IsValid())
		return;

	m_iMaterial = iMaterial;

	// Don't let the listeners know yet, we want to generate the new one first so there is no lapse in displaying.
//	m_bNewNormal2Available = true;

	if (!m_avecTextureTexels)
		m_avecTextureTexels = new Vector[hDiffuseTexture->m_iWidth*hDiffuseTexture->m_iHeight];

	CRenderer::ReadTextureFromGL(hDiffuseTexture, m_avecTextureTexels);

	m_iNormal2Width = hDiffuseTexture->m_iWidth;
	m_iNormal2Height = hDiffuseTexture->m_iHeight;

	if (!m_aflLowPassTexels)
		m_aflLowPassTexels = new float[hDiffuseTexture->m_iWidth*hDiffuseTexture->m_iHeight];
	if (!m_aflMidPassTexels)
		m_aflMidPassTexels = new float[hDiffuseTexture->m_iWidth*hDiffuseTexture->m_iHeight];

	if (!m_avecNormal2Texels)
		m_avecNormal2Texels = new Vector[hDiffuseTexture->m_iWidth*hDiffuseTexture->m_iHeight];

	Setup();

	UpdateNormal2();
}

void CNormalGenerator::UpdateNormal2()
{
	m_bNormal2Changed = true;
	m_bNormal2Generated = false;
}

void CNormalGenerator::StartGenerationJobs()
{
	m_bNormal2Changed = false;

	if (m_pNormal2Parallelizer && m_avecNormal2Texels)
	{
		m_pNormal2Parallelizer->RestartJobs();
		return;
	}

	if (m_pNormal2Parallelizer)
		delete m_pNormal2Parallelizer;
	m_pNormal2Parallelizer = new CParallelizer((JobCallback)::NormalizeHeightValue);

	normal2_data_t oJob;
	oJob.pGenerator = this;

	for (size_t x = 0; x < m_iNormal2Width; x++)
	{
		for (size_t y = 0; y < m_iNormal2Height; y++)
		{
			oJob.x = x;
			oJob.y = y;
			m_pNormal2Parallelizer->AddJob(&oJob, sizeof(oJob));
		}
	}

	m_pNormal2Parallelizer->Start();
}

void CNormalGenerator::RegenerateNormal2Texture()
{
	if (!m_avecNormal2Texels)
		return;

	m_hNewNormal2 = CTextureLibrary::AddTexture(m_avecNormal2Texels, m_iNormal2Width, m_iNormal2Height);

	m_bNewNormal2Available = true;
	m_bNormal2Generated = true;
}

bool CNormalGenerator::IsNewNormal2Available()
{
	if (m_pNormal2Parallelizer)
	{
		if (m_pNormal2Parallelizer->AreAllJobsDone() && !m_bNormal2Generated && !m_bNormal2Changed)
		{
			RegenerateNormal2Texture();

			m_bNewNormal2Available = true;
		}
	}

	return m_bNewNormal2Available;
}

bool CNormalGenerator::IsGeneratingNewNormal2()
{
	if (!m_pNormal2Parallelizer)
		return false;

	if (m_pNormal2Parallelizer->AreAllJobsDone())
		return false;

	return true;
}

float CNormalGenerator::GetNormal2GenerationProgress()
{
	if (!m_pNormal2Parallelizer)
		return 0;

	if (m_pNormal2Parallelizer->GetJobsTotal() == 0)
		return 0;

	return (float)m_pNormal2Parallelizer->GetJobsDone() / (float)m_pNormal2Parallelizer->GetJobsTotal();
}

void CNormalGenerator::GetNormalMap2(CTextureHandle& hNormal2)
{
	hNormal2 = m_hNewNormal2;

	m_bNewNormal2Available = false;
}
