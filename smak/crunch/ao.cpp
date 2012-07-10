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

#include <time.h>

#include <geometry.h>
#include <maths.h>
#include <matrix.h>
#include <mtrand.h>
#include <raytracer/raytracer.h>
#include <tinker_platform.h>
#include <textures/texturelibrary.h>
#include <renderer/renderingcontext.h>
#include <tinker/cvar.h>

#include "smak/ui/smak_renderer.h"
#include "ui/smakwindow.h"

void DrawTexture(size_t iTexture, float flScale, CRenderingContext& c);

CVar ao_debug("ao_debug", "0");

CAOGenerator::CAOGenerator(CConversionScene* pScene)
{
	m_eAOMethod = AOMETHOD_NONE;
	m_pScene = pScene;

	m_avecShadowValues = NULL;
	m_avecShadowGeneratedValues = NULL;
	m_aiShadowReads = NULL;
	m_bPixelMask = NULL;
	m_pvecPixels = NULL;

	// Default options
	SetSize(512, 512);
	m_bUseTexture = true;
	m_iBleed = 5;
	m_iSamples = 15;
	m_bRandomize = false;
	m_bCreaseEdges = true;
	m_bGroundOcclusion = false;
	m_flRayFalloff = 1;

	SetRenderPreviewViewport(0, 0, 100, 100);

	m_pWorkListener = NULL;

	m_bIsGenerating = false;
	m_bDoneGenerating = false;
	m_bStopGenerating = false;

	m_pRaytraceParallelizer = NULL;
}

CAOGenerator::~CAOGenerator()
{
	delete[] m_pvecPixels;
	free(m_bPixelMask);
	delete[] m_avecShadowValues;
	delete[] m_avecShadowGeneratedValues;
	delete[] m_aiShadowReads;

	if (m_pRaytraceParallelizer)
		delete m_pRaytraceParallelizer;
}

void CAOGenerator::SetSize(size_t iWidth, size_t iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	if (m_avecShadowValues)
	{
		delete[] m_avecShadowValues;
		delete[] m_avecShadowGeneratedValues;
		delete[] m_aiShadowReads;
	}

	// Shadow volume result buffer.
	m_avecShadowValues = new Vector[iWidth*iHeight];
	m_avecShadowGeneratedValues = new Vector[iWidth*iHeight];
	m_aiShadowReads = new size_t[iWidth*iHeight];

	// Big hack incoming!
	memset(&m_avecShadowValues[0].x, 0, iWidth*iHeight*sizeof(Vector));
	memset(&m_avecShadowGeneratedValues[0].x, 0, iWidth*iHeight*sizeof(Vector));
	memset(&m_aiShadowReads[0], 0, iWidth*iHeight*sizeof(size_t));

	if (m_bPixelMask)
		free(m_bPixelMask);
	m_bPixelMask = (bool*)malloc(m_iWidth*m_iHeight*sizeof(bool));
}

void CAOGenerator::SetRenderPreviewViewport(int x, int y, int w, int h)
{
	m_iRPVX = x;
	m_iRPVY = y;
	m_iRPVW = w;
	m_iRPVH = h;

	if (m_pvecPixels)
		delete[] m_pvecPixels;

	// Pixel reading buffer
	m_iPixelDepth = 4;
	m_pvecPixels = new Vector4D[m_iRPVW*m_iRPVH];
}

void CAOGenerator::ShadowMapSetupScene()
{
	// Create a list with the required polys so it draws quicker.
	tvector<float> aflVerts, aflVertsDepth;

	// Overload the render preview viewport as a method for storing our pixels.
	SetRenderPreviewViewport(0, 0, (int)m_iWidth, (int)m_iHeight);

	ShadowMapSetupSceneNode(m_pScene->GetScene(0), aflVertsDepth, true);

	// Overload the render preview viewport as a method for storing our pixels.
	SetRenderPreviewViewport(0, 0, (int)m_iWidth, (int)m_iHeight);

	ShadowMapSetupSceneNode(m_pScene->GetScene(0), aflVerts, false);

	m_iScene = CRenderer::LoadVertexDataIntoGL(aflVerts.size()*sizeof(float), aflVerts.data());
	m_iSceneVerts = aflVerts.size()/8;
	m_iSceneDepth = CRenderer::LoadVertexDataIntoGL(aflVertsDepth.size()*sizeof(float), aflVertsDepth.data());
	m_iSceneDepthVerts = aflVertsDepth.size()/8;
}

void AddShadowMapVertex(tvector<float>& aflVerts, const Vector& v, const Vector& vn, const Vector2D& vt)
{
	aflVerts.push_back(v.x);
	aflVerts.push_back(v.y);
	aflVerts.push_back(v.z);
	aflVerts.push_back(vn.x);
	aflVerts.push_back(vn.y);
	aflVerts.push_back(vn.z);
	aflVerts.push_back(vt.x);
	aflVerts.push_back(vt.y);
}

