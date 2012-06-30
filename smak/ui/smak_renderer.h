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

#include <renderer/renderer.h>
#include <textures/materialhandle.h>

class CMirror;

class CSMAKRenderer : public CRenderer
{
	DECLARE_CLASS(CSMAKRenderer, CRenderer);

public:
					CSMAKRenderer();

public:
	virtual void	Initialize();

	void			Render();
	void			DrawBackground(CRenderingContext* r);
	void			Render3D();
	void			RenderGround();
	void			RenderObjects();
	void			RenderSceneNode(class CConversionSceneNode* pNode);
	void			RenderMeshInstance(class CConversionMeshInstance* pMeshInstance);
	void			RenderLightSource();
	void			RenderUV();

	void			MoveUVLight(float flX, float flY);

	float			BloomBrightnessCutoff() const { return 1.25f; }

	CMaterialHandle&	GetWireframeTexture() { return m_hWireframe; };
	CMaterialHandle&	GetSmoothTexture() { return m_hSmooth; };
	CMaterialHandle&	GetUVTexture() { return m_hUV; };
	CMaterialHandle&	GetLightTexture() { return m_hLight; };
	CMaterialHandle&	GetTextureTexture() { return m_hTexture; };
	CMaterialHandle&	GetNormalTexture() { return m_hNormal; };
	CMaterialHandle&	GetAOTexture() { return m_hAO; };
	CMaterialHandle&	GetColorAOTexture() { return m_hCAO; };
	CMaterialHandle&	GetArrowTexture() { return m_hArrow; };
	CMaterialHandle&	GetEditTexture() { return m_hEdit; };
	CMaterialHandle&	GetVisibilityTexture() { return m_hVisibility; };
	CMaterialHandle&	GetMaterialsNodeTexture() { return m_hTexture; };
	CMaterialHandle&	GetMeshesNodeTexture() { return m_hWireframe; };
	CMaterialHandle&	GetScenesNodeTexture() { return m_hAO; };

protected:
	Vector			m_vecLightPosition;
	Vector			m_vecLightPositionUV;

	CMaterialHandle	m_hLightHalo;
	CMaterialHandle	m_hLightBeam;

	CMaterialHandle m_hWireframe;
	CMaterialHandle m_hFlat;
	CMaterialHandle m_hSmooth;
	CMaterialHandle m_hUV;
	CMaterialHandle m_hLight;
	CMaterialHandle m_hTexture;
	CMaterialHandle m_hNormal;
	CMaterialHandle m_hAO;
	CMaterialHandle m_hCAO;
	CMaterialHandle m_hArrow;
	CMaterialHandle m_hVisibility;
	CMaterialHandle m_hEdit;
};

CSMAKRenderer* SMAKRenderer();
