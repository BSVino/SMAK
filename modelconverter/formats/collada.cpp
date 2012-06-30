/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef NO_COLLADA

#include "../modelconverter.h"

bool CModelConverter::ReadDAE(const tstring& sFilename)
{
	return false;
}

void CModelConverter::ReadDAESceneTree(FCDSceneNode* pNode, CConversionSceneNode* pScene)
{
}

void CModelConverter::SaveDAE(const tstring& sFilename)
{
}

#else

#include <FCollada.h>
#include <FCDocument/FCDocument.h>
#include <FCDocument/FCDocumentTools.h>
#include <FCDocument/FCDAsset.h>
#include <FCDocument/FCDLibrary.h>
#include <FCDocument/FCDSceneNode.h>
#include <FCDocument/FCDTransform.h>
#include <FCDocument/FCDEntityInstance.h>
#include <FCDocument/FCDGeometryInstance.h>
#include <FCDocument/FCDMaterialInstance.h>
#include <FCDocument/FCDMaterial.h>
#include <FCDocument/FCDEffect.h>
#include <FCDocument/FCDEffectProfile.h>
#include <FCDocument/FCDEffectStandard.h>
#include <FCDocument/FCDEffectParameter.h>
#include <FCDocument/FCDEffectParameterSampler.h>
#include <FCDocument/FCDEffectParameterSurface.h>
#include <FCDocument/FCDImage.h>
#include <FCDocument/FCDGeometry.h>
#include <FCDocument/FCDGeometryMesh.h>
#include <FCDocument/FCDGeometrySource.h>
#include <FCDocument/FCDGeometryPolygons.h>
#include <FCDocument/FCDGeometryPolygonsInput.h>

#include <strutils.h>

#include "../modelconverter.h"

#ifdef _WIN32
#define convert_to_fstring(x) fstring(convert_to_wstring(x).c_str())
#define convert_from_fstring(x) convert_from_wstring(std::wstring(x.c_str()))
#define fstring_literal(x) fstring(convert_to_wstring(x).c_str())
#else
#define convert_to_fstring(x) (x).c_str()
#define convert_from_fstring(x) (x).c_str()
#define fstring_literal(x) (x)
#endif