void CAOGenerator::ShadowMapSetupSceneNode(CConversionSceneNode* pNode, tvector<float>& aflVerts, bool bDepth)
{
	if (!pNode)
		return;

	for (size_t c = 0; c < pNode->GetNumChildren(); c++)
		ShadowMapSetupSceneNode(pNode->GetChild(c), aflVerts, bDepth);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
	{
		CConversionMeshInstance* pMeshInstance = pNode->GetMeshInstance(m);
		CConversionMesh* pMesh = pMeshInstance->GetMesh();
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);

			if (!bDepth)
			{
				// Allow this in the depth model so that it still projects a shadow, but we don't produce a map for it.
				if (pFace->m != ~0 && pMeshInstance->GetMappedMaterial(pFace->m))
				{
					if (!pMeshInstance->GetMappedMaterial(pFace->m)->IsVisible())
						continue;

					CConversionMaterial* pMaterial = m_pScene->GetMaterial(pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial);
					if (pMaterial && !pMaterial->IsVisible())
						continue;
				}
			}

			CConversionVertex* pVertex0 = pFace->GetVertex(0);

			for (size_t k = 2; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex1 = pFace->GetVertex(k-1);
				CConversionVertex* pVertex2 = pFace->GetVertex(k);

				AddShadowMapVertex(aflVerts, pMeshInstance->GetVertex(pVertex0->v), pMeshInstance->GetNormal(pVertex0->vn), pMesh->GetUV(pVertex0->vu));
				AddShadowMapVertex(aflVerts, pMeshInstance->GetVertex(pVertex1->v), pMeshInstance->GetNormal(pVertex1->vn), pMesh->GetUV(pVertex1->vu));
				AddShadowMapVertex(aflVerts, pMeshInstance->GetVertex(pVertex2->v), pMeshInstance->GetNormal(pVertex2->vn), pMesh->GetUV(pVertex2->vu));
			}
		}
	}
}

void CAOGenerator::RenderSetupScene()
{
	TUnimplemented();

	tvector<tvector<float>> aaflVerts;
	RenderSetupSceneNode(m_pScene->GetScene(0), aaflVerts);

	for (size_t i = 0; i < aaflVerts.size(); i++)
	{
		m_aiSceneMaterials.push_back(CRenderer::LoadVertexDataIntoGL(aaflVerts[i].size()*sizeof(float), aaflVerts[i].data()));
		m_aiSceneMaterialVerts.push_back(aaflVerts[i].size()/8);
	}

	m_oRenderFB = SMAKRenderer()->CreateFrameBuffer(m_iWidth, m_iHeight, (fb_options_e)(FB_TEXTURE|FB_LINEAR|FB_DEPTH));
}

void AddRenderedVertex(tvector<float>& aflVerts, const Vector& v, const Vector2D& vt)
{
	aflVerts.push_back(v.x);
	aflVerts.push_back(v.y);
	aflVerts.push_back(v.z);
	aflVerts.push_back(vt.x);
	aflVerts.push_back(vt.y);
}

void CAOGenerator::RenderSetupSceneNode(CConversionSceneNode* pNode, tvector<tvector<float>>& aaflVerts)
{
	if (!pNode)
		return;

	for (size_t c = 0; c < pNode->GetNumChildren(); c++)
		RenderSetupSceneNode(pNode->GetChild(c), aaflVerts);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
	{
		CConversionMeshInstance* pMeshInstance = pNode->GetMeshInstance(m);
		CConversionMesh* pMesh = pMeshInstance->GetMesh();
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);

			size_t iMaterial = pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial;
			while (aaflVerts.size() <= iMaterial)
				aaflVerts.push_back();

			CConversionVertex* pVertex0 = pFace->GetVertex(0);

			for (size_t k = 2; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex1 = pFace->GetVertex(k-1);
				CConversionVertex* pVertex2 = pFace->GetVertex(k);

				AddRenderedVertex(aaflVerts[iMaterial], pMeshInstance->GetVertex(pVertex0->v), pMesh->GetUV(pVertex0->vu));
				AddRenderedVertex(aaflVerts[iMaterial], pMeshInstance->GetVertex(pVertex1->v), pMesh->GetUV(pVertex1->vu));
				AddRenderedVertex(aaflVerts[iMaterial], pMeshInstance->GetVertex(pVertex2->v), pMesh->GetUV(pVertex2->vu));
			}
		}
	}
}

