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

class CCavityGenerator
{
public:
							CCavityGenerator(CConversionScene* pScene);
							~CCavityGenerator();

public:
	void					Think();

	void					SetWorkListener(IWorkListener* pListener) { m_pWorkListener = pListener; };

	void					Generate();
	CTextureHandle			GenerateTexture(bool bInMedias = false);

	void					SaveToFile(const tchar* pszFilename);

	bool					Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask = true);
	bool					Texel(size_t w, size_t h, size_t& iTexel, size_t tw, size_t th, bool* abMask = NULL);

	bool					IsGenerating() { return m_bIsGenerating; }
	bool					DoneGenerating() { return m_bDoneGenerating; }
	void					StopGenerating() { m_bStopGenerating = true; }
	bool					IsStopped() { return m_bStopGenerating; }

	void					SetNormalTexture(CMaterialHandle hMaterial);
	void					FindCavityValue(size_t x, size_t y);

protected:
	CConversionScene*		m_pScene;

	IWorkListener*			m_pWorkListener;

	bool*					m_bPixelMask;

	bool					m_bIsGenerating;
	bool					m_bDoneGenerating;
	bool					m_bStopGenerating;

	CParallelizer*			m_pCavityParallelizer;

	tvector<Vector>			m_avecNormalTexels;
	size_t					m_iNormalWidth;
	size_t					m_iNormalHeight;

	tvector<Vector>			m_avecCavityTexels;
};
