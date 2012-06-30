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

#include "smakwindow.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <maths.h>

#include <tinker_platform.h>
#include <files.h>

#include <modelconverter/modelconverter.h>
#include <glgui/glgui.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>
#include <glgui/filedialog.h>
#include <tinker/renderer/renderer.h>
#include <datamanager/data.h>
#include <textures/materiallibrary.h>
#include <renderer/shaders.h>
#include <tinker/profiler.h>

#include "smak_renderer.h"
#include "scenetree.h"
#include "models/models.h"

using namespace glgui;

//#define RAYTRACE_DEBUG
#ifdef RAYTRACE_DEBUG
#include <raytracer/raytracer.h>
#endif

CSMAKWindow* CSMAKWindow::s_pSMAKWindow = NULL;

CSMAKWindow::CSMAKWindow(int argc, char** argv)
	: CApplication(argc, argv)
{
	s_pSMAKWindow = this;

	m_bLoadingFile = false;

	m_aiObjects.clear();
	m_iObjectsCreated = 0;

	m_flCameraDistance = 100;

	m_bCameraRotating = false;
	m_bCameraDollying = false;
	m_bCameraPanning = false;
	m_bLightRotating = false;

	m_iMouseStartX = 0;
	m_iMouseStartY = 0;

	m_flCameraYaw = -135;
	m_flCameraPitch = -25;

	m_flCameraUVZoom = 1;
	m_flCameraUVX = 0;
	m_flCameraUVY = 0;

	m_flLightYaw = -40;
	m_flLightPitch = -45;

	int iScreenWidth;
	int iScreenHeight;

	GetScreenSize(iScreenWidth, iScreenHeight);

	m_iWindowWidth = iScreenWidth*2/3;
	m_iWindowHeight = iScreenHeight*2/3;

	m_aDebugLines.set_capacity(2);
}

void CSMAKWindow::OpenWindow()
{
	SetMultisampling(true);
	BaseClass::OpenWindow(m_iWindowWidth, m_iWindowHeight, false, true);

	InitUI();

	SetRenderMode(false);
	SetDisplayWireframe(false);
	SetDisplayUVWireframe(true);
	SetDisplayLight(true);
	SetDisplayTexture(true);
	SetDisplayAO(false);

	CSceneTreePanel::Get()->UpdateTree();

	RootPanel()->Layout();
}

CSMAKWindow::~CSMAKWindow()
{
}

CRenderer* CSMAKWindow::CreateRenderer()
{
	return new CSMAKRenderer();
}

void CSMAKWindow::Run()
{
	while (IsOpen())
	{
		CProfiler::BeginFrame();
		Render();
		CProfiler::Render();
		SwapBuffers();
	}
}

void CSMAKWindow::DestroyAll()
{
	m_Scene.DestroyAll();

	m_sFileLoaded.clear();

	m_aiObjects.clear();
	m_iObjectsCreated = 0;
	m_ahMaterials.clear();

	CModelLibrary::ResetReferenceCounts();
	CModelLibrary::ClearUnreferenced();

	CRootPanel::Get()->UpdateScene();
	CSceneTreePanel::Get()->UpdateTree();

	CRootPanel::Get()->Layout();
}

void CSMAKWindow::ReadFile(const tchar* pszFile)
{
	if (!pszFile)
		return;

	if (m_bLoadingFile)
		return;

	CSceneTreePanel::Get()->CloseMaterialEditor();

	// Save it in here in case m_szFileLoaded was passed into ReadFile, in which case it would be destroyed by DestroyAll.
	tstring sFile = pszFile;

	DestroyAll();

	ReadFileIntoScene(sFile.c_str());
}