void CAOGenerator::Generate()
{
	if (!m_eAOMethod)
		return;

	if (m_pWorkListener)
	{
		m_pWorkListener->BeginProgress();
		m_pWorkListener->SetAction("Setting up", 0);
	}

	m_bIsGenerating = true;
	m_bStopGenerating = false;
	m_bDoneGenerating = false;
	m_bIsBleeding = false;

	m_flLowestValue = -1;
	m_flHighestValue = 0;

	if (SMAKWindow())
		SMAKWindow()->ClearDebugLines();

	memset(&m_bPixelMask[0], 0, m_iWidth*m_iHeight*sizeof(bool));

	if (m_eAOMethod == AOMETHOD_SHADOWMAP)
	{
		ShadowMapSetupScene();
		GenerateShadowMaps();
	}
	else
	{
		if (m_eAOMethod == AOMETHOD_RENDER)
			RenderSetupScene();
		// In AO debug mode we need this to do the debug rendering, so do it anyways.
		else if (ao_debug.GetBool())
			RenderSetupScene();

		GenerateByTexel();
	}

	size_t i;

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Averaging reads", m_iWidth*m_iHeight);

	// Average out all of the reads.
	for (i = 0; i < m_iWidth*m_iHeight; i++)
	{
		// Don't immediately return, just skip this loop. We have cleanup work to do.
		if (m_bStopGenerating)
			break;

		if (m_eAOMethod == AOMETHOD_SHADOWMAP)
			m_avecShadowValues[i] = Vector(m_avecShadowValues[i].x, m_avecShadowValues[i].x, m_avecShadowValues[i].x);

		if (m_aiShadowReads[i])
			m_avecShadowValues[i] /= (float)m_aiShadowReads[i];
		else
			m_avecShadowValues[i] = Vector(0,0,0);

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(i);
	}

	if (m_eAOMethod == AOMETHOD_RENDER || m_eAOMethod == AOMETHOD_SHADOWMAP)
	{
		if (m_eAOMethod == AOMETHOD_SHADOWMAP)
		{
			m_oAOFB.Destroy();
			CRenderer::UnloadVertexDataFromGL(m_iScene);
			CRenderer::UnloadVertexDataFromGL(m_iSceneDepth);
		}
		else
		{
			for (size_t i = 0; i < m_aiSceneMaterials.size(); i++)
				CRenderer::UnloadVertexDataFromGL(m_aiSceneMaterials[i]);

			m_oRenderFB.Destroy();
		}
	}

	// Somebody get this ao some clotters and morphine, STAT!
	m_bIsBleeding = true;
	if (!m_bStopGenerating)
		Bleed();
	m_bIsBleeding = false;

	if (!m_bStopGenerating)
		m_bDoneGenerating = true;
	m_bIsGenerating = false;

	// One last call to let them know we're done.
	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}