bool CModelConverter::ReadDAE(const tstring& sFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	FCollada::Initialize();

	FCDocument* pDoc = FCollada::NewTopDocument();

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Reading file", 0);

	if (FCollada::LoadDocumentFromFile(pDoc, convert_to_fstring(sFilename)))
	{
		size_t i;

		FCDocumentTools::StandardizeUpAxisAndLength(pDoc, FMVector3(0, 1, 0));

		FCDMaterialLibrary* pMatLib = pDoc->GetMaterialLibrary();
		size_t iEntities = pMatLib->GetEntityCount();

		if (m_pWorkListener)
			m_pWorkListener->SetAction("Reading materials", iEntities);

		for (i = 0; i < iEntities; ++i)
		{
			FCDMaterial* pColladaMaterial = pMatLib->GetEntity(i);
			FCDEffect* pEffect = pColladaMaterial->GetEffect();

			size_t iMaterial = m_pScene->AddMaterial(convert_from_fstring(pColladaMaterial->GetName()));
			CConversionMaterial* pMaterial = m_pScene->GetMaterial(iMaterial);

			if (pEffect->GetProfileCount() < 1)
				continue;

			FCDEffectProfile* pEffectProfile = pEffect->GetProfile(0);

			FUDaeProfileType::Type eProfileType = pEffectProfile->GetType();
			if (eProfileType == FUDaeProfileType::COMMON)
			{
				FCDEffectStandard* pStandardProfile = dynamic_cast<FCDEffectStandard*>(pEffectProfile);
				if (pStandardProfile)
				{
					pMaterial->m_vecAmbient = Vector((float*)pStandardProfile->GetAmbientColor());
					pMaterial->m_vecDiffuse = Vector((float*)pStandardProfile->GetDiffuseColor());
					pMaterial->m_vecSpecular = Vector((float*)pStandardProfile->GetSpecularColor());
					pMaterial->m_vecEmissive = Vector((float*)pStandardProfile->GetEmissionColor());
					pMaterial->m_flShininess = pStandardProfile->GetShininess();
				}
			}

			for (size_t j = 0; j < pEffectProfile->GetEffectParameterCount(); j++)
			{
				FCDEffectParameter* pEffectParameter = pEffectProfile->GetEffectParameter(j);

				FCDEffectParameter::Type eType = pEffectParameter->GetType();

				if (eType == FCDEffectParameter::SAMPLER)
				{
					FCDEffectParameterSampler* pSampler = dynamic_cast<FCDEffectParameterSampler*>(pEffectParameter);
					if (pSampler)
					{
						FCDEffectParameterSurface* pSurface = pSampler->GetSurface();
						if (pSurface)
						{
							if (pSurface->GetImageCount())
							{
								// Christ Collada why do you have to make things so damn complicated?
								FCDImage* pImage = pSurface->GetImage(0);

								pMaterial->m_sDiffuseTexture = convert_from_fstring(pImage->GetFilename());
							}
						}
					}
				}
			}

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(i+1);
		}

		FCDGeometryLibrary* pGeoLib = pDoc->GetGeometryLibrary();
		iEntities = pGeoLib->GetEntityCount();

		if (m_pWorkListener)
			m_pWorkListener->SetAction("Loading entities", iEntities);

		for (i = 0; i < iEntities; ++i)
		{
			FCDGeometry* pGeometry = pGeoLib->GetEntity(i);
			if (pGeometry->IsMesh())
			{
				size_t j;

				size_t iMesh = m_pScene->AddMesh(convert_from_fstring(pGeometry->GetName()));
				CConversionMesh* pMesh = m_pScene->GetMesh(iMesh);
				pMesh->AddBone(convert_from_fstring(pGeometry->GetName()));

				FCDGeometryMesh* pGeoMesh = pGeometry->GetMesh();
				FCDGeometrySource* pPositionSource = pGeoMesh->GetPositionSource();
				size_t iVertexCount = pPositionSource->GetValueCount();

				for (j = 0; j < iVertexCount; j++)
				{
					const float* pflValues = pPositionSource->GetValue(j);
					pMesh->AddVertex(pflValues[0], pflValues[1], pflValues[2]);
				}

				FCDGeometrySource* pNormalSource = pGeoMesh->FindSourceByType(FUDaeGeometryInput::NORMAL);
				if (pNormalSource)
				{
					iVertexCount = pNormalSource->GetValueCount();
					for (j = 0; j < iVertexCount; j++)
					{
						const float* pflValues = pNormalSource->GetValue(j);
						pMesh->AddNormal(pflValues[0], pflValues[1], pflValues[2]);
					}
				}

				FCDGeometrySource* pUVSource = pGeoMesh->FindSourceByType(FUDaeGeometryInput::TEXCOORD);
				if (pUVSource)
				{
					iVertexCount = pUVSource->GetValueCount();
					for (j = 0; j < iVertexCount; j++)
					{
						const float* pflValues = pUVSource->GetValue(j);
						pMesh->AddUV(pflValues[0], pflValues[1]);
					}
				}

				for (j = 0; j < pGeoMesh->GetPolygonsCount(); j++)
				{
					FCDGeometryPolygons* pPolygons = pGeoMesh->GetPolygons(j);
					FCDGeometryPolygonsInput* pPositionInput = pPolygons->FindInput(FUDaeGeometryInput::POSITION);
					FCDGeometryPolygonsInput* pNormalInput = pPolygons->FindInput(FUDaeGeometryInput::NORMAL);
					FCDGeometryPolygonsInput* pUVInput = pPolygons->FindInput(FUDaeGeometryInput::TEXCOORD);

					size_t iPositionCount = pPositionInput->GetIndexCount();
					uint32* pPositions = pPositionInput->GetIndices();

					size_t iNormalCount = 0;
					uint32* pNormals = NULL;

					if (pNormalInput)
					{
						iNormalCount = pNormalInput->GetIndexCount();
						pNormals = pNormalInput->GetIndices();
					}

					size_t iUVCount = 0;
					uint32* pUVs = NULL;

					if (pUVInput)
					{
						iUVCount = pUVInput->GetIndexCount();
						pUVs = pUVInput->GetIndices();
					}

					fm::stringT<fchar> sMaterial = pPolygons->GetMaterialSemantic();

					size_t iCurrentMaterial = pMesh->AddMaterialStub(convert_from_fstring(sMaterial));

					if (pPolygons->TestPolyType() == 3)
					{
						// All triangles!
						for (size_t k = 0; k < iPositionCount; k+=3)
						{
							size_t iFace = pMesh->AddFace(iCurrentMaterial);

							pMesh->AddVertexToFace(iFace, pPositions[k+0], pUVs?pUVs[k+0]:~0, pNormals?pNormals[k+0]:~0);
							pMesh->AddVertexToFace(iFace, pPositions[k+1], pUVs?pUVs[k+1]:~0, pNormals?pNormals[k+1]:~0);
							pMesh->AddVertexToFace(iFace, pPositions[k+2], pUVs?pUVs[k+2]:~0, pNormals?pNormals[k+2]:~0);
						}
					}
					else if (pPolygons->TestPolyType() == 4)
					{
						// All quads!
						for (size_t k = 0; k < iPositionCount; k+=4)
						{
							size_t iFace = pMesh->AddFace(iCurrentMaterial);

							pMesh->AddVertexToFace(iFace, pPositions[k+0], pUVs?pUVs[k+0]:~0, pNormals?pNormals[k+0]:~0);
							pMesh->AddVertexToFace(iFace, pPositions[k+1], pUVs?pUVs[k+1]:~0, pNormals?pNormals[k+1]:~0);
							pMesh->AddVertexToFace(iFace, pPositions[k+2], pUVs?pUVs[k+2]:~0, pNormals?pNormals[k+2]:~0);
							pMesh->AddVertexToFace(iFace, pPositions[k+3], pUVs?pUVs[k+3]:~0, pNormals?pNormals[k+3]:~0);
						}
					}
					else
					{
						size_t iFaces = pPolygons->GetFaceCount();
						for (size_t k = 0; k < iFaces; k++)
						{
							size_t iFace = pMesh->AddFace(iCurrentMaterial);
							size_t o = pPolygons->GetFaceVertexOffset(k);
							size_t iFaceVertexCount = pPolygons->GetFaceVertexCount(k);
							for (size_t v = 0; v < iFaceVertexCount; v++)
								pMesh->AddVertexToFace(iFace, pPositions[o+v], pUVs?pUVs[o+v]:~0, pNormals?pNormals[o+v]:~0);
						}
					}
				}
			}

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(i+1);
		}

		FCDVisualSceneNodeLibrary* pVisualScenes = pDoc->GetVisualSceneLibrary();
		iEntities = pVisualScenes->GetEntityCount();
		for (i = 0; i < iEntities; ++i)
		{
			FCDSceneNode* pNode = pVisualScenes->GetEntity(i);

			size_t iScene = m_pScene->AddScene(convert_from_fstring(pNode->GetName()));
			ReadDAESceneTree(pNode, m_pScene->GetScene(iScene));
		}
	}
	else
	{
		printf("Oops! Some kind of error happened!\n");
		return false;
	}

	m_pScene->SetWorkListener(m_pWorkListener);

	m_pScene->CalculateExtends();

	for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
	{
		m_pScene->GetMesh(i)->CalculateEdgeData();

		if (m_pScene->GetMesh(i)->GetNumNormals() == 0)
			m_pScene->GetMesh(i)->CalculateVertexNormals();

		m_pScene->GetMesh(i)->CalculateVertexTangents();
	}

	pDoc->Release();

	FCollada::Release();

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();

	return true;
}

