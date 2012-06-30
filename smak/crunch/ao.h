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

namespace raytrace
{
	class CRaytracer;
	class CTraceResult;
};

typedef enum
{
	AOMETHOD_NONE = 0,
	AOMETHOD_RENDER,
	AOMETHOD_RAYTRACE,
	AOMETHOD_SHADOWMAP,
} aomethod_t;

class CAOGenerator
{
public:
							CAOGenerator(CConversionScene* pScene);
							~CAOGenerator();

public:
	void					SetMethod(aomethod_t eMethod) { m_eAOMethod = eMethod; };
	void					SetSize(size_t iWidth, size_t iHeight);
	void					SetUseTexture(bool bUseTexture) { m_bUseTexture = bUseTexture; };
	void					SetBleed(size_t iBleed) { m_iBleed = iBleed; };
	void					SetRenderPreviewViewport(int x, int y, int w, int h);
	void					SetSamples(size_t iSamples) { m_iSamples = iSamples; };
	void					SetRandomize(bool bRandomize) { m_bRandomize = bRandomize; };
	void					SetCreaseEdges(bool bCreaseEdges) { m_bCreaseEdges = bCreaseEdges; };
	void					SetGroundOcclusion(bool bGroundOcclusion) { m_bGroundOcclusion = bGroundOcclusion; };
	void					SetRayFalloff(float flRayFalloff) { m_flRayFalloff = flRayFalloff; };

	void					SetWorkListener(IWorkListener* pListener) { m_pWorkListener = pListener; };

	void					ShadowMapSetupScene();
	void					ShadowMapSetupSceneNode(CConversionSceneNode* pNode, tvector<float>& aflVerts, bool bDepth);
	void					RenderSetupScene();
	void					RenderSetupSceneNode(CConversionSceneNode* pNode, tvector<tvector<float>>& aaflVerts);
	void					Generate();
	void					GenerateShadowMaps();
	void					GenerateByTexel();
	void					GenerateNodeByTexel(CConversionSceneNode* pNode, class raytrace::CRaytracer* pTracer, size_t& iRendered);
	void					GenerateTriangleByTexel(CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t v1, size_t v2, size_t v3, class raytrace::CRaytracer* pTracer, size_t& iRendered);
	Vector					RenderSceneFromPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pFace);
	void					DebugRenderSceneLookAtPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pRenderFace);
	void					AccumulateTexture(size_t iTexture);
	void					Bleed();

	void					RaytraceSetupThreads();
	void					RaytraceCleanupThreads();
	void					RaytraceSceneMultithreaded(class raytrace::CRaytracer* pTracer, Vector vecPosition, Vector vecDirection, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t iTexel);
	void					RaytraceSceneFromPosition(class raytrace::CRaytracer* pTracer, Vector vecPosition, Vector vecDirection, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t iTexel);
	void					RaytraceJoinThreads();

	CTextureHandle			GenerateTexture(bool bInMedias = false);
	void					SaveToFile(const tchar* pszFilename);

	bool					Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask = true);

	bool					IsGenerating() { return m_bIsGenerating; }
	bool					DoneGenerating() { return m_bDoneGenerating; }
	void					StopGenerating() { m_bStopGenerating = true; }
	bool					IsStopped() { return m_bStopGenerating; }

protected:
	CConversionScene*		m_pScene;

	size_t					m_iWidth;
	size_t					m_iHeight;
	bool					m_bUseTexture;
	size_t					m_iBleed;
	size_t					m_iSamples;
	bool					m_bRandomize;
	bool					m_bCreaseEdges;
	bool					m_bGroundOcclusion;
	float					m_flRayFalloff;
	int						m_iRPVX;
	int						m_iRPVY;
	int						m_iRPVW;
	int						m_iRPVH;
	aomethod_t				m_eAOMethod;

	IWorkListener*			m_pWorkListener;

	size_t					m_iPixelDepth;
	Vector4D*				m_pvecPixels;
	bool*					m_bPixelMask;

	size_t					m_iScene;
	size_t					m_iSceneVerts;
	size_t					m_iSceneDepth;
	size_t					m_iSceneDepthVerts;

	tvector<size_t>			m_aiSceneMaterials;
	tvector<size_t>			m_aiSceneMaterialVerts;
	CFrameBuffer			m_oRenderFB;

	Vector*					m_avecShadowValues;
	Vector*					m_avecShadowGeneratedValues;
	size_t*					m_aiShadowReads;
	float					m_flLowestValue;
	float					m_flHighestValue;
	CFrameBuffer			m_oAOFB;

	bool					m_bIsGenerating;
	bool					m_bIsBleeding;
	bool					m_bDoneGenerating;
	bool					m_bStopGenerating;

	CParallelizer*			m_pRaytraceParallelizer;
};