void CAOGenerator::GenerateShadowMaps()
{
	double flProcessSceneRead = 0;
	double flProgress = 0;

	size_t iShadowMapSize = 1024;

	// A frame buffer for holding the depth buffer shadow render
	CFrameBuffer oDepthFB = SMAKRenderer()->CreateFrameBuffer(iShadowMapSize, iShadowMapSize, (fb_options_e)(FB_DEPTH_TEXTURE|FB_RENDERBUFFER)); // RB unused

	// A frame buffer for holding the UV layout once it is rendered flat with the shadow
	CFrameBuffer oUVFB = SMAKRenderer()->CreateFrameBuffer(m_iWidth, m_iHeight, (fb_options_e)(FB_TEXTURE|FB_LINEAR|FB_DEPTH)); // Depth unused

	// A frame buffer for holding the completed AO map
	m_oAOFB = SMAKRenderer()->CreateFrameBuffer(m_iWidth, m_iHeight, (fb_options_e)(FB_TEXTURE|FB_TEXTURE_HALF_FLOAT|FB_LINEAR|FB_DEPTH)); // Depth unused

	CRenderingContext c(SMAKRenderer());

	c.UseFrameBuffer(&m_oAOFB);

	c.ClearColor(Color(0, 0, 0, 0));

	c.SetDepthFunction(DF_LEQUAL);
	c.SetDepthTest(true);
	c.SetBackCulling(false);

	Matrix4x4 mBias(
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.5f, 0.0f,
	0.5f, 0.5f, 0.5f, 1.0f); // Bias from [-1, 1] to [0, 1]

	AABB oBox = m_pScene->m_oExtends;
	Vector vecCenter = oBox.Center();
	float flSize = oBox.Size().Length();	// Length of the box's diagonal

	Matrix4x4 mLightProjection = Matrix4x4::ProjectOrthographic(-flSize/2, flSize/2, -flSize/2, flSize/2, 1, flSize*2);

	size_t iSamples = (size_t)sqrt((float)m_iSamples);

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Taking exposures", m_iSamples);

	for (size_t x = 0; x <= iSamples; x++)
	{
		float flPitch = -asin(RemapVal((float)x, 0, (float)iSamples, -1, 1)) * 90 / (M_PI/2);

		for (size_t y = 0; y < iSamples; y++)
		{
			if (x == 0 || x == iSamples)
			{
				// Don't do a bunch of samples from the same spot on the poles.
				if (y != 0)
					continue;
			}

			float flYaw = RemapVal((float)y, 0, (float)iSamples, -180, 180);

			// Randomize the direction a tad to help fight moire
			Vector vecDir = AngleVector(EAngle(flPitch+RandomFloat(-1, 1)/2, flYaw+RandomFloat(-1, 1)/2, 0));
			Vector vecLightPosition = vecDir*flSize + vecCenter;	// Puts us twice as far from the closest vertex

			if (ao_debug.GetInt() > 1)
				SMAKWindow()->AddDebugLine(vecLightPosition, vecLightPosition-vecDir);

			Matrix4x4 mLightView = Matrix4x4::ConstructCameraView(vecLightPosition, (vecCenter-vecLightPosition).Normalized(), Vector(0, 1, 0));

			c.SetProjection(mLightProjection);
			c.SetView(mLightView);

			// If we're looking from below and ground occlusion is on, don't bother with this render.
			if (!(flPitch < -10 && m_bGroundOcclusion))
			{
				c.UseProgram("model");
				c.UseFrameBuffer(&oDepthFB);
				c.SetViewport(Rect(0, 0, iShadowMapSize, iShadowMapSize));
				c.SetBackCulling(false);
				c.ClearDepth();

				c.BeginRenderVertexArray(m_iSceneDepth);
				c.SetPositionBuffer((size_t)0, 8*sizeof(float));
				c.SetNormalsBuffer((size_t)3*sizeof(float), 8*sizeof(float));
				c.SetTexCoordBuffer((size_t)6*sizeof(float), 8*sizeof(float));
				c.EndRenderVertexArray(m_iSceneDepthVerts);

				c.UseFrameBuffer(nullptr);

				if (ao_debug.GetBool())
				{
					CRenderingContext c(SMAKRenderer());
					c.SetViewport(Rect(0, 0, iShadowMapSize/2, iShadowMapSize/2));

					DrawTexture(oDepthFB.m_iDepthTexture, 1, c);
				}
			}

			Matrix4x4 mTextureMatrix = mBias*mLightProjection*mLightView;

			{
				CRenderingContext c(SMAKRenderer(), true);

				c.UseFrameBuffer(&oUVFB);
				c.SetViewport(Rect(0, 0, m_iWidth, m_iHeight));
				c.ClearColor(Color(0, 0, 0, 0));
				c.ClearDepth();

				c.UseProgram("flat_shadow");
				c.SetUniform("mBiasedLightMatrix", mTextureMatrix);
				c.SetUniform("iShadowMap", 0);
				c.SetUniform("vecLightNormal", -vecDir);
				c.SetUniform("bOccludeAll", (flPitch < -10 && m_bGroundOcclusion));
				c.SetUniform("flTime", (float)Application()->GetTime());
				c.BindTexture(oDepthFB.m_iDepthTexture);

				c.BeginRenderVertexArray(m_iScene);
				c.SetPositionBuffer((size_t)0, 8*sizeof(float));
				c.SetNormalsBuffer((size_t)3*sizeof(float), 8*sizeof(float));
				c.SetTexCoordBuffer((size_t)6*sizeof(float), 8*sizeof(float));
				c.EndRenderVertexArray(m_iSceneVerts);
			}

			if (ao_debug.GetBool())
			{
				CRenderingContext c(SMAKRenderer());
				c.SetViewport(Rect(iShadowMapSize/2, 0, m_iWidth, m_iHeight));
				DrawTexture(oUVFB.m_iMap, 1, c);
			}

			double flTimeBefore = SMAKWindow()->GetTime();

			c.SetViewport(Rect(0, 0, m_iWidth, m_iHeight));
			c.UseFrameBuffer(&m_oAOFB);
			AccumulateTexture(oUVFB.m_iMap);
			c.UseFrameBuffer(nullptr);

			if (ao_debug.GetBool())
			{
				CRenderingContext c(SMAKRenderer());
				c.UseProgram("ao");
				c.SetViewport(Rect(iShadowMapSize/2+m_iWidth, 0, m_iWidth, m_iHeight));
				c.SetUniform("iAOMap", 0);
				c.SetBlend(BLEND_ALPHA);
				DrawTexture(m_oAOFB.m_iMap, 1, c);
			}

			flProcessSceneRead += (SMAKWindow()->GetTime() - flTimeBefore);
			flTimeBefore = SMAKWindow()->GetTime();

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(x*iSamples + y);

			flProgress += (SMAKWindow()->GetTime() - flTimeBefore);

			if (m_bStopGenerating)
				break;
		}

		if (m_bStopGenerating)
			break;
	}

	c.UseFrameBuffer(&m_oAOFB);
	c.ReadPixels(0, 0, m_iWidth, m_iHeight, m_pvecPixels);
	c.UseFrameBuffer(nullptr);

	if (!m_bStopGenerating)
	{
		size_t iBufferSize = m_iWidth*m_iHeight;

		if (m_pWorkListener)
			m_pWorkListener->SetAction("Reading pixels", iBufferSize);

		for (size_t p = 0; p < iBufferSize; p++)
		{
			if (m_pvecPixels[p].w == 0.0f)
				continue;

			m_avecShadowValues[p].x = m_pvecPixels[p].x;
			m_aiShadowReads[p] = (size_t)m_pvecPixels[p].w;
			m_bPixelMask[p] = true;

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(p);
		}
	}

	oDepthFB.Destroy();
	oUVFB.Destroy();
	// Don't destroy m_oAOFB yet, we need it in a bit. It gets destroyed later.
}

void CAOGenerator::AccumulateTexture(size_t iTexture)
{
	CRenderingContext c(SMAKRenderer(), true);

	c.UseProgram("quad");
	c.SetDepthTest(false);
	c.SetUniform("iDiffuse", 0);
	c.BindTexture(iTexture);
	c.SetBlend(BLEND_BOTH);

	c.BeginRenderTriFan();
		c.TexCoord(Vector2D(0, 0));
		c.Vertex(Vector(-1, -1, 0));

		c.TexCoord(Vector2D(0, 1));
		c.Vertex(Vector(-1, 1, 0));

		c.TexCoord(Vector2D(1, 1));
		c.Vertex(Vector(1, 1, 0));

		c.TexCoord(Vector2D(1, 0));
		c.Vertex(Vector(1, -1, 0));
	c.EndRender();
}