void CModelConverter::ReadDAESceneTree(FCDSceneNode* pNode, CConversionSceneNode* pScene)
{
	size_t iTransforms = pNode->GetTransformCount();

	Matrix4x4 mTransformations;

	for (size_t t = 0; t < iTransforms; t++)
	{
		FCDTransform* pTransform = pNode->GetTransform(t);
		FMMatrix44 m = pTransform->ToMatrix();
		mTransformations *= Matrix4x4(m);
	}

	pScene->SetTransformations(mTransformations);

	size_t iInstances = pNode->GetInstanceCount();

	for (size_t i = 0; i < iInstances; i++)
	{
		FCDEntityInstance* pInstance = pNode->GetInstance(i);

		switch (pInstance->GetType())
		{
		case FCDEntityInstance::GEOMETRY:
		{
			FCDGeometryInstance* pGeometryInstance = dynamic_cast<FCDGeometryInstance*>(pInstance);
			FCDEntity* pEntity = pGeometryInstance->GetEntity();
			size_t iMesh = pScene->m_pScene->FindMesh(convert_from_fstring(pEntity->GetName()));
			size_t iMeshInstance = pScene->AddMeshInstance(iMesh);

			size_t iMaterialInstances = pGeometryInstance->GetMaterialInstanceCount();
			for (size_t m = 0; m < iMaterialInstances; m++)
			{
				FCDMaterialInstance* pMaterialInstance = pGeometryInstance->GetMaterialInstance(m);
				FCDMaterial* pMaterial = pMaterialInstance->GetMaterial();
				tstring sMaterial = pMaterial?convert_from_fstring(pMaterialInstance->GetMaterial()->GetName()):"";
				tstring sMaterialStub = convert_from_fstring(pMaterialInstance->GetSemantic());

				size_t iMaterial = pScene->m_pScene->FindMaterial(sMaterial);
				size_t iMaterialStub = pScene->m_pScene->GetMesh(iMesh)->FindMaterialStub(sMaterialStub);

				pScene->GetMeshInstance(iMeshInstance)->AddMappedMaterial(iMaterialStub, iMaterial);
			}
		}
		}
	}

	size_t iChildren = pNode->GetChildrenCount();
	for (size_t j = 0; j < iChildren; j++)
	{
		FCDSceneNode* pChildNode = pNode->GetChild(j);
		size_t iNode = pScene->AddChild(convert_from_fstring(pChildNode->GetName()));
		ReadDAESceneTree(pChildNode, pScene->GetChild(iNode));
	}
}

