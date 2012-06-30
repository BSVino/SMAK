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

#include "smak_renderer.h"

#include <tinker/application.h>
#include <renderer/shaders.h>
#include <renderer/renderingcontext.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <textures/materiallibrary.h>

#include "smakwindow.h"
#include "models/models.h"
#include "scenetree.h"
#include "combogenerator.h"

CSMAKRenderer::CSMAKRenderer()
	: CRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
	m_bDrawBackground = true;

	m_vecLightPositionUV = Vector(0.5f, 0.5f, 1.0f);
}

void CSMAKRenderer::Initialize()
{
	BaseClass::Initialize();

	m_hLightBeam = CMaterialLibrary::AddMaterial("materials/lightbeam.mat");
	m_hLightHalo = CMaterialLibrary::AddMaterial("materials/lighthalo.mat");

	m_hWireframe = CMaterialLibrary::AddMaterial("materials/wireframe.mat");
	m_hSmooth = CMaterialLibrary::AddMaterial("materials/smooth.mat");
	m_hUV = CMaterialLibrary::AddMaterial("materials/uv.mat");
	m_hLight = CMaterialLibrary::AddMaterial("materials/light.mat");
	m_hTexture = CMaterialLibrary::AddMaterial("materials/texture.mat");
	m_hNormal = CMaterialLibrary::AddMaterial("materials/normal.mat");
	m_hAO = CMaterialLibrary::AddMaterial("materials/ao.mat");
	m_hCAO = CMaterialLibrary::AddMaterial("materials/aocolor.mat");
	m_hArrow = CMaterialLibrary::AddMaterial("materials/arrow.mat");
	m_hEdit = CMaterialLibrary::AddMaterial("materials/pencil.mat");
	m_hVisibility = CMaterialLibrary::AddMaterial("materials/eye.mat");
}

void CSMAKRenderer::Render()
{
	if (SMAKWindow()->IsRenderingUV())
	{
		m_bRenderOrthographic = true;

		m_flCameraOrthoHeight = SMAKWindow()->GetCameraUVZoom();
		m_vecCameraPosition.x = SMAKWindow()->GetCameraUVX();
		m_vecCameraPosition.y = SMAKWindow()->GetCameraUVY();

		m_vecCameraDirection = Vector(0, 0, -1);
		m_vecCameraUp = Vector(0, 1, 0);
	}
	else
	{
		m_bRenderOrthographic = false;

		float flSceneSize = SMAKWindow()->GetScene()->m_oExtends.Size().Length()/2;
		if (flSceneSize < 150)
			flSceneSize = 150;

		m_flCameraFOV = 44;
		m_flCameraNear = 1;
		m_flCameraFar = SMAKWindow()->GetCameraDistance() + flSceneSize;

		Vector vecSceneCenter = SMAKWindow()->GetScene()->m_oExtends.Center();

		Vector vecCameraVector = AngleVector(EAngle(SMAKWindow()->GetCameraPitch(), SMAKWindow()->GetCameraYaw(), 0)) * -SMAKWindow()->GetCameraDistance() + vecSceneCenter;

		m_vecCameraPosition = vecCameraVector;
		m_vecCameraDirection = (vecSceneCenter - vecCameraVector).Normalized();
		m_vecCameraUp = Vector(0, 1, 0);
	}

	PreRender();

	{
		CRenderingContext c(this);
		ModifyContext(&c);
		SetupFrame(&c);
		StartRendering(&c);

		if (SMAKWindow()->IsRenderingUV())
			RenderUV();
		else
			Render3D();

		FinishRendering(&c);
		FinishFrame(&c);
	}

	PostRender();
}

void CSMAKRenderer::DrawBackground(CRenderingContext* r)
{
	CRenderingContext c;
	c.SetWinding(true);
	c.SetDepthTest(false);
	c.SetBackCulling(false);

	c.UseFrameBuffer(&m_oSceneBuffer);

	c.UseProgram("background");

	c.BeginRenderVertexArray();

	c.SetTexCoordBuffer(&m_vecFullscreenTexCoords[0][0]);
	c.SetPositionBuffer(&m_vecFullscreenVertices[0][0]);

	c.EndRenderVertexArray(6);
}