void CAOGenerator::GenerateByTexel()
{
	if (m_eAOMethod == AOMETHOD_RAYTRACE)
		RaytraceSetupThreads();

	float flTotalArea = 0;

	for (size_t m = 0; m < m_pScene->GetNumMeshes(); m++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(m);
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);
			flTotalArea += pFace->GetUVArea();
		}
	}

	raytrace::CRaytracer* pTracer = NULL;

	if (m_eAOMethod == AOMETHOD_RAYTRACE)
	{
		if (m_pWorkListener)
			m_pWorkListener->SetAction("Building tree", 0);

		pTracer = new raytrace::CRaytracer(m_pScene);
		pTracer->AddMeshesFromNode(m_pScene->GetScene(0));
		pTracer->BuildTree();

		srand((unsigned int)time(0));
	}

	if (m_pWorkListener)
	{
		if (m_eAOMethod == AOMETHOD_RAYTRACE && GetNumberOfProcessors() > 1)
			m_pWorkListener->SetAction("Dispatching jobs", (size_t)(flTotalArea*m_iWidth*m_iHeight));
		else
			m_pWorkListener->SetAction("Rendering", (size_t)(flTotalArea*m_iWidth*m_iHeight));
	}

	size_t iRendered = 0;

	if (m_pScene->GetNumScenes())
		GenerateNodeByTexel(m_pScene->GetScene(0), pTracer, iRendered);

	if (m_eAOMethod == AOMETHOD_RAYTRACE)
	{
		RaytraceJoinThreads();
		RaytraceCleanupThreads();
		delete pTracer;
	}
}

void CAOGenerator::GenerateNodeByTexel(CConversionSceneNode* pNode, raytrace::CRaytracer* pTracer, size_t& iRendered)
{
	for (size_t c = 0; c < pNode->GetNumChildren(); c++)
		GenerateNodeByTexel(pNode->GetChild(c), pTracer, iRendered);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
	{
		CConversionMeshInstance* pMeshInstance = pNode->GetMeshInstance(m);
		CConversionMesh* pMesh = pMeshInstance->GetMesh();

		if (!pMesh->GetNumUVs())
			continue;

		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);

			if (pFace->m != ~0)
			{
				if (!pMeshInstance->GetMappedMaterial(pFace->m)->IsVisible())
					continue;

				CConversionMaterial* pMaterial = m_pScene->GetMaterial(pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial);
				if (pMaterial && !pMaterial->IsVisible())
					continue;
			}

			tvector<Vector> avecPoints;
			tvector<size_t> aiPoints;
			for (size_t t = 0; t < pFace->GetNumVertices(); t++)
			{
				avecPoints.push_back(pMeshInstance->GetVertex(pFace->GetVertex(t)->v));
				aiPoints.push_back(t);
			}

			while (avecPoints.size() > 3)
			{
				size_t iEar = FindEar(avecPoints);
				size_t iLast = iEar==0?avecPoints.size()-1:iEar-1;
				size_t iNext = iEar==avecPoints.size()-1?0:iEar+1;
				GenerateTriangleByTexel(pMeshInstance, pFace, aiPoints[iLast], aiPoints[iEar], aiPoints[iNext], pTracer, iRendered);
				avecPoints.erase(avecPoints.begin()+iEar);
				aiPoints.erase(aiPoints.begin()+iEar);
				if (m_bStopGenerating)
					break;
			}
			GenerateTriangleByTexel(pMeshInstance, pFace, aiPoints[0], aiPoints[1], aiPoints[2], pTracer, iRendered);
			if (m_bStopGenerating)
				break;
		}
		if (m_bStopGenerating)
			break;
	}
}