void CModelConverter::SaveDAE(const tstring& sFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	FCollada::Initialize();

	FCDocument* pDoc = FCollada::NewTopDocument();

	FCDocumentTools::StandardizeUpAxisAndLength(pDoc, FMVector3(0, 1, 0));

	FCDAsset* pAsset = pDoc->GetAsset();
	FCDAssetContributor* pContributor = pAsset->AddContributor();
	pContributor->SetAuthoringTool(fstring_literal("Created by SMAK using FCollada"));

	FCDMaterialLibrary* pMatLib = pDoc->GetMaterialLibrary();

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Saving materials", m_pScene->GetNumMaterials());

	for (size_t iMaterial = 0; iMaterial < m_pScene->GetNumMaterials(); iMaterial++)
	{
		CConversionMaterial* pConversionMaterial = m_pScene->GetMaterial(iMaterial);

		FCDMaterial* pColladaMaterial = pMatLib->AddEntity();
		pColladaMaterial->SetName(convert_to_fstring(pConversionMaterial->GetName()));
		FCDEffect* pEffect = pMatLib->GetDocument()->GetEffectLibrary()->AddEntity();
		pColladaMaterial->SetEffect(pEffect);
		FCDEffectProfile* pEffectProfile = pEffect->AddProfile(FUDaeProfileType::COMMON);

		pEffect->SetName(convert_to_fstring(pConversionMaterial->GetName()));

		FCDEffectStandard* pStandardProfile = dynamic_cast<FCDEffectStandard*>(pEffectProfile);
		if (pStandardProfile)
		{
			pStandardProfile->SetLightingType(FCDEffectStandard::PHONG);
			pStandardProfile->SetAmbientColor(FMVector4(FMVector3((float*)pConversionMaterial->m_vecAmbient), 1));
			pStandardProfile->SetDiffuseColor(FMVector4(FMVector3((float*)pConversionMaterial->m_vecDiffuse), 1));
			pStandardProfile->SetSpecularColor(FMVector4(FMVector3((float*)pConversionMaterial->m_vecSpecular), 1));
			pStandardProfile->SetEmissionColor(FMVector4(FMVector3((float*)pConversionMaterial->m_vecEmissive), 1));
			pStandardProfile->SetShininess(pConversionMaterial->m_flShininess);
		}

		if (pConversionMaterial->GetDiffuseTexture().length())
		{
			FCDEffectParameter* pEffectParameterSampler = pEffectProfile->AddEffectParameter(FCDEffectParameter::SAMPLER);
			FCDEffectParameter* pEffectParameterSurface = pEffectProfile->AddEffectParameter(FCDEffectParameter::SURFACE);
			FCDEffectParameterSampler* pSampler = dynamic_cast<FCDEffectParameterSampler*>(pEffectParameterSampler);
			FCDEffectParameterSurface* pSurface = dynamic_cast<FCDEffectParameterSurface*>(pEffectParameterSurface);
			FCDImage* pSurfaceImage = pMatLib->GetDocument()->GetImageLibrary()->AddEntity();

			pSurfaceImage->SetFilename(convert_to_fstring(pConversionMaterial->GetDiffuseTexture()));

			pSurface->SetInitMethod(new FCDEffectParameterSurfaceInitFrom());
			pSurface->AddImage(pSurfaceImage);
			pSurface->SetReference((pConversionMaterial->GetName() + "-surface").c_str());

			pSampler->SetSurface(pSurface);
		}

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(iMaterial);
	}

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Saving geometry", m_pScene->GetNumMeshes());

	FCDGeometryLibrary* pGeoLib = pDoc->GetGeometryLibrary();

	for (size_t i = 0; i < m_pScene->GetNumMeshes(); ++i)
	{
		CConversionMesh* pConversionMesh = m_pScene->GetMesh(i);

		FCDGeometry* pGeometry = pGeoLib->AddEntity();
		pGeometry->SetName(convert_to_fstring(pConversionMesh->GetName()));
		pGeometry->CreateMesh();
		FCDGeometryMesh* pMesh = pGeometry->GetMesh();

		FCDGeometrySource* pPositionSource = pMesh->AddSource(FUDaeGeometryInput::POSITION);
		pPositionSource->SetName(convert_to_fstring(pConversionMesh->GetName() + "-position"));
		pPositionSource->SetStride(3);
		pPositionSource->SetValueCount(pConversionMesh->GetNumVertices());
		for (size_t j = 0; j < pConversionMesh->GetNumVertices(); j++)
			pPositionSource->SetValue(j, pConversionMesh->GetVertex(j));

		pMesh->AddVertexSource(pPositionSource);

		FCDGeometrySource* pNormalSource = pMesh->AddSource(FUDaeGeometryInput::NORMAL);
		pNormalSource->SetName(convert_to_fstring(pConversionMesh->GetName() + "-normal"));
		pNormalSource->SetStride(3);
		pNormalSource->SetValueCount(pConversionMesh->GetNumNormals());
		for (size_t j = 0; j < pConversionMesh->GetNumNormals(); j++)
			pNormalSource->SetValue(j, pConversionMesh->GetNormal(j));

		FCDGeometrySource* pUVSource = NULL;
		if (pConversionMesh->GetNumUVs())
		{
			pUVSource = pMesh->AddSource(FUDaeGeometryInput::TEXCOORD);
			pUVSource->SetName(convert_to_fstring(pConversionMesh->GetName() + "-texcoord"));
			pUVSource->SetStride(2);
			pUVSource->SetValueCount(pConversionMesh->GetNumUVs());
			for (size_t j = 0; j < pConversionMesh->GetNumUVs(); j++)
				pUVSource->SetValue(j, pConversionMesh->GetUV(j));
		}

		for (size_t iMaterials = 0; iMaterials < pConversionMesh->GetNumMaterialStubs(); iMaterials++)
		{
			CConversionMaterialStub* pStub = pConversionMesh->GetMaterialStub(iMaterials);

			FCDGeometryPolygons* pPolygons = pMesh->AddPolygons();
			pPolygons->SetMaterialSemantic(convert_to_fstring(pStub->GetName()));
			pPolygons->AddInput(pPositionSource, 0);
			pPolygons->AddInput(pNormalSource, 1);
			if (pConversionMesh->GetNumUVs())
				pPolygons->AddInput(pUVSource, 2);

			FCDGeometryPolygonsInput* pPositionInput = pPolygons->FindInput(pPositionSource);
			FCDGeometryPolygonsInput* pNormalInput = pPolygons->FindInput(pNormalSource);
			FCDGeometryPolygonsInput* pUVInput = pPolygons->FindInput(pUVSource);

			for (size_t iFace = 0; iFace < pConversionMesh->GetNumFaces(); iFace++)
			{
				CConversionFace* pFace = pConversionMesh->GetFace(iFace);

				if (pFace->m != iMaterials)
					continue;

				pPolygons->AddFaceVertexCount(pFace->GetNumVertices());
				for (size_t iVertex = 0; iVertex < pFace->GetNumVertices(); iVertex++)
				{
					pPositionInput->AddIndex(pFace->GetVertex(iVertex)->v);
					pNormalInput->AddIndex(pFace->GetVertex(iVertex)->vn);
					if (pConversionMesh->GetNumUVs())
						pUVInput->AddIndex(pFace->GetVertex(iVertex)->vu);
				}
			}
		}

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(i);
	}

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Saving scenes", m_pScene->GetNumScenes());

	FCDVisualSceneNodeLibrary* pVisualScenes = pDoc->GetVisualSceneLibrary();
	for (size_t i = 0; i < m_pScene->GetNumScenes(); ++i)
	{
		FCDSceneNode* pNode = pVisualScenes->AddEntity();

		SaveDAEScene(pNode, m_pScene->GetScene(i));

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(i);
	}

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Writing to disk...", 0);

	FCollada::SaveDocument(pDoc, convert_to_fstring(sFilename));

	pDoc->Release();

	FCollada::Release();

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}