void CSMAKRenderer::Render3D()
{
	// Reposition the light source.
	Vector vecLightDirection = AngleVector(EAngle(SMAKWindow()->GetLightPitch(), SMAKWindow()->GetLightYaw(), 0));

	m_vecLightPosition = vecLightDirection * -SMAKWindow()->GetCameraDistance()/2;

	RenderGround();

	RenderObjects();

	// Render light source on top of objects, since it doesn't use the depth buffer.
	RenderLightSource();

	// Don't render while generating since it's multithreaded and that does nasty stuff.
	if (SMAKWindow()->GetDebugLines().size() && !CComboGeneratorPanel::Get()->IsGenerating())
	{
		CRenderingContext c(this, true);

		c.UseProgram("grid");
		for (size_t i = 0; i < SMAKWindow()->GetDebugLines().size(); i++)
		{
			c.BeginRenderLines();
				c.Color(SMAKWindow()->GetDebugLines()[i].clrLine);
				c.Vertex(SMAKWindow()->GetDebugLines()[i].vecStart);
				c.Vertex(SMAKWindow()->GetDebugLines()[i].vecEnd);
			c.EndRender();
		}
		c.BeginRenderPoints(2);
			c.Color(Vector(0.6f, 0.6f, 0.6f));
			for (size_t i = 0; i < SMAKWindow()->GetDebugLines().size(); i++)
			{
				c.Vertex(SMAKWindow()->GetDebugLines()[i].vecStart);
				c.Vertex(SMAKWindow()->GetDebugLines()[i].vecEnd);
			}
		c.EndRender();
	}

#ifdef RAYTRACE_DEBUG
	static raytrace::CRaytracer* pTracer = NULL;
	Vector vecStart(0.55841917f, 0.28102291f, 2.4405572f);
	Vector vecDirection(-0.093336411f, 0.99130136f, -0.092789143f);
	if (!pTracer && GetScene()->GetNumScenes() && !m_bLoadingFile)
	{
		pTracer = new raytrace::CRaytracer(GetScene());
		pTracer->AddMeshInstance(GetScene()->GetScene(0)->GetChild(2)->GetMeshInstance(0));
		pTracer->BuildTree();
		AddDebugLine(vecStart, vecStart+vecDirection*10);
	}
	if (pTracer)
		pTracer->Raytrace(Ray(vecStart, vecDirection));
#endif
}

void CSMAKRenderer::RenderGround()
{
	CRenderingContext c(this, true);

	c.UseProgram("grid");
	c.Translate(SMAKWindow()->GetScene()->m_oExtends.Center());

	int i;

	Color clrBorderLineBright(0.7f, 0.7f, 0.7f);
	Color clrBorderLineDarker(0.6f, 0.6f, 0.6f);
	Color clrInsideLineBright(0.5f, 0.5f, 0.5f);
	Color clrInsideLineDarker(0.4f, 0.4f, 0.4f);

	for (i = 0; i < 20; i++)
	{
		Vector vecStartX(-100, 0, -100);
		Vector vecEndX(-100, 0, 100);
		Vector vecStartZ(-100, 0, -100);
		Vector vecEndZ(100, 0, -100);

		for (int j = 0; j <= 20; j++)
		{
			c.BeginRenderLines();

				if (j == 0 || j == 20 || j == 10)
					c.Color(clrBorderLineBright);
				else
					c.Color(clrInsideLineBright);

				c.Vertex(vecStartX);

				if (j == 0 || j == 20 || j == 10)
					c.Color(clrBorderLineDarker);
				else
					c.Color(clrInsideLineDarker);

				if (j == 10)
					c.Vertex(Vector(0, 0, 0));
				else
					c.Vertex(vecEndX);

			c.EndRender();

			if (j == 10)
			{
				c.BeginRenderLines();
					c.Color(Color(0.9f, 0.2f, 0.2f));
					c.Vertex(Vector(0, 0, 0));
					c.Vertex(Vector(100, 0, 0));
				c.EndRender();
			}

			c.BeginRenderLines();

				if (j == 0 || j == 20 || j == 10)
					c.Color(clrBorderLineBright);
				else
					c.Color(clrInsideLineBright);

				c.Vertex(vecStartZ);

				if (j == 0 || j == 20 || j == 10)
					c.Color(clrBorderLineDarker);
				else
					c.Color(clrInsideLineDarker);

				if (j == 10)
					c.Vertex(Vector(0, 0, 0));
				else
					c.Vertex(vecEndZ);

			c.EndRender();

			if (j == 10)
			{
				c.BeginRenderLines();
					c.Color(Color(0.2f, 0.2f, 0.7f));
					c.Vertex(Vector(0, 0, 0));
					c.Vertex(Vector(0, 0, 100));
				c.EndRender();
			}

			vecStartX.x += 10;
			vecEndX.x += 10;
			vecStartZ.z += 10;
			vecEndZ.z += 10;
		}
	}
}