void CAOGenerator::GenerateTriangleByTexel(CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t v1, size_t v2, size_t v3, raytrace::CRaytracer* pTracer, size_t& iRendered)
{
	CConversionVertex* pV1 = pFace->GetVertex(v1);
	CConversionVertex* pV2 = pFace->GetVertex(v2);
	CConversionVertex* pV3 = pFace->GetVertex(v3);

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	Vector vu1 = pMesh->GetUV(pV1->vu);
	Vector vu2 = pMesh->GetUV(pV2->vu);
	Vector vu3 = pMesh->GetUV(pV3->vu);

	Vector vecLoUV = vu1;
	Vector vecHiUV = vu1;

	if (vu2.x < vecLoUV.x)
		vecLoUV.x = vu2.x;
	if (vu3.x < vecLoUV.x)
		vecLoUV.x = vu3.x;
	if (vu2.x > vecHiUV.x)
		vecHiUV.x = vu2.x;
	if (vu3.x > vecHiUV.x)
		vecHiUV.x = vu3.x;

	if (vu2.y < vecLoUV.y)
		vecLoUV.y = vu2.y;
	if (vu3.y < vecLoUV.y)
		vecLoUV.y = vu3.y;
	if (vu2.y > vecHiUV.y)
		vecHiUV.y = vu2.y;
	if (vu3.y > vecHiUV.y)
		vecHiUV.y = vu3.y;

	size_t iLoX = (size_t)(vecLoUV.x * m_iWidth);
	size_t iLoY = (size_t)(vecLoUV.y * m_iHeight);
	size_t iHiX = (size_t)(vecHiUV.x * m_iWidth);
	size_t iHiY = (size_t)(vecHiUV.y * m_iHeight);

	for (size_t i = iLoX; i <= iHiX; i++)
	{
		for (size_t j = iLoY; j <= iHiY; j++)
		{
			float flU = ((float)i + 0.5f)/(float)m_iWidth;
			float flV = ((float)j + 0.5f)/(float)m_iHeight;

			bool bInside = PointInTriangle(Vector(flU,flV,0), vu1, vu2, vu3);

			if (!bInside)
				continue;

			Vector v1 = pMeshInstance->GetVertex(pV1->v);
			Vector v2 = pMeshInstance->GetVertex(pV2->v);
			Vector v3 = pMeshInstance->GetVertex(pV3->v);

			Vector vn1 = pMeshInstance->GetNormal(pV1->vn);
			Vector vn2 = pMeshInstance->GetNormal(pV2->vn);
			Vector vn3 = pMeshInstance->GetNormal(pV3->vn);

			// Find where the UV is in world space.

			// First build 2x2 a "matrix" of the UV values.
			float mta = vu2.x - vu1.x;
			float mtb = vu3.x - vu1.x;
			float mtc = vu2.y - vu1.y;
			float mtd = vu3.y - vu1.y;

			// Invert it.
			float d = mta*mtd - mtb*mtc;
			float mtia =  mtd / d;
			float mtib = -mtb / d;
			float mtic = -mtc / d;
			float mtid =  mta / d;

			// Now build a 2x3 "matrix" of the vertices.
			float mva = v2.x - v1.x;
			float mvb = v3.x - v1.x;
			float mvc = v2.y - v1.y;
			float mvd = v3.y - v1.y;
			float mve = v2.z - v1.z;
			float mvf = v3.z - v1.z;

			// Multiply them together.
			// [a b]   [a b]   [a b]
			// [c d] * [c d] = [c d]
			// [e f]           [e f]
			// Really wish I had a matrix math library about now!
			float mra = mva*mtia + mvb*mtic;
			float mrb = mva*mtib + mvb*mtid;
			float mrc = mvc*mtia + mvd*mtic;
			float mrd = mvc*mtib + mvd*mtid;
			float mre = mve*mtia + mvf*mtic;
			float mrf = mve*mtib + mvf*mtid;

			// These vectors should be the U and V axis in world space.
			Vector vecUAxis(mra, mrc, mre);
			Vector vecVAxis(mrb, mrd, mrf);

			Vector vecUVOrigin = v1 - vecUAxis * vu1.x - vecVAxis * vu1.y;

			Vector vecUVPosition = vecUVOrigin + vecUAxis * flU + vecVAxis * flV;

			Vector vecNormal;

			if (m_bCreaseEdges)
				vecNormal = pFace->GetNormal();
			else
			{
				float wv1 = DistanceToLine(vecUVPosition, v2, v3) / DistanceToLine(v1, v2, v3);
				float wv2 = DistanceToLine(vecUVPosition, v1, v3) / DistanceToLine(v2, v1, v3);
				float wv3 = DistanceToLine(vecUVPosition, v1, v2) / DistanceToLine(v3, v1, v2);

				vecNormal = vn1 * wv1 + vn2 * wv2 + vn3 * wv3;
			}

			if (ao_debug.GetInt() > 1)
				SMAKWindow()->AddDebugLine(vecUVPosition, vecUVPosition + vecNormal/2);

			size_t iTexel;
			if (!Texel(i, j, iTexel, false))
				continue;

			if (m_eAOMethod == AOMETHOD_RENDER)
			{
				// Render the scene from this location
				m_avecShadowValues[iTexel] += RenderSceneFromPosition(vecUVPosition, vecNormal, pFace);
			}
			else if (m_eAOMethod == AOMETHOD_RAYTRACE)
			{
				RaytraceSceneMultithreaded(pTracer, vecUVPosition, vecNormal, pMeshInstance, pFace, iTexel);
			}

			m_aiShadowReads[iTexel]++;
			m_bPixelMask[iTexel] = true;

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(++iRendered);

			if (m_bStopGenerating)
				break;
		}
		if (m_bStopGenerating)
			break;
	}
}

Vector CAOGenerator::RenderSceneFromPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pRenderFace)
{
	TUnimplemented();

	CRenderingContext c(SMAKRenderer());

	c.UseFrameBuffer(&m_oRenderFB);

	// Bring it away from its poly so that the camera never clips around behind it.
	// Adds .001 because of a bug where GL for some reason won't show the faces unless I do that.
	Vector vecEye = vecPosition + (vecDirection + Vector(.001f, .001f, .001f)) * 0.1f;

	c.SetViewport(Rect(m_iRPVX, m_iRPVY, m_iRPVW, m_iRPVH));

	c.SetProjection(Matrix4x4::ProjectPerspective(120.0f, 1, 0.01f, 100.0f));
	c.SetView(Matrix4x4::ConstructCameraView(vecEye, vecDirection, Vector(0, 1, 0)));

	c.ClearColor();
	c.ClearDepth();

	for (size_t i = 0; i < m_aiSceneMaterials.size(); i++)
	{
		TAssert(i < SMAKWindow()->GetMaterials().size());
		if (i >= SMAKWindow()->GetMaterials().size())
			break;

		if (!m_aiSceneMaterialVerts[i])
			continue;

		c.UseMaterial(SMAKWindow()->GetMaterials()[i]);

		c.BeginRenderVertexArray(m_aiSceneMaterials[i]);
		c.SetPositionBuffer((size_t)0, 5*sizeof(float));
		c.SetTexCoordBuffer((size_t)3*sizeof(float), 5*sizeof(float));
		c.EndRenderVertexArray(m_aiSceneMaterialVerts[i]);
	}

	c.Finish();

#ifdef AO_DEBUG
	DebugRenderSceneLookAtPosition(vecPosition, vecDirection, pRenderFace);
	glFinish();
#endif

	c.ReadPixels(m_iRPVX, m_iRPVY, m_iRPVW, m_iRPVH, m_pvecPixels);

	Vector vecShadowColor;
	float flTotal = 0;

	for (size_t p = 0; p < m_iRPVW*m_iRPVH*m_iPixelDepth; p++)
	{
		float flColumn = fmod((float)p / (float)m_iPixelDepth, (float)m_iRPVW);

		Vector vecUV(flColumn / m_iRPVW, (float)(p / m_iPixelDepth / m_iRPVW) / m_iRPVH, 0);
		Vector vecUVCenter(0.5f, 0.5f, 0);

		// Weight the pixel based on its distance to the center.
		// With the huge FOV that we work with, polygons to the
		// outside are huge on the screen.
		float flWeight = (0.5f-(vecUV - vecUVCenter).Length())*2.0f;
		if (flWeight <= 0.1)
			continue;

		// Pixels in the center of the screen are much, much more important.
		flWeight = SLerp(flWeight, 0.2f);

		Vector vecPixel(m_pvecPixels[p].x, m_pvecPixels[p].y, m_pvecPixels[p].z);

		vecShadowColor += vecPixel * flWeight;
		flTotal += flWeight;
	}

	vecShadowColor /= flTotal;

	return vecShadowColor;
}

