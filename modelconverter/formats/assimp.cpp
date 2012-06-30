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

#include "../modelconverter.h"

#ifndef WITH_ASSIMP

bool CModelConverter::ReadAssImp(const tstring& sFilename)
{
	return false;
}

void CModelConverter::ReadAssImpSceneTree(FCDSceneNode* pNode, CConversionSceneNode* pScene)
{
}

#else

#include <assimp.h>
#include <aiPostProcess.h>
#include <aiScene.h>

bool CModelConverter::ReadAssImp(const tstring& sFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Reading file", 0);

	const struct aiScene* pScene = aiImportFile(sFilename.c_str(), aiProcess_CalcTangentSpace|aiProcess_GenSmoothNormals|aiProcess_GenUVCoords|aiProcess_TransformUVCoords);

	if (!pScene)
	{
		if (m_pWorkListener)
			m_pWorkListener->EndProgress();

		return false;
	}

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Reading materials", pScene->mNumMaterials);

	for (unsigned int i = 0; i < pScene->mNumMaterials; i++)
	{
		aiMaterial* pAIMaterial = pScene->mMaterials[i];

		aiString sName;
		if (pAIMaterial->Get(AI_MATKEY_NAME, sName) != AI_SUCCESS)
			continue;

		size_t iMaterial = m_pScene->AddMaterial(sName.data);
		CConversionMaterial* pMaterial = m_pScene->GetMaterial(iMaterial);

		aiColor3D clr;
		if (pAIMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, clr) == AI_SUCCESS)
			pMaterial->m_vecDiffuse = &clr[0];

		if (pAIMaterial->Get(AI_MATKEY_COLOR_AMBIENT, clr) == AI_SUCCESS)
			pMaterial->m_vecAmbient = &clr[0];

		if (pAIMaterial->Get(AI_MATKEY_COLOR_SPECULAR, clr) == AI_SUCCESS)
			pMaterial->m_vecSpecular = &clr[0];

		if (pAIMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, clr) == AI_SUCCESS)
			pMaterial->m_vecEmissive = &clr[0];

		float f;
		if (pAIMaterial->Get(AI_MATKEY_SHININESS, f) == AI_SUCCESS)
			pMaterial->m_flShininess = f;

		aiString sDiffuse;
		if (pAIMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), sDiffuse) == AI_SUCCESS)
			pMaterial->m_sDiffuseTexture = sDiffuse.data;

		aiString sNormal;
		if (pAIMaterial->Get(AI_MATKEY_TEXTURE_NORMALS(0), sNormal) == AI_SUCCESS)
			pMaterial->m_sNormalTexture = sNormal.data;

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(i+1);
	}

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Reading meshes", pScene->mNumMeshes);

	for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
	{
		aiMesh* pAIMesh = pScene->mMeshes[i];

		size_t iMesh = m_pScene->AddMesh(pAIMesh->mName.data);
		CConversionMesh* pMesh = m_pScene->GetMesh(iMesh);
		pMesh->AddBone(pAIMesh->mName.data);

		// Below code copies verts directly from source to destination buffers, so it depends on sizes being the same.
		TAssertNoMsg(sizeof(Vector) == sizeof(aiVector3D));

		if (pAIMesh->mNumVertices)
		{
			if (pAIMesh->mVertices)
			{
				pMesh->m_aVertices.resize(pAIMesh->mNumVertices);
				memcpy(pMesh->m_aVertices.data(), pAIMesh->mVertices, sizeof(Vector)*pMesh->m_aVertices.size());
				pMesh->m_aaVertexFaceMap.resize(pAIMesh->mNumVertices);
			}

			if (pAIMesh->mNormals)
			{
				pMesh->m_aNormals.resize(pAIMesh->mNumVertices);
				memcpy(pMesh->m_aNormals.data(), pAIMesh->mNormals, sizeof(Vector)*pMesh->m_aNormals.size());
			}

			if (pAIMesh->mTangents)
			{
				pMesh->m_aTangents.resize(pAIMesh->mNumVertices);
				memcpy(pMesh->m_aTangents.data(), pAIMesh->mTangents, sizeof(Vector)*pMesh->m_aTangents.size());
			}

			if (pAIMesh->mBitangents)
			{
				pMesh->m_aBitangents.resize(pAIMesh->mNumVertices);
				memcpy(pMesh->m_aBitangents.data(), pAIMesh->mBitangents, sizeof(Vector)*pMesh->m_aBitangents.size());
			}

			if (pAIMesh->mTextureCoords[0])
			{
				// UVs are stored as 3D vectors in Assimp so direct copy is not possible.
				pMesh->m_aUVs.reserve(pAIMesh->mNumVertices);
				for (unsigned int j = 0; j < pAIMesh->mNumVertices; j++)
					pMesh->AddUV(pAIMesh->mTextureCoords[0][j].x, pAIMesh->mTextureCoords[0][j].y);
			}
		}

		aiString sMaterialStubName;
		pScene->mMaterials[pAIMesh->mMaterialIndex]->Get(AI_MATKEY_NAME, sMaterialStubName);
		size_t iCurrentMaterial = pMesh->AddMaterialStub(sMaterialStubName.data);

		for (unsigned int j = 0; j < pAIMesh->mNumFaces; j++)
		{
			size_t iFace = pMesh->AddFace(iCurrentMaterial);

			aiFace* pAIFace = &pAIMesh->mFaces[j];
			for (unsigned int k = 0; k < pAIFace->mNumIndices; k++)
				pMesh->AddVertexToFace(iFace, pAIFace->mIndices[k], pAIFace->mIndices[k], pAIFace->mIndices[k]);
		}

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(i+1);
	}

	// Assimp doesn't have a concept of multiple root scenes, so this is my best approximation
	if (pScene->mRootNode->mNumChildren > 2 && pScene->mRootNode->mNumMeshes == 0)
	{
		if (m_pWorkListener)
			m_pWorkListener->SetAction("Reading scene tree", pScene->mRootNode->mNumChildren);

		for (unsigned int i = 0; i < pScene->mRootNode->mNumChildren; i++)
		{
			aiNode* pAINode = pScene->mRootNode->mChildren[i];
			size_t iScene = m_pScene->AddScene(pAINode->mName.data);
			ReadAssImpSceneTree(pScene, pAINode, m_pScene->GetScene(iScene));
		}
	}
	else
	{
		if (m_pWorkListener)
			m_pWorkListener->SetAction("Reading scene tree", 0);

		size_t iScene = m_pScene->AddScene(pScene->mRootNode->mName.data);
		ReadAssImpSceneTree(pScene, pScene->mRootNode, m_pScene->GetScene(iScene));
	}

	aiReleaseImport(pScene);

	m_pScene->SetWorkListener(m_pWorkListener);

	m_pScene->CalculateExtends();

	for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
	{
		m_pScene->GetMesh(i)->CalculateEdgeData();

		if (m_pScene->GetMesh(i)->GetNumNormals() == 0)
			m_pScene->GetMesh(i)->CalculateVertexNormals();

		if (m_pScene->GetMesh(i)->GetNumTangents() == 0)
			m_pScene->GetMesh(i)->CalculateVertexTangents();
	}

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();

	return true;
}

void CModelConverter::ReadAssImpSceneTree(const aiScene* pAIScene, aiNode* pNode, CConversionSceneNode* pScene)
{
	pScene->SetTransformations(Matrix4x4(&pNode->mTransformation.a1).Transposed());

	for (unsigned int i = 0; i < pNode->mNumMeshes; i++)
	{
		size_t iInstance = pScene->AddMeshInstance(pNode->mMeshes[i]);
		pScene->GetMeshInstance(iInstance)->AddMappedMaterial(pAIScene->mMeshes[i]->mMaterialIndex, pAIScene->mMeshes[i]->mMaterialIndex);
	}

	// Wow that was easy. I like AssImp.

	for (unsigned int i = 0; i < pNode->mNumChildren; i++)
	{
		size_t iScene = pScene->AddChild(pNode->mChildren[i]->mName.data);
		ReadAssImpSceneTree(pAIScene, pNode->mChildren[i], pScene->GetChild(iScene));
	}
}

#endif