void CSMAKRenderer::RenderObjects()
{
	CRenderingContext c(this, true);

	for (size_t i = 0; i < SMAKWindow()->GetScene()->GetNumScenes(); i++)
		RenderSceneNode(SMAKWindow()->GetScene()->GetScene(i));
}

void CSMAKRenderer::RenderSceneNode(CConversionSceneNode* pNode)
{
	if (!pNode)
		return;

	if (!pNode->IsVisible())
		return;

	CRenderingContext c(this, true);
	c.Transform(pNode->m_mTransformations);

	for (size_t i = 0; i < pNode->GetNumChildren(); i++)
		RenderSceneNode(pNode->GetChild(i));

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
		RenderMeshInstance(pNode->GetMeshInstance(m));
}

CVar smak_shownormals("smak_shownormals", "0");

void CSMAKRenderer::RenderMeshInstance(CConversionMeshInstance* pMeshInstance)
{
	if (!pMeshInstance->IsVisible())
		return;

	CModel* pModel = CModelLibrary::GetModel(pMeshInstance->m_iMesh);

	if (!pModel)
		return;

	CRenderingContext c(this, true);

	if (SMAKWindow()->IsRenderingWireframe())
	{
		c.UseProgram("wireframe");

		c.SetUniform("flAlpha", 1.0f);
		c.SetUniform("bDiffuse", false);

		if (SMAKWindow()->IsRenderingLight())
		{
			c.SetUniform("bLight", true);
			c.SetUniform("vecLightDirection", -m_vecLightPosition.Normalized());
			c.SetUniform("clrLightDiffuse", Vector(1, 1, 1));
			c.SetUniform("clrLightAmbient", Vector(0.2f, 0.2f, 0.2f));
			c.SetUniform("clrLightSpecular", Vector(1, 1, 1));
		}
		else
			c.SetUniform("bLight", false);

		c.BeginRenderVertexArray(pModel->m_iVertexWireframeBuffer);
		c.SetPositionBuffer(pModel->WireframePositionOffset(), pModel->WireframeStride());
		c.SetNormalsBuffer(pModel->NormalsOffset(), pModel->WireframeStride());
		c.EndRenderVertexArray(pModel->m_iVertexWireframeBufferSize, true);
	}
	else
	{
		for (size_t i = 0; i < pModel->m_asMaterialStubs.size(); i++)
		{
			auto pMaterialMap = pMeshInstance->GetMappedMaterial(i);

			if (!pMaterialMap)
				continue;

			if (!pMaterialMap->IsVisible())
				continue;

			size_t iMaterial = pMaterialMap->m_iMaterial;

			if (!SMAKWindow()->GetScene()->GetMaterial(iMaterial)->IsVisible())
				continue;

			if (!pModel->m_aiVertexBufferSizes[i])
				continue;

			CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[iMaterial];
			c.UseMaterial(hMaterial);

			if (!c.GetActiveShader())
			{
				c.UseProgram("model");

				c.SetUniform("flAlpha", 1.0f);
				c.SetUniform("vecColor", Color(255, 255, 255));
				c.SetUniform("bDiffuse", false);
			}

			size_t iDiffuse = c.GetActiveShader()->FindTextureByUniform("iDiffuse");
			size_t iNormal = c.GetActiveShader()->FindTextureByUniform("iNormal");
			size_t iNormal2 = c.GetActiveShader()->FindTextureByUniform("iNormal2");

			bool bTexture = SMAKWindow()->IsRenderingTexture() && hMaterial.IsValid() && iDiffuse < hMaterial->m_ahTextures.size() && hMaterial->m_ahTextures[iDiffuse].IsValid();
			bool bNormal = SMAKWindow()->IsRenderingNormal() && hMaterial.IsValid() && iNormal < hMaterial->m_ahTextures.size() && hMaterial->m_ahTextures[iNormal].IsValid();
			bool bNormal2 = SMAKWindow()->IsRenderingNormal() && hMaterial.IsValid() && iNormal2 < hMaterial->m_ahTextures.size() && hMaterial->m_ahTextures[iNormal2].IsValid();

			if (!bTexture)
				c.SetUniform("bDiffuse", false);

			c.SetUniform("bShadeBottoms", bNormal||bNormal2);
			c.SetUniform("bNormal", bNormal);
			c.SetUniform("bNormal2", bNormal2);

			if (!SMAKWindow()->IsRenderingAO())
				c.SetUniform("bAO", false);

			c.SetUniform("flRimLight", 0.05f);

			if (SMAKWindow()->IsRenderingLight())
			{
				c.SetUniform("bLight", true);
				c.SetUniform("vecLightDirection", -m_vecLightPosition.Normalized());
				c.SetUniform("clrLightDiffuse", Vector(1, 1, 1));
				c.SetUniform("clrLightAmbient", Vector(0.2f, 0.2f, 0.2f));
				c.SetUniform("clrLightSpecular", Vector(1, 1, 1));
			}
			else
			{
				c.SetUniform("bLight", false);
				c.SetUniform("bShadeBottoms", true);
			}

			if (!bTexture && !SMAKWindow()->IsRenderingLight())
				c.SetUniform("flRimLight", 0.15f);

			c.BeginRenderVertexArray(pModel->m_aiVertexBuffers[i]);
			c.SetPositionBuffer(pModel->PositionOffset(), pModel->Stride());
			c.SetNormalsBuffer(pModel->NormalsOffset(), pModel->Stride());
			c.SetTangentsBuffer(pModel->TangentsOffset(), pModel->Stride());
			c.SetBitangentsBuffer(pModel->BitangentsOffset(), pModel->Stride());
			c.SetTexCoordBuffer(pModel->TexCoordOffset(), pModel->Stride());
			c.EndRenderVertexArray(pModel->m_aiVertexBufferSizes[i]);
		}
	}

	if (smak_shownormals.GetBool())
	{
		CRenderingContext c(this, true);
		c.UseProgram("grid");

		CConversionMesh* pMesh = pMeshInstance->GetMesh();

		for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
		{
			CConversionFace* pFace = pMesh->GetFace(j);
			for (size_t k = 0; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex = pFace->GetVertex(k);

				Vector vecVertex = pMesh->GetVertex(pVertex->v);
				Vector vecNormal = pMesh->GetNormal(pVertex->vn);
				Vector vecTangent = pMesh->GetTangent(pVertex->vt);
				Vector vecBitangent = pMesh->GetBitangent(pVertex->vb);
				//vecNormal = Vector(pVertex->m_mInverseTBN.GetColumn(2));
				//vecTangent = Vector(pVertex->m_mInverseTBN.GetColumn(0));
				//vecBitangent = Vector(pVertex->m_mInverseTBN.GetColumn(1));

				c.BeginRenderLines();
					c.Color(Color(0.2f, 0.2f, 0.8f));
					c.Normal(vecNormal);
					c.Vertex(vecVertex);
					c.Vertex(vecVertex + vecNormal);
				c.EndRender();

				if (smak_shownormals.GetInt() > 1)
				{
					c.BeginRenderLines();
						c.Color(Color(0.8f, 0.2f, 0.2f));
						c.Normal(vecTangent);
						c.Vertex(vecVertex);
						c.Vertex(vecVertex + vecTangent);
					c.EndRender();

					c.BeginRenderLines();
						c.Color(Color(0.2f, 0.8f, 0.2f));
						c.Normal(vecBitangent);
						c.Vertex(vecVertex);
						c.Vertex(vecVertex + vecBitangent);
					c.EndRender();
				}
			}
		}
	}
}