void CAOGenerator::DebugRenderSceneLookAtPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pRenderFace)
{
#ifdef AO_DEBUG
#ifdef OPENGL2
	glDrawBuffer(GL_FRONT);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);

	Vector vecLookAt = vecPosition;
	Vector vecEye = vecPosition + pRenderFace->GetNormal()*10;
	vecEye.y += 2;
	vecEye.x += 2;

	glViewport(0, 100, 512, 512);

	// Set up some rendering stuff.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
			44.0,
			1,
			1,
			100.0
		);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(
		vecEye.x, vecEye.y, vecEye.z,
		vecLookAt.x, vecLookAt.y, vecLookAt.z,
		0.0, 1.0, 0.0);

	// It uses this color if the texture is missing.
	GLfloat flMaterialColor[] = {0.0, 0.0, 0.0, 1.0};

	glMaterialfv(GL_FRONT, GL_DIFFUSE, flMaterialColor);
	glColor4fv(flMaterialColor);

	glCallList(m_iSceneList);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_POLYGON);
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor4f(0, 1, 0, 0.5f);
		for (size_t p = 0; p < pRenderFace->GetNumVertices(); p++)
			glVertex3fv(pRenderFace->m_pScene->GetMesh(pRenderFace->m_iMesh)->GetVertex(pRenderFace->GetVertex(p)->v) + pRenderFace->GetNormal()*0.01f);
	glEnd();

	glDisable(GL_BLEND);

	Vector vecNormalEnd = vecPosition + vecDirection;
	glBindTexture(GL_TEXTURE_2D, 0);
	glLineWidth(2);
	glBegin(GL_LINES);
		glColor4f(1, 1, 1, 1);
		glVertex3f(vecPosition.x, vecPosition.y, vecPosition.z);
		glVertex3f(vecNormalEnd.x, vecNormalEnd.y, vecNormalEnd.z);
	glEnd();

	glFinish();

	glViewport(m_iRPVX, m_iRPVY, m_iRPVW, m_iRPVH);
#endif
#endif
}