void CSMAKWindow::ReadFileIntoScene(const tchar* pszFile)
{
	if (!pszFile)
		return;

	if (m_bLoadingFile)
		return;

	m_bLoadingFile = true;

	CModelConverter c(&m_Scene);

	c.SetWorkListener(this);

	if (!c.ReadModel(pszFile))
	{
		m_bLoadingFile = false;
		return;
	}

	m_sFileLoaded = pszFile;

	glgui::CFileDialog::SetDefaultDirectory(GetDirectory(m_sFileLoaded));

	BeginProgress();
	SetAction("Loading into video hardware", 0);
	LoadIntoGL();
	EndProgress();

	m_flCameraDistance = m_Scene.m_oExtends.Size().Length() * 1.5f;

	CRootPanel::Get()->UpdateScene();
	CSceneTreePanel::Get()->UpdateTree();

	m_bLoadingFile = false;
}

void CSMAKWindow::ReloadFromFile()
{
	ReadFile(m_sFileLoaded.c_str());
}

void CSMAKWindow::LoadIntoGL()
{
	LoadModelsIntoGL();
	LoadMaterialsIntoGL();

	ClearDebugLines();
}

void CSMAKWindow::LoadModelsIntoGL()
{
	for (size_t i = 0; i < m_Scene.GetNumMeshes(); i++)
		CModelLibrary::AddModel(&m_Scene, i);
}

void CSMAKWindow::LoadMaterialsIntoGL()
{
	m_ahMaterials.clear();
	m_ahMaterials.resize(m_Scene.GetNumMaterials());

	for (size_t i = 0; i < m_Scene.GetNumMaterials(); i++)
	{
		CData oMaterialData;
		CData* pShader = oMaterialData.AddChild("Shader", "model");
		if (m_Scene.GetMaterial(i))
		{
			CConversionMaterial* pMaterial = m_Scene.GetMaterial(i);
			pShader->AddChild("_TextureDirectory", GetDirectory(m_sFileLoaded));
			pShader->AddChild("DiffuseTexture", pMaterial->GetDiffuseTexture());
			pShader->AddChild("Diffuse", sprintf("%f %f %f", pMaterial->m_vecDiffuse.x, pMaterial->m_vecDiffuse.y, pMaterial->m_vecDiffuse.z));
			pShader->AddChild("Ambient", sprintf("%f %f %f", pMaterial->m_vecAmbient.x, pMaterial->m_vecAmbient.y, pMaterial->m_vecAmbient.z));
			pShader->AddChild("Emissive", sprintf("%f %f %f", pMaterial->m_vecEmissive.x, pMaterial->m_vecEmissive.y, pMaterial->m_vecEmissive.z));
			pShader->AddChild("Specular", sprintf("%f %f %f", pMaterial->m_vecSpecular.x, pMaterial->m_vecSpecular.y, pMaterial->m_vecSpecular.z));
			pShader->AddChild("Shininess", sprintf("%f", pMaterial->m_flShininess));
			m_ahMaterials[i] = CMaterialLibrary::AddMaterial(&oMaterialData);
		}

		//TAssert(m_aiMaterials[i]);
		if (!m_ahMaterials[i].IsValid())
			TError(tstring("Couldn't create material \"") + m_Scene.GetMaterial(i)->GetName() + "\"\n");
	}
}

void CSMAKWindow::SaveFile(const tchar* pszFile)
{
	if (!pszFile)
		return;

	CModelConverter c(&m_Scene);

	c.SetWorkListener(this);

	c.SaveModel(pszFile);
}

void CSMAKWindow::Render()
{
	TPROF("CSMAKWindow::Render");

	GetSMAKRenderer()->Render();

	CRootPanel::Get()->Think(GetTime());
	CRootPanel::Get()->Paint(0, 0, (float)m_iWindowWidth, (float)m_iWindowHeight);
}

void CSMAKWindow::WindowResize(int w, int h)
{
	if (!IsOpen())
		return;

	BaseClass::WindowResize(w, h);
}

size_t CSMAKWindow::GetNextObjectId()
{
	return (m_iObjectsCreated++)+1;
}