void CSMAKRenderer::RenderUV()
{
	CRenderingContext c(this, true);

	c.SetDepthTest(false);
	c.SetBackCulling(false);

	CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[0];
	size_t iLastSelectedMaterial = SceneTree()->GetLastSelectedMaterial();
	if (iLastSelectedMaterial < SMAKWindow()->GetMaterials().size())
		hMaterial = SMAKWindow()->GetMaterials()[iLastSelectedMaterial];

	if (hMaterial.IsValid())
	{
		c.UseMaterial(hMaterial);

		size_t iDiffuse = c.GetActiveShader()->FindTextureByUniform("iDiffuse");
		size_t iNormal = c.GetActiveShader()->FindTextureByUniform("iNormal");
		size_t iNormal2 = c.GetActiveShader()->FindTextureByUniform("iNormal2");

		bool bTexture = SMAKWindow()->IsRenderingTexture() && hMaterial.IsValid() && iDiffuse < hMaterial->m_ahTextures.size() && hMaterial->m_ahTextures[iDiffuse].IsValid();
		bool bNormal = SMAKWindow()->IsRenderingNormal() && hMaterial.IsValid() && iNormal < hMaterial->m_ahTextures.size() && hMaterial->m_ahTextures[iNormal].IsValid();
		bool bNormal2 = SMAKWindow()->IsRenderingNormal() && hMaterial.IsValid() && iNormal2 < hMaterial->m_ahTextures.size() && hMaterial->m_ahTextures[iNormal2].IsValid();

		c.SetUniform("bDiffuse", bTexture);
		c.SetUniform("bShadeBottoms", bNormal||bNormal2);
		c.SetUniform("bNormal", bNormal);
		c.SetUniform("bNormal2", bNormal2);

		if (SMAKWindow()->IsRenderingLight())
		{
			c.SetUniform("bLight", true);
			c.SetUniform("vecLightDirection", -m_vecLightPositionUV.Normalized());
			c.SetUniform("clrLightDiffuse", Vector(1, 1, 1));
			c.SetUniform("clrLightAmbient", Vector(0.2f, 0.2f, 0.2f));
			c.SetUniform("clrLightSpecular", Vector(1, 1, 1));
		}
		else
			c.SetUniform("bLight", false);

		Vector vecUV;

		c.BeginRenderTriFan();

			vecUV = Vector(0.0f, 1.0f, 0.0f);
			c.TexCoord(vecUV);
			//glVertexAttrib3fv(iTangent, Vector(0.8165f, 0.4082f, 0.4082f));
			//glVertexAttrib3fv(iBitangent, Vector(0.0f, 0.7071f, -0.7071f));
			c.Normal(Vector(-0.5574f, 0.5574f, 0.5574f));
			c.Vertex(Vector(-0.5f, 0.5f, 0));

			vecUV = Vector(1.0f, 1.0f, 0.0f);
			c.TexCoord(vecUV);
			//glVertexAttrib3fv(iTangent, Vector(0.8165f, -0.4082f, -0.4082f));
			//glVertexAttrib3fv(iBitangent, Vector(0.0f, 0.7071f, 0.7071f));
			c.Normal(Vector(0.5574f, 0.5574f, 0.5574f));
			c.Vertex(Vector(0.5f, 0.5f, 0));

			vecUV = Vector(1.0f, 0.0f, 0.0f);
			c.TexCoord(vecUV);
			//glVertexAttrib3fv(iTangent, Vector(0.8165f, 0.4082f, -0.4082f));
			//glVertexAttrib3fv(iBitangent, Vector(0.0f, 0.7071f, 0.7071f));
			c.Normal(Vector(0.5574f, -0.5574f, 0.5574f));
			c.Vertex(Vector(0.5f, -0.5f, 0));

			vecUV = Vector(0.0f, 0.0f, 0.0f);
			c.TexCoord(vecUV);
			//glVertexAttrib3fv(iTangent, Vector(0.8165f, -0.4082f, 0.4082f));
			//glVertexAttrib3fv(iBitangent, Vector(0.0f, 0.7071f, 0.7071f));
			c.Normal(Vector(-0.5574f, -0.5574f, 0.5574f));
			c.Vertex(Vector(-0.5f, -0.5f, 0));

		c.EndRender();
	}

	if (SMAKWindow()->IsRenderingUVWireframe())
	{
		c.Translate(Vector(-0.5f, -0.5f, 0));

		c.UseProgram("wireframe");

		c.SetUniform("flAlpha", 1.0f);
		c.SetUniform("bDiffuse", false);
		c.SetUniform("bLight", false);

		Vector vecOffset(-0.5f, -0.5f, 0);

		for (size_t i = 0; i < SMAKWindow()->GetScene()->GetNumMeshes(); i++)
		{
			CConversionMesh* pMesh = SMAKWindow()->GetScene()->GetMesh(i);
			if (!pMesh->IsVisible())
				continue;

			CModel* pModel = CModelLibrary::GetModel(i);

			if (!pModel)
				continue;

			if (!pModel->m_iVertexUVBufferSize)
				continue;

			c.BeginRenderVertexArray(pModel->m_iVertexUVBuffer);
			c.SetPositionBuffer(pModel->UVPositionOffset(), pModel->UVStride());
			c.EndRenderVertexArray(pModel->m_iVertexUVBufferSize, true);
		}
	}
}