void CAOGenerator::Bleed()
{
	bool* abPixelMask = (bool*)malloc(m_iWidth*m_iHeight*sizeof(bool));

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Bleeding edges", m_iBleed);

	for (size_t i = 0; i < m_iBleed; i++)
	{
		// This is for pixels that have been set this frame.
		memset(&abPixelMask[0], 0, m_iWidth*m_iHeight*sizeof(bool));

		for (size_t w = 0; w < m_iWidth; w++)
		{
			for (size_t h = 0; h < m_iHeight; h++)
			{
				Vector vecTotal(0,0,0);
				size_t iTotal = 0;
				size_t iTexel;

				// If the texel has the mask on then it already has a value so skip it.
				if (Texel(w, h, iTexel, true))
					continue;

				if (Texel(w-1, h-1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w-1, h, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w-1, h+1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w, h+1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w+1, h+1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w+1, h, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w+1, h-1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w, h-1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				Texel(w, h, iTexel, false);

				if (iTotal)
				{
					vecTotal /= (float)iTotal;
					m_avecShadowValues[iTexel] = vecTotal;
					abPixelMask[iTexel] = true;
				}
			}
		}

		for (size_t p = 0; p < m_iWidth*m_iHeight; p++)
			m_bPixelMask[p] |= abPixelMask[p];

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(i);

		if (m_bStopGenerating)
			break;
	}

	free(abPixelMask);
}

CTextureHandle CAOGenerator::GenerateTexture(bool bInMedias)
{
	Vector* avecShadowValues = m_avecShadowValues;

	if (bInMedias)
	{
		// Use this temporary buffer so we don't clobber the original.
		avecShadowValues = m_avecShadowGeneratedValues;

		if (m_eAOMethod == AOMETHOD_SHADOWMAP) // If bleeding, we have reads already so do it the old fasioned way so it actually shows the bleeds expanding.
		{
			if (m_bIsBleeding)
			{
				for (size_t i = 0; i < m_iWidth*m_iHeight; i++)
				{
					// Don't immediately return, just skip this loop. We have cleanup work to do.
					if (m_bStopGenerating)
						break;

					avecShadowValues[i] = m_avecShadowValues[i];
				}
			}
			else
			{
				CRenderingContext c(SMAKRenderer());
				c.UseFrameBuffer(&m_oAOFB);
				c.ReadPixels(0, 0, m_iWidth, m_iHeight, m_pvecPixels);

				size_t iBufferSize = m_iWidth*m_iHeight;
				for (size_t p = 0; p < iBufferSize; p++)
				{
					if (m_pvecPixels[p].w == 0.0f)
						avecShadowValues[p].x = 0;
					else
						avecShadowValues[p].x = m_pvecPixels[p].x/m_pvecPixels[p].w;

					avecShadowValues[p].y = avecShadowValues[p].z = avecShadowValues[p].x;
				}
			}
		}
		else
		{
			// Average out all of the reads.
			for (size_t i = 0; i < m_iWidth*m_iHeight; i++)
			{
				// Don't immediately return, just skip this loop. We have cleanup work to do.
				if (m_bStopGenerating)
					break;

				if (!m_aiShadowReads[i])
				{
					avecShadowValues[i] = Vector(0,0,0);
					continue;
				}

				avecShadowValues[i] = m_avecShadowValues[i] / (float)m_aiShadowReads[i];

				// When exporting to png sometimes a pure white value will suffer integer overflow.
				avecShadowValues[i] *= 0.99f;
			}
		}
	}

	return CTextureLibrary::AddTexture(avecShadowValues, m_iWidth, m_iHeight);
}

void CAOGenerator::SaveToFile(const tchar *pszFilename)
{
	if (!pszFilename)
		return;

	tvector<Color> aclrData;
	aclrData.resize(m_iWidth*m_iHeight);

	for (size_t i = 0; i < m_iWidth*m_iHeight; i++)
	{
		if (m_eAOMethod != AOMETHOD_SHADOWMAP)
			// When exporting to png sometimes a pure white value will suffer integer overflow.
			m_avecShadowValues[i] *= 0.99f;

		aclrData[i] = m_avecShadowValues[i];
	}

	CRenderer::WriteTextureToFile(aclrData.data(), m_iWidth, m_iHeight, pszFilename);
}

bool CAOGenerator::Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask)
{
	if (w < 0 || h < 0 || w >= m_iWidth || h >= m_iHeight)
		return false;

	TAssert(m_iHeight == m_iWidth); // This formula is actually wrong unless the thing is a perfect square
	iTexel = m_iHeight*(m_iHeight-h-1) + w;

	TAssert(iTexel >= 0 && iTexel < m_iWidth * m_iHeight);

	if (bUseMask && !m_bPixelMask[iTexel])
		return false;

	return true;
}

#ifdef _DEBUG
void DrawSplit(const raytrace::CKDNode* pNode)
{
	// No children, no split.
	if (!pNode->GetLeftChild())
		return;

	AABB oBox = pNode->GetBounds();
	size_t iSplitAxis = pNode->GetSplitAxis();
	float flSplitPos = pNode->GetSplitPos();

	// Construct four points that are the corners or a rectangle representing this portal split.
	Vector v0 = oBox.m_vecMins;
	v0[iSplitAxis] = flSplitPos;
	Vector v1 = v0;
	v1[(iSplitAxis+1)%3] = oBox.m_vecMaxs[(iSplitAxis+1)%3];
	Vector v2 = v0;
	v2[(iSplitAxis+1)%3] = oBox.m_vecMaxs[(iSplitAxis+1)%3];
	v2[(iSplitAxis+2)%3] = oBox.m_vecMaxs[(iSplitAxis+2)%3];
	Vector v3 = v0;
	v3[(iSplitAxis+2)%3] = oBox.m_vecMaxs[(iSplitAxis+2)%3];

	SMAKWindow()->AddDebugLine(v0, v1);
	SMAKWindow()->AddDebugLine(v1, v2);
	SMAKWindow()->AddDebugLine(v2, v3);
	SMAKWindow()->AddDebugLine(v3, v0);

	if (pNode->GetLeftChild())
		DrawSplit(pNode->GetLeftChild());
	if (pNode->GetRightChild())
		DrawSplit(pNode->GetRightChild());
}
#endif

void DrawTexture(size_t iTexture, float flScale, CRenderingContext& c)
{
	if (!c.GetActiveShader())
	{
		c.UseProgram("quad");
		c.SetUniform("iDiffuse", 0);
	}

	c.BindTexture(iTexture);
	c.ClearDepth();
	c.SetDepthTest(false);
	c.SetBlend(BLEND_ALPHA);
	c.SetBackCulling(false);

	c.SetView(Matrix4x4());
	c.SetProjection(Matrix4x4());
	c.ResetTransformations();

	c.BeginRenderTriFan();
		c.TexCoord(Vector2D(0, 1));
		c.Vertex(Vector(-flScale, -flScale, 0));

		c.TexCoord(Vector2D(0, 0));
		c.Vertex(Vector(-flScale, flScale, 0));

		c.TexCoord(Vector2D(1, 0));
		c.Vertex(Vector(flScale, flScale, 0));

		c.TexCoord(Vector2D(1, 1));
		c.Vertex(Vector(flScale, -flScale, 0));
	c.EndRender();

	c.Finish();
	Application()->SwapBuffers();
}
