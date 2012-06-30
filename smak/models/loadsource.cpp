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

#include <stdio.h>

#include <tinker_platform.h>
#include <files.h>

#include <modelconverter/modelconverter.h>
#include <renderer/renderer.h>
#include <textures/materiallibrary.h>
#include <tinker/application.h>
#include <datamanager/data.h>

tvector<tvector<float> >	g_aaflData;
tvector<float>				g_aflWireframeData;
tvector<float>				g_aflUVData;
AABB						g_aabbBounds;

void AddVertex(size_t iMaterial, const Vector& v, const Vector& vn, const Vector& vt, const Vector& vb, const Vector2D& vu)
{
	g_aaflData[iMaterial].push_back(v.x);
	g_aaflData[iMaterial].push_back(v.y);
	g_aaflData[iMaterial].push_back(v.z);
	g_aaflData[iMaterial].push_back(vn.x);
	g_aaflData[iMaterial].push_back(vn.y);
	g_aaflData[iMaterial].push_back(vn.z);
	g_aaflData[iMaterial].push_back(vt.x);
	g_aaflData[iMaterial].push_back(vt.y);
	g_aaflData[iMaterial].push_back(vt.z);
	g_aaflData[iMaterial].push_back(vb.x);
	g_aaflData[iMaterial].push_back(vb.y);
	g_aaflData[iMaterial].push_back(vb.z);
	g_aaflData[iMaterial].push_back(vu.x);
	g_aaflData[iMaterial].push_back(vu.y);

	for (int i = 0; i < 3; i++)
	{
		if (v[i] < g_aabbBounds.m_vecMins[i])
			g_aabbBounds.m_vecMins[i] = v[i];
		if (v[i] > g_aabbBounds.m_vecMaxs[i])
			g_aabbBounds.m_vecMaxs[i] = v[i];
	}
}

void AddWireframe(const Vector& v1, const Vector& vn1, const Vector& v2, const Vector& vn2)
{
	g_aflWireframeData.push_back(v1.x);
	g_aflWireframeData.push_back(v1.y);
	g_aflWireframeData.push_back(v1.z);
	g_aflWireframeData.push_back(vn1.x);
	g_aflWireframeData.push_back(vn1.y);
	g_aflWireframeData.push_back(vn1.z);
	g_aflWireframeData.push_back(v2.x);
	g_aflWireframeData.push_back(v2.y);
	g_aflWireframeData.push_back(v2.z);
	g_aflWireframeData.push_back(vn2.x);
	g_aflWireframeData.push_back(vn2.y);
	g_aflWireframeData.push_back(vn2.z);
}

void AddUV(const Vector& v1, const Vector& v2)
{
	g_aflUVData.push_back(v1.x);
	g_aflUVData.push_back(v1.y);
	g_aflUVData.push_back(v2.x);
	g_aflUVData.push_back(v2.y);
}

void LoadMesh(CConversionScene* pScene, size_t iMesh)
{
	TAssert(iMesh < pScene->GetNumMeshes());
	if (iMesh >= pScene->GetNumMeshes())
		return;

	// Reserve space for n+1, the last one represents the default material.
	g_aaflData.resize(pScene->GetNumMaterials()+1);

	CConversionMesh* pMesh = pScene->GetMesh(iMesh);

	for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
	{
		size_t k;
		CConversionFace* pFace = pMesh->GetFace(j);

		size_t iMaterial = pFace->m;
		if (iMaterial == ~0)
			iMaterial = pScene->GetNumMaterials();

		CConversionVertex* pVertex0 = pFace->GetVertex(0);
		CConversionVertex* pVertex1 = pFace->GetVertex(1);
		AddWireframe(pMesh->GetVertex(pVertex0->v), pMesh->GetNormal(pVertex0->vn), pMesh->GetVertex(pVertex1->v), pMesh->GetNormal(pVertex1->vn));
		AddUV(pMesh->GetUV(pVertex0->vu), pMesh->GetUV(pVertex1->vu));

		for (k = 2; k < pFace->GetNumVertices(); k++)
		{
			CConversionVertex* pVertex1 = pFace->GetVertex(k-1);
			CConversionVertex* pVertex2 = pFace->GetVertex(k);

			AddVertex(iMaterial, pMesh->GetVertex(pVertex0->v), pMesh->GetNormal(pVertex0->vn), pMesh->GetTangent(pVertex0->vt), pMesh->GetBitangent(pVertex0->vb), pMesh->GetUV(pVertex0->vu));
			AddVertex(iMaterial, pMesh->GetVertex(pVertex1->v), pMesh->GetNormal(pVertex1->vn), pMesh->GetTangent(pVertex1->vt), pMesh->GetBitangent(pVertex1->vb), pMesh->GetUV(pVertex1->vu));
			AddVertex(iMaterial, pMesh->GetVertex(pVertex2->v), pMesh->GetNormal(pVertex2->vn), pMesh->GetTangent(pVertex2->vt), pMesh->GetBitangent(pVertex2->vb), pMesh->GetUV(pVertex2->vu));

			AddWireframe(pMesh->GetVertex(pVertex1->v), pMesh->GetNormal(pVertex1->vn), pMesh->GetVertex(pVertex2->v), pMesh->GetNormal(pVertex2->vn));
			AddUV(pMesh->GetUV(pVertex1->vu), pMesh->GetUV(pVertex2->vu));
		}

		CConversionVertex* pLastVertex = pFace->GetVertex(pFace->GetNumVertices()-1);
		AddWireframe(pMesh->GetVertex(pLastVertex->v), pMesh->GetNormal(pLastVertex->vn), pMesh->GetVertex(pVertex0->v), pMesh->GetNormal(pVertex0->vn));
		AddUV(pMesh->GetUV(pLastVertex->vu), pMesh->GetUV(pVertex0->vu));
	}
}

bool CModel::Load(class CConversionScene* pScene, size_t iMesh)
{
	g_aaflData.clear();
	g_aflWireframeData.clear();
	g_aflUVData.clear();
	g_aabbBounds = AABB(Vector(999, 999, 999), Vector(-999, -999, -999));

	LoadMesh(pScene, iMesh);

	m_iVertexWireframeBuffer = CRenderer::LoadVertexDataIntoGL(g_aflWireframeData.size()*4, &g_aflWireframeData[0]);
	m_iVertexWireframeBufferSize = g_aflWireframeData.size()/FloatsPerWireframeVertex();

	m_iVertexUVBuffer = CRenderer::LoadVertexDataIntoGL(g_aflUVData.size()*4, &g_aflUVData[0]);
	m_iVertexUVBufferSize = g_aflUVData.size()/FloatsPerUVVertex();

	size_t iMaterials = g_aaflData.size();

	for (size_t i = 0; i < iMaterials; i++)
	{
		if (g_aaflData[i].size() == 0)
			continue;

		m_aiVertexBuffers.push_back(CRenderer::LoadVertexDataIntoGL(g_aaflData[i].size()*4, &g_aaflData[i][0]));
		m_aiVertexBufferSizes.push_back(g_aaflData[i].size()/FloatsPerVertex());

		if (m_aiVertexBuffers.size() <= pScene->GetMesh(iMesh)->GetNumMaterialStubs())
			m_asMaterialStubs.push_back(pScene->GetMesh(iMesh)->GetMaterialStub(m_asMaterialStubs.size())->GetName());
		else
			m_asMaterialStubs.push_back("");
	}

	m_aabbBoundingBox = g_aabbBounds;

	return true;
}
