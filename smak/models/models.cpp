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

#include "models.h"

#include <files.h>

#include <toys/toy_util.h>
#include <toys/toy.h>
#include <renderer/renderer.h>
#include <textures/materiallibrary.h>
#include <tinker/application.h>

CModelLibrary* CModelLibrary::s_pModelLibrary = NULL;
static CModelLibrary g_ModelLibrary = CModelLibrary();

CModelLibrary::CModelLibrary()
{
	m_iModelsLoaded = 0;
	s_pModelLibrary = this;
}

CModelLibrary::~CModelLibrary()
{
	for (size_t i = 0; i < m_apModels.size(); i++)
	{
		if (m_apModels[i])
		{
			m_apModels[i]->m_iReferences = 0;
			delete m_apModels[i];
		}
	}

	s_pModelLibrary = NULL;
}

void CModelLibrary::AddModel(class CConversionScene* pScene, size_t iMesh)
{
	while (Get()->m_apModels.size() <= iMesh)
		Get()->m_apModels.push_back(nullptr);

	CModel* pModel = Get()->m_apModels[iMesh];
	if (pModel)
	{
		pModel->m_iReferences++;
		return;
	}

	pModel = new CModel();

	Get()->m_apModels[iMesh] = pModel;

	if (!pModel->Load(pScene, iMesh))
	{
		Get()->m_apModels[iMesh] = nullptr;
		delete pModel;
		return;
	}

	pModel->m_iReferences++;

	Get()->m_iModelsLoaded++;
}

CModel* CModelLibrary::GetModel(size_t i)
{
	if (i >= Get()->m_apModels.size())
		return NULL;

	return Get()->m_apModels[i];
}

void CModelLibrary::ReleaseModel(size_t i)
{
	CModel* pModel = GetModel(i);

	if (!pModel)
		return;

	TAssert(pModel->m_iReferences > 0);
	if (pModel->m_iReferences)
		pModel->m_iReferences--;
}

void CModelLibrary::UnloadModel(size_t i)
{
	CModel* pModel = GetModel(i);

	if (!pModel)
		return;

	pModel->m_iReferences = 0;

	delete pModel;
	Get()->m_apModels[i] = nullptr;
	Get()->m_iModelsLoaded--;
}

void CModelLibrary::ResetReferenceCounts()
{
	for (size_t i = 0; i < Get()->m_apModels.size(); i++)
	{
		if (!Get()->m_apModels[i])
			continue;

		Get()->m_apModels[i]->m_iReferences = 0;
	}
}

void CModelLibrary::ClearUnreferenced()
{
	for (size_t i = 0; i < Get()->m_apModels.size(); i++)
	{
		CModel* pModel = Get()->m_apModels[i];
		if (!pModel)
			continue;

		if (!pModel->m_iReferences)
		{
			delete pModel;
			Get()->m_apModels[i] = nullptr;
			Get()->m_iModelsLoaded--;
		}
	}
}

CModel::CModel()
{
	m_iReferences = 0;
}

CModel::~CModel()
{
	TAssert(m_iReferences == 0);

	Unload();
}

void CModel::Unload()
{
	for (size_t i = 0; i < m_aiVertexBuffers.size(); i++)
	{
		if (m_aiVertexBufferSizes[i] == 0)
			continue;

		UnloadBufferFromGL(m_aiVertexBuffers[i]);
	}
}

void CModel::UnloadBufferFromGL(size_t iBuffer)
{
	CRenderer::UnloadVertexDataFromGL(iBuffer);
}