void CSMAKWindow::SetRenderMode(bool bUV)
{
	m_hRender3D->SetState(false, false);
	m_hRenderUV->SetState(false, false);

	if (bUV)
		m_hRenderUV->SetState(true, false);
	else
		m_hRender3D->SetState(true, false);

	m_bRenderUV = bUV;

	m_hWireframe->SetVisible(!m_bRenderUV);
	m_hUVWireframe->SetVisible(m_bRenderUV);

	Layout();
}

void CSMAKWindow::SetDisplayWireframe(bool bWire)
{
	m_bDisplayWireframe = bWire;
	m_hWireframe->SetState(bWire, false);
}

void CSMAKWindow::SetDisplayUVWireframe(bool bWire)
{
	m_bDisplayUV = bWire;
	m_hUVWireframe->SetState(bWire, false);
}

void CSMAKWindow::SetDisplayLight(bool bLight)
{
	m_bDisplayLight = bLight;
	m_hLight->SetState(bLight, false);
}

void CSMAKWindow::SetDisplayTexture(bool bTexture)
{
	m_bDisplayTexture = bTexture;
	m_hTexture->SetState(bTexture, false);
}

void CSMAKWindow::SetDisplayNormal(bool bNormal)
{
	m_bDisplayNormal = bNormal;
	m_hNormal->SetState(bNormal, false);
}

void CSMAKWindow::SetDisplayAO(bool bAO)
{
	m_bDisplayAO = bAO;
	m_hAO->SetState(bAO, false);
}

void CSMAKWindow::SetDisplayColorAO(bool bColorAO)
{
	m_bDisplayColorAO = bColorAO;
	m_hColorAO->SetState(bColorAO, false);
}

// Returns a normalized vector from the specified image file with dimensions iNormalWidth by iNormalHeight
// at x, y if the image were resampled to iSampleWidth, iSampleHeight
Vector NormalSample(const tvector<Vector>& vecNormal, size_t iNormalWidth, size_t iNormalHeight, size_t x, size_t y, size_t iSampleWidth, size_t iSampleHeight)
{
	TAssert(vecNormal.size() == iNormalWidth*iNormalHeight);

	float flX = RemapVal((float)x, 0, (float)iSampleWidth, 0, (float)iNormalWidth);
	float flY = RemapVal((float)y, 0, (float)iSampleHeight, 0, (float)iNormalHeight);

	int iX = (int)flX;	// Truncation desired!
	int iY = (int)flY;	// Truncation desired!
	int iX2 = iX + 1;
	int iY2 = iY + 1;

	iX = Clamp<int>(iX, 0, iNormalWidth-1);
	iX2 = Clamp<int>(iX2, 0, iNormalHeight-1);
	iY = Clamp<int>(iY, 0, iNormalWidth-1);
	iY2 = Clamp<int>(iY2, 0, iNormalHeight-1);

	float flXLerp = flX - iX;
	float flYLerp = flY - iY;

	float flXLerpInv = 1-flXLerp;
	float flYLerpInv = 1-flYLerp;

	Vector xy = (vecNormal[iNormalHeight*(iNormalHeight-iY-1) + iX] - Vector(0.5f, 0.5f, 0.5f)) * 2;
	Vector xY = (vecNormal[iNormalHeight*(iNormalHeight-iY2-1) + iX] - Vector(0.5f, 0.5f, 0.5f)) * 2;
	Vector Xy = (vecNormal[iNormalHeight*(iNormalHeight-iY-1) + iX2] - Vector(0.5f, 0.5f, 0.5f)) * 2;
	Vector XY = (vecNormal[iNormalHeight*(iNormalHeight-iY2-1) + iX2] - Vector(0.5f, 0.5f, 0.5f)) * 2;

	return ((xy*flYLerpInv + xY*flYLerp)*flXLerpInv + (Xy*flYLerpInv + XY*flYLerp)*flXLerp).Normalized();
}