void CModelConverter::SaveDAEScene(class FCDSceneNode* pNode, CConversionSceneNode* pScene)
{
	pNode->SetName(convert_to_fstring(pScene->GetName()));

	FCDTMatrix* pTransform = (FCDTMatrix*)pNode->AddTransform(FCDTransform::MATRIX);
	pTransform->SetTransform(FMMatrix44(pScene->m_mTransformations));

	for (size_t i = 0; i < pScene->GetNumMeshInstances(); i++)
	{
		CConversionMeshInstance* pMeshInstance = pScene->GetMeshInstance(i);

		FCDEntityInstance* pInstance = pNode->AddInstance(FCDEntity::GEOMETRY);

		FCDGeometryInstance* pGeometryInstance = dynamic_cast<FCDGeometryInstance*>(pInstance);

		FCDGeometryLibrary* pGeoLib = pNode->GetDocument()->GetGeometryLibrary();
		pGeometryInstance->SetEntity(pGeoLib->GetEntity(pMeshInstance->m_iMesh));	// Relies on both libraries having the same indexes.

		FCDMaterialLibrary* pMatLib = pNode->GetDocument()->GetMaterialLibrary();

		tmap<size_t, CConversionMaterialMap>::iterator j;
		for (j = pMeshInstance->m_aiMaterialsMap.begin(); j != pMeshInstance->m_aiMaterialsMap.end(); j++)
		{
			CConversionMaterialMap* pMap = &j->second;
			pGeometryInstance->AddMaterialInstance(pMatLib->GetEntity(pMap->m_iMaterial), convert_to_fstring(pMeshInstance->GetMesh()->GetMaterialStub(pMap->m_iStub)->GetName()));
		}
	}

	size_t iChildren = pScene->GetNumChildren();
	for (size_t i = 0; i < iChildren; ++i)
		SaveDAEScene(pNode->AddChildNode(), pScene->GetChild(i));
}

#endif