void CSMAKRenderer::RenderLightSource()
{
	if (!SMAKWindow()->IsRenderingLight())
		return;

	float flScale = SMAKWindow()->GetCameraDistance()/60;

	CRenderingContext c(this, true);

	c.Translate(SMAKWindow()->GetScene()->m_oExtends.Center());
	c.Translate(m_vecLightPosition);
	c.Rotate(-SMAKWindow()->GetLightYaw(), Vector(0, 1, 0));
	c.Rotate(SMAKWindow()->GetLightPitch(), Vector(0, 0, 1));
	c.Scale(flScale, flScale, flScale);

	if (m_hLightBeam.IsValid() && m_hLightHalo.IsValid())
	{
		Vector vecCameraDirection = AngleVector(EAngle(SMAKWindow()->GetCameraPitch(), SMAKWindow()->GetCameraYaw(), 0));

		c.SetDepthTest(false);

		c.UseMaterial(m_hLightHalo);

		float flDot = vecCameraDirection.Dot(m_vecLightPosition.Normalized());

		if (flDot > 0.2)
		{
			float flScale = RemapVal(flDot, 0.2f, 1.0f, 0.0f, 0.5f);
			c.SetUniform("flAlpha", flScale);

			flScale *= 10;

			c.BeginRenderTriFan();
				c.TexCoord(Vector(0, 1, 0));
				c.Vertex(Vector(0, flScale, flScale));
				c.TexCoord(Vector(1, 1, 0));
				c.Vertex(Vector(0, -flScale, flScale));
				c.TexCoord(Vector(1, 0, 0));
				c.Vertex(Vector(0, -flScale, -flScale));
				c.TexCoord(Vector(0, 0, 0));
				c.Vertex(Vector(0, flScale, -flScale));
			c.EndRender();
		}

		c.SetBackCulling(false);
		c.UseMaterial(m_hLightBeam);

		Vector vecLightRight, vecLightUp;
		Matrix4x4 mLight(EAngle(SMAKWindow()->GetLightPitch(), SMAKWindow()->GetLightYaw(), 0), Vector());
		vecLightRight = mLight.GetRightVector();
		vecLightUp = mLight.GetUpVector();

		flDot = vecCameraDirection.Dot(vecLightRight) * 0.5f;
		c.SetUniform("flAlpha", (float)fabs(flDot));

		c.BeginRenderTriFan();
			c.TexCoord(Vector(1, 1, 0));
			c.Vertex(Vector(25, -5, 0));
			c.TexCoord(Vector(0, 1, 0));
			c.Vertex(Vector(25, 5, 0));
			c.TexCoord(Vector(0, 0, 0));
			c.Vertex(Vector(0, 5, 0));
			c.TexCoord(Vector(1, 0, 0));
			c.Vertex(Vector(0, -5, 0));
		c.EndRender();

		flDot = vecCameraDirection.Dot(vecLightUp) * 0.5f;
		c.SetUniform("flAlpha", (float)fabs(flDot));

		c.BeginRenderTriFan();
			c.TexCoord(Vector(1, 1, 0));
			c.Vertex(Vector(25, 0, -5));
			c.TexCoord(Vector(0, 1, 0));
			c.Vertex(Vector(25, 0, 5));
			c.TexCoord(Vector(0, 0, 0));
			c.Vertex(Vector(0, 0, 5));
			c.TexCoord(Vector(1, 0, 0));
			c.Vertex(Vector(0, 0, -5));
		c.EndRender();
	}
}

void CSMAKRenderer::MoveUVLight(float flX, float flY)
{
	m_vecLightPositionUV.x += flX;
	m_vecLightPositionUV.y += flY;

	if (m_vecLightPositionUV.x < -3.0f)
		m_vecLightPositionUV.x = -3.0f;
	if (m_vecLightPositionUV.x > 3.0f)
		m_vecLightPositionUV.x = 3.0f;
	if (m_vecLightPositionUV.y < -3.0f)
		m_vecLightPositionUV.y = -3.0f;
	if (m_vecLightPositionUV.y > 3.0f)
		m_vecLightPositionUV.y = 3.0f;
}

CSMAKRenderer* SMAKRenderer()
{
	return static_cast<CSMAKRenderer*>(SMAKWindow()->GetRenderer());
}
