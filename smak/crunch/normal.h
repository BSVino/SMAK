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

#pragma once

#include <parallelize.h>
#include <worklistener.h>
#include <common.h>

#include <modelconverter/convmesh.h>
#include <textures/texturehandle.h>
#include <tinker/renderer/renderer.h>

#include "ui/smakwindow.h"

class CNormalGenerator
{
public:
							CNormalGenerator(CConversionScene* pScene);
							~CNormalGenerator();

public:
	void					Think();

	void					SaveToFile(const tchar* pszFilename);

	bool					Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask = true);
	bool					Texel(size_t w, size_t h, size_t& iTexel, size_t tw, size_t th, bool* abMask = NULL);

	bool					IsGenerating() { return m_bIsGenerating; }
	bool					DoneGenerating() { return m_bDoneGenerating || m_avecNormal2Texels != NULL; }
	void					StopGenerating() { m_bStopGenerating = true; }
	bool					IsStopped() { return m_bStopGenerating; }

	void					NormalizeHeightValue(size_t x, size_t y);
	void					GeneratePass(int x, int y);
	void					Setup();
	bool					IsSettingUp();
	bool					IsSetupComplete();
	float					GetSetupProgress();

	void					SetNormalTextureDepth(float flDepth) { m_flNormalTextureDepth = flDepth; };
	void					SetNormalTextureHiDepth(float flDepth) { m_flNormalTextureHiDepth = flDepth; };
	void					SetNormalTextureMidDepth(float flDepth) { m_flNormalTextureMidDepth = flDepth; };
	void					SetNormalTextureLoDepth(float flDepth) { m_flNormalTextureLoDepth = flDepth; };
	void					SetNormalTexture(size_t iMaterial);
	size_t					GetGenerationMaterial() { return m_iMaterial; }
	void					UpdateNormal2();
	void					StartGenerationJobs();
	void					RegenerateNormal2Texture();
	bool					IsNewNormal2Available();
	bool					IsGeneratingNewNormal2();
	float					GetNormal2GenerationProgress();
	void					GetNormalMap2(CTextureHandle& hNormal2);

protected:
	CConversionScene*		m_pScene;

	IWorkListener*			m_pWorkListener;

	bool*					m_bPixelMask;

	bool					m_bIsGenerating;
	bool					m_bDoneGenerating;
	bool					m_bStopGenerating;

	size_t					m_iMaterial;
	float					m_flNormalTextureDepth;
	float					m_flNormalTextureHiDepth;
	float					m_flNormalTextureMidDepth;
	float					m_flNormalTextureLoDepth;
	size_t					m_iNormal2Width;
	size_t					m_iNormal2Height;
	Vector*					m_avecTextureTexels;
	float*					m_aflMidPassTexels;
	float*					m_aflLowPassTexels;
	Vector*					m_avecNormal2Texels;
	CTextureHandle			m_hNewNormal2;
	bool					m_bNewNormal2Available;
	bool					m_bNormal2Generated;
	bool					m_bNormal2Changed;

	CParallelizer*			m_pGenerationParallelizer;
	CParallelizer*			m_pNormal2Parallelizer;
};