void CSMAKWindow::SaveNormal(size_t iMaterial, const tstring& sFilename)
{
	CMaterialHandle hMaterial = GetMaterials()[iMaterial];

	CShader* pShader = CShaderLibrary::GetShader(hMaterial->m_sShader);
	size_t iNormal = pShader->FindTextureByUniform("iNormal");
	size_t iNormal2 = pShader->FindTextureByUniform("iNormal2");

	if (iNormal == ~0 && iNormal2 == ~0)
		return;

	// If we only have one, spit it out as is.
	if (iNormal == ~0 || iNormal2 == ~0)
	{
		if (iNormal != ~0)
			CRenderer::WriteTextureToFile(hMaterial->m_ahTextures[iNormal]->m_iGLID, sFilename);
		else
			CRenderer::WriteTextureToFile(hMaterial->m_ahTextures[iNormal2]->m_iGLID, sFilename);

		return;
	}

	CTextureHandle hNormal = hMaterial->m_ahTextures[iNormal];
	CTextureHandle hNormal2 = hMaterial->m_ahTextures[iNormal2];

	tvector<Vector> avecNormals;
	tvector<Vector> avecNormals2;

	avecNormals.resize(hNormal->m_iWidth*hNormal->m_iHeight);
	avecNormals2.resize(hNormal2->m_iWidth*hNormal2->m_iHeight);

	CRenderer::ReadTextureFromGL(hNormal, avecNormals.data());
	CRenderer::ReadTextureFromGL(hNormal2, avecNormals2.data());

	size_t iTotalWidth = hNormal->m_iWidth > hNormal2->m_iWidth ? hNormal->m_iWidth : hNormal2->m_iWidth;
	size_t iTotalHeight = hNormal->m_iHeight > hNormal2->m_iHeight ? hNormal->m_iHeight : hNormal2->m_iHeight;

	tvector<Color> aclrMergedNormalValues;
	aclrMergedNormalValues.resize(iTotalWidth*iTotalHeight);

	for (size_t i = 0; i < iTotalWidth; i++)
	{
		for (size_t j = 0; j < iTotalHeight; j++)
		{
			size_t iTexel = iTotalHeight*(iTotalHeight-j-1) + i;

			Vector vecNormal = NormalSample(avecNormals, hNormal->m_iWidth, hNormal->m_iHeight, i, j, iTotalWidth, iTotalHeight);
			Vector vecNormal2 = NormalSample(avecNormals2, hNormal2->m_iWidth, hNormal2->m_iHeight, i, j, iTotalWidth, iTotalHeight);

			Vector vecBitangent = vecNormal.Cross(Vector(1, 0, 0)).Normalized();
			Vector vecTangent = vecBitangent.Cross(vecNormal).Normalized();

			Matrix4x4 mTBN;
			mTBN.SetForwardVector(vecTangent);
			mTBN.SetUpVector(vecBitangent);
			mTBN.SetRightVector(vecNormal);

			Vector vecMergedNormal = (mTBN * vecNormal2)/1.99f + Vector(0.5f, 0.5f, 0.5f);
			aclrMergedNormalValues[iTexel] = vecMergedNormal;
		}
	}

	CRenderer::WriteTextureToFile(aclrMergedNormalValues.data(), iTotalWidth, iTotalHeight, sFilename);
}

CSMAKRenderer* CSMAKWindow::GetSMAKRenderer()
{
	return static_cast<CSMAKRenderer*>(m_pRenderer);
};

void CSMAKWindow::ClearDebugLines()
{
	m_aDebugLines.clear();
}

void CSMAKWindow::AddDebugLine(Vector vecStart, Vector vecEnd, Color clrLine)
{
	DebugLine& l = m_aDebugLines.push_back();
	l.vecStart = vecStart;
	l.vecEnd = vecEnd;
	l.clrLine = clrLine;
}
