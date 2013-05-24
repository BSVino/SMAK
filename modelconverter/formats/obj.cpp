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

#include <stdio.h>
#include <string.h>

#include <common.h>

#include "../modelconverter.h"
#include "strutils.h"

bool CModelConverter::ReadOBJ(const tstring& sFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	FILE* fp = tfopen(sFilename, "r");

	if (!fp)
	{
		printf("No input file. Sorry!\n");
		return false;
	}

	CConversionSceneNode* pScene = m_pScene->GetScene(m_pScene->AddScene(GetFilename(sFilename).append(".obj")));

	CConversionMesh* pMesh = m_pScene->GetMesh(m_pScene->AddMesh(GetFilename(sFilename)));
	// Make sure it exists.
	CConversionSceneNode* pMeshNode = m_pScene->GetDefaultSceneMeshInstance(pScene, pMesh);

	size_t iCurrentMaterial = ~0;
	size_t iSmoothingGroup = ~0;

	bool bSmoothingGroups = false;

	tstring sLastTask;

	int iTotalVertices = 0;
	int iTotalFaces = 0;
	int iVerticesComplete = 0;
	int iFacesComplete = 0;

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Reading file into memory...", 0);

	fseek(fp, 0L, SEEK_END);
	long iOBJSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	// Make sure we allocate more than we need just in case.
	size_t iFileSize = (iOBJSize+1) * (sizeof(tchar)+1);
	tchar* pszEntireFile = (tchar*)malloc(iFileSize);
	tchar* pszCurrent = pszEntireFile;
	pszCurrent[0] = '\0';

	// Read the entire file into an array first for faster processing.
	tstring sLine;
	while (fgetts(sLine, fp))
	{
		tstrncpy(pszCurrent, iFileSize-(pszCurrent-pszEntireFile), sLine.c_str(), sLine.length());
		size_t iLength = sLine.length();

		tchar cLastChar = pszCurrent[iLength-1];
		while (cLastChar == '\n' || cLastChar == '\r')
		{
			pszCurrent[iLength-1] = '\0';
			iLength--;
			cLastChar = pszCurrent[iLength-1];
		}

		pszCurrent += iLength;
		pszCurrent++;

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(0);
	}

	pszCurrent[0] = '\0';

	fclose(fp);

	const tchar* pszFileEnd = pszCurrent;
	const tchar* pszLine = pszEntireFile;
	const tchar* pszNextLine = NULL;
	while (pszLine < pszFileEnd)
	{
		if (pszNextLine)
			pszLine = pszNextLine;

		int iLineLength = tstrlen(pszLine);

		pszNextLine = pszLine + iLineLength + 1;

		// This code used to call StripWhitespace() but that's too slow for very large files w/ millions of lines.
		// Instead we'll just cut the whitespace off the front and deal with whitespace on the end when we come to it.
		while (*pszLine && IsWhitespace(*pszLine))
			pszLine++;

		if (pszLine >= pszFileEnd)
			continue;

		iLineLength = tstrlen(pszLine);
		if (tstrlen(pszLine) == 0)
			continue;

		if (pszLine[0] == '#')
		{
			// ZBrush is kind enough to notate exactly how many vertices and faces we have in the comments at the top of the file.
			if (tstrncmp(pszLine, "#Vertex Count", 13) == 0)
			{
				iTotalVertices = atoi(pszLine+13);
				pMesh->SetTotalVertices(iTotalVertices);
			}

			if (tstrncmp(pszLine, "#Face Count", 11) == 0)
			{
				iTotalFaces = atoi(pszLine+11);
				pMesh->SetTotalFaces(iTotalFaces);
			}

			continue;
		}

		tchar szToken[1024];
		TAssertNoMsg(iLineLength < 1024);

		tstrncpy(szToken, 1024, pszLine, iLineLength+1);
		tchar* pszState = NULL;
		tchar* pszToken = strtok<tchar>(szToken, " ", &pszState);

		if (tstrncmp(pszToken, "mtllib", 6) == 0)
		{
			tstring sDirectory = GetDirectory(sFilename);
			tstring sMaterial = sprintf(tstring("%s/%s"), sDirectory.c_str(), pszLine + 7);
			ReadMTL(sMaterial);
		}
		else if (tstrncmp(pszToken, "o", 1) == 0)
		{
			// Dunno what this does.
		}
		else if (tstrncmp(pszToken, "v", 2) == 0)
		{
			if (m_pWorkListener)
			{
				if (tstrncmp(sLastTask.c_str(), pszToken, sLastTask.length()) == 0)
					m_pWorkListener->WorkProgress(iVerticesComplete++);
				else
				{
					m_pWorkListener->SetAction("Reading vertex data", iTotalVertices);
					sLastTask = tstring(pszToken);
				}
			}

			// A vertex.
			float v[3];
			// scanf is pretty slow even for such a short string due to lots of mallocs.
			const tchar* pszToken = pszLine+1;
			int iDimension = 0;
			while (*pszToken)
			{
				while (pszToken[0] == ' ')
					pszToken++;

				v[iDimension++] = (float)atof(pszToken);
				if (iDimension >= 3)
					break;

				while (pszToken[0] != ' ')
					pszToken++;
			}
			pMesh->AddVertex(v[0], v[1], v[2]);
		}
		else if (tstrncmp(pszToken, "vn", 3) == 0)
		{
			if (m_pWorkListener)
			{
				if (tstrncmp(sLastTask.c_str(), pszToken, sLastTask.length()) == 0)
					m_pWorkListener->WorkProgress(0);
				else
					m_pWorkListener->SetAction("Reading vertex normal data", 0);
			}
			sLastTask = tstring(pszToken);

			// A vertex normal.
			float x, y, z;
			tvector<tstring> asTokens;
			tstrtok(pszLine, asTokens, " ");
			if (asTokens.size() == 4)
			{
				x = (float)atof(asTokens[1].c_str());
				y = (float)atof(asTokens[2].c_str());
				z = (float)atof(asTokens[3].c_str());
				pMesh->AddNormal(x, y, z);
			}
		}
		else if (tstrncmp(pszToken, "vt", 3) == 0)
		{
			if (m_pWorkListener)
			{
				if (tstrncmp(sLastTask.c_str(), pszToken, sLastTask.length()) == 0)
					m_pWorkListener->WorkProgress(0);
				else
					m_pWorkListener->SetAction("Reading texture coordinate data", 0);
			}
			sLastTask = tstring(pszToken);

			// A UV coordinate for a vertex.
			float u, v;
			tvector<tstring> asTokens;
			tstrtok(pszLine, asTokens, " ");
			if (asTokens.size() >= 3)
			{
				u = (float)atof(asTokens[1].c_str());
				v = (float)atof(asTokens[2].c_str());
				pMesh->AddUV(u, v);
			}
		}
		else if (tstrncmp(pszToken, "g", 1) == 0)
		{
			// A group of faces.
			pMesh->AddBone(pszLine+2);
		}
		else if (tstrncmp(pszToken, "usemtl", 6) == 0)
		{
			// All following faces should use this material.
			tstring sMaterial = tstring(pszLine+7);
			size_t iMaterial = pMesh->FindMaterialStub(sMaterial);
			if (iMaterial == ((size_t)~0))
			{
				size_t iSceneMaterial = m_pScene->FindMaterial(sMaterial);
				if (iSceneMaterial == ((size_t)~0))
					iCurrentMaterial = m_pScene->AddDefaultSceneMaterial(pScene, pMesh, sMaterial);
				else
				{
					size_t iMaterialStub = pMesh->AddMaterialStub(sMaterial);
					m_pScene->GetDefaultSceneMeshInstance(pScene, pMesh)->GetMeshInstance(0)->AddMappedMaterial(iMaterialStub, iSceneMaterial);
					iCurrentMaterial = iMaterialStub;
				}
			}
			else
				iCurrentMaterial = iMaterial;
		}
		else if (tstrncmp(pszToken, "s", 1) == 0)
		{
			if (tstrncmp(pszLine, "s off", 5) == 0)
			{
				iSmoothingGroup = ~0;
			}
			else
			{
				bSmoothingGroups = true;
				tvector<tstring> asTokens;
				tstrtok(pszLine, asTokens, " ");
				if (asTokens.size() == 2)
					iSmoothingGroup = atoi(asTokens[1].c_str());
			}
		}
		else if (tstrncmp(pszToken, "f", 1) == 0)
		{
			if (m_pWorkListener)
			{
				if (tstrncmp(sLastTask.c_str(), pszToken, sLastTask.length()) == 0)
					m_pWorkListener->WorkProgress(iFacesComplete++);
				else
				{
					m_pWorkListener->SetAction("Reading polygon data", iTotalFaces);
					sLastTask = tstring(pszToken);
				}
			}

			if (iCurrentMaterial == ~0)
				iCurrentMaterial = m_pScene->AddDefaultSceneMaterial(pScene, pMesh, pMesh->GetName());

			// A face.
			size_t iFace = pMesh->AddFace(iCurrentMaterial);

			pMesh->GetFace(iFace)->m_iSmoothingGroup = iSmoothingGroup;

			while (pszToken = strtok<tchar>(NULL, " ", &pszState))
			{
				if (tstrlen(pszToken) == 0)
					continue;

				// We don't use size_t because SOME EXPORTS put out negative numbers.
				long f[3];
				bool bValues[3];
				bValues[0] = false;
				bValues[1] = false;
				bValues[2] = false;

				// scanf is pretty slow even for such a short string due to lots of mallocs.
				const tchar* pszValues = pszToken;
				int iValue = 0;
				do
				{
					if (!pszValues)
						break;

					if (!bValues[0] || pszValues[0] == '/')
					{
						if (pszValues[0] == '/')
							pszValues++;

						bValues[iValue] = true;
						f[iValue++] = (long)atoi(pszValues);
						if (iValue >= 3)
							break;
					}

					// Don't advance if we're on a slash, because that means empty slashes. ie, 11//12 <-- the 12 would get skipped.
					if (pszValues[0] != '/')
						pszValues++;
				}
				while (*pszValues);

				if (bValues[0])
				{
					if (f[0] < 0)
						f[0] = (long)pMesh->GetNumVertices()+f[0]+1;
					TAssertNoMsg ( f[0] >= 1 && f[0] < (long)pMesh->GetNumVertices()+1 );
				}

				if (bValues[1] && pMesh->GetNumUVs())
				{
					if (f[1] < 0)
						f[1] = (long)pMesh->GetNumUVs()+f[1]+1;
					TAssertNoMsg ( f[1] >= 1 && f[1] < (long)pMesh->GetNumUVs()+1 );
				}

				if (bValues[2] && pMesh->GetNumNormals())
				{
					if (f[2] < 0)
						f[2] = (long)pMesh->GetNumNormals()+f[2]+1;
					TAssertNoMsg ( f[2] >= 1 && f[2] < (long)pMesh->GetNumNormals()+1 );
				}

				// OBJ uses 1-based indexing.
				// Convert to 0-based indexing.
				f[0]--;
				f[1]--;
				f[2]--;

				if (!pMesh->GetNumUVs())
					f[1] = ~0;
				if (bValues[2] == false || !pMesh->GetNumNormals())
					f[2] = ~0;

				pMesh->AddVertexToFace(iFace, f[0], f[1], f[2]);
			}
		}
	}

	free(pszEntireFile);

	m_pScene->SetWorkListener(m_pWorkListener);

	m_pScene->CalculateExtends();

	for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
	{
		if (m_bWantEdges)
			m_pScene->GetMesh(i)->CalculateEdgeData();

		if (bSmoothingGroups || m_pScene->GetMesh(i)->GetNumNormals() == 0)
			m_pScene->GetMesh(i)->CalculateVertexNormals();

		m_pScene->GetMesh(i)->CalculateVertexTangents();
	}

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();

	return true;
}

void CModelConverter::ReadMTL(const tstring& sFilename)
{
	FILE* fp = tfopen(sFilename, "r");

	if (!fp)
		return;

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Reading materials", 0);

	size_t iCurrentMaterial = ~0;

	tstring sLine;
	while (fgetts(sLine, fp))
	{
		sLine = StripWhitespace(sLine);

		if (sLine.length() == 0)
			continue;

		if (sLine[0] == '#')
			continue;

		tvector<tstring> asTokens;
		tstrtok(sLine, asTokens, " ");
		const tchar* pszToken = NULL;
		pszToken = asTokens[0].c_str();

		CConversionMaterial* pMaterial = NULL;
		if (iCurrentMaterial != ~0)
			pMaterial = m_pScene->GetMaterial(iCurrentMaterial);

		if (tstrncmp(pszToken, "newmtl", 6) == 0)
		{
			pszToken = asTokens[1].c_str();
			CConversionMaterial oMaterial(pszToken, Vector(0.2f,0.2f,0.2f), Vector(0.8f,0.8f,0.8f), Vector(1,1,1), Vector(0,0,0), 1.0, 0);
			iCurrentMaterial = m_pScene->AddMaterial(oMaterial);
		}
		else if (tstrncmp(pszToken, "Ka", 2) == 0)
		{
			tvector<tstring> asTokens;
			tstrtok(sLine, asTokens, " ");
			if (asTokens.size() == 4)
			{
				pMaterial->m_vecAmbient.x = (float)atof(asTokens[1].c_str());
				pMaterial->m_vecAmbient.y = (float)atof(asTokens[2].c_str());
				pMaterial->m_vecAmbient.z = (float)atof(asTokens[3].c_str());
			}
		}
		else if (tstrncmp(pszToken, "Kd", 2) == 0)
		{
			tvector<tstring> asTokens;
			tstrtok(sLine, asTokens, " ");
			if (asTokens.size() == 4)
			{
				pMaterial->m_vecDiffuse.x = (float)atof(asTokens[1].c_str());
				pMaterial->m_vecDiffuse.y = (float)atof(asTokens[2].c_str());
				pMaterial->m_vecDiffuse.z = (float)atof(asTokens[3].c_str());
			}
		}
		else if (tstrncmp(pszToken, "Ks", 2) == 0)
		{
			tvector<tstring> asTokens;
			tstrtok(sLine, asTokens, " ");
			if (asTokens.size() == 4)
			{
				pMaterial->m_vecSpecular.x = (float)atof(asTokens[1].c_str());
				pMaterial->m_vecSpecular.y = (float)atof(asTokens[2].c_str());
				pMaterial->m_vecSpecular.z = (float)atof(asTokens[3].c_str());
			}
		}
		else if (tstrncmp(pszToken, "d", 1) == 0 || tstrncmp(pszToken, "Tr", 2) == 0)
		{
			pMaterial->m_flTransparency = (float)atof(asTokens[1].c_str());
		}
		else if (tstrncmp(pszToken, "Ns", 2) == 0)
		{
			pMaterial->m_flShininess = (float)atof(asTokens[1].c_str())*128/1000;
		}
		else if (tstrncmp(pszToken, "illum", 5) == 0)
		{
			pMaterial->m_eIllumType = (IllumType_t)atoi(asTokens[1].c_str());
		}
		else if (tstrncmp(pszToken, "map_Kd", 6) == 0)
		{
			tstring sFile = sLine.substr(7);
			pszToken = sFile.c_str();

			FILE* fpTest = tfopen(pszToken, "r");

			if (fpTest)
			{
				fclose(fpTest);

				pMaterial->m_sDiffuseTexture = tstring(pszToken);
			}
			else
			{
				tstring sDirectory = GetDirectory(sFilename);

				pMaterial->m_sDiffuseTexture = sprintf(tstring("%s/%s"), sDirectory.c_str(), pszToken);
			}
		}
	}

	fclose(fp);
}

void CModelConverter::SaveOBJ(const tstring& sFilename)
{
	tstring sMaterialFileName = tstring(GetDirectory(sFilename).c_str()) + "/" + GetFilename(sFilename).c_str() + ".mtl";

	std::wofstream sMaterialFile(sMaterialFileName.c_str());
	if (!sMaterialFile.is_open())
		return;

	if (m_pWorkListener)
	{
		m_pWorkListener->BeginProgress();
		m_pWorkListener->SetAction("Writing materials file", 0);
	}

	for (size_t i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		CConversionMaterial* pMaterial = m_pScene->GetMaterial(i);
		sMaterialFile << "newmtl " << pMaterial->GetName().c_str() << std::endl;
		sMaterialFile << "Ka " << pMaterial->m_vecAmbient.x << " " << pMaterial->m_vecAmbient.y << " " << pMaterial->m_vecAmbient.z << std::endl;
		sMaterialFile << "Kd " << pMaterial->m_vecDiffuse.x << " " << pMaterial->m_vecDiffuse.y << " " << pMaterial->m_vecDiffuse.z << std::endl;
		sMaterialFile << "Ks " << pMaterial->m_vecSpecular.x << " " << pMaterial->m_vecSpecular.y << " " << pMaterial->m_vecSpecular.z << std::endl;
		sMaterialFile << "d " << pMaterial->m_flTransparency << std::endl;
		sMaterialFile << "Ns " << pMaterial->m_flShininess << std::endl;
		sMaterialFile << "illum " << pMaterial->m_eIllumType << std::endl;
		if (pMaterial->GetDiffuseTexture().length() > 0)
			sMaterialFile << "map_Kd " << pMaterial->GetDiffuseTexture().c_str() << std::endl;
		sMaterialFile << std::endl;
	}

	sMaterialFile.close();

	for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(i);

		// Find the default scene for this mesh.
		CConversionSceneNode* pScene = NULL;
		for (size_t j = 0; j < m_pScene->GetNumScenes(); j++)
		{
			if (m_pScene->GetScene(j)->GetName() == pMesh->GetName() + ".obj")
			{
				pScene = m_pScene->GetScene(j);
				break;
			}
		}

		tstring sNodeName = pMesh->GetName();

		tstring sOBJFilename = tstring(GetDirectory(sFilename).c_str()) + "/" + GetFilename(sNodeName).c_str() + ".obj";
		tstring sMTLFilename = tstring(GetFilename(sFilename).c_str()) + ".mtl";

		if (m_pScene->GetNumMeshes() == 1)
			sOBJFilename = sFilename;

		std::ofstream sOBJFile(sOBJFilename.c_str());
		sOBJFile.precision(8);
		sOBJFile.setf(std::ios::fixed, std::ios::floatfield);

		sOBJFile << "mtllib " << sMTLFilename.c_str() << std::endl;
		sOBJFile << std::endl;

		sOBJFile << "o " << sNodeName.c_str() << std::endl;

		if (m_pWorkListener)
			m_pWorkListener->SetAction((tstring("Writing ") + sNodeName + " vertices...").c_str(), pMesh->GetNumVertices());

		for (size_t iVertices = 0; iVertices < pMesh->GetNumVertices(); iVertices++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iVertices);

			Vector vecVertex = pMesh->GetVertex(iVertices);
			sOBJFile << "v " << vecVertex.x << " " << vecVertex.y << " " << vecVertex.z << std::endl;
		}

		if (m_pWorkListener)
			m_pWorkListener->SetAction((tstring("Writing ") + sNodeName + " normals...").c_str(), pMesh->GetNumNormals());

		for (size_t iNormals = 0; iNormals < pMesh->GetNumNormals(); iNormals++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iNormals);

			Vector vecNormal = pMesh->GetNormal(iNormals);
			sOBJFile << "vn " << vecNormal.x << " " << vecNormal.y << " " << vecNormal.z << std::endl;
		}

		if (m_pWorkListener)
			m_pWorkListener->SetAction((tstring("Writing ") + sNodeName + " UVs...").c_str(), pMesh->GetNumUVs());

		for (size_t iUVs = 0; iUVs < pMesh->GetNumUVs(); iUVs++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iUVs);

			Vector vecUV = pMesh->GetUV(iUVs);
			sOBJFile << "vt " << vecUV.x << " " << vecUV.y << std::endl;
		}

		if (m_pWorkListener)
			m_pWorkListener->SetAction((tstring("Writing ") + sNodeName + " faces...").c_str(), pMesh->GetNumFaces());

		size_t iLastMaterial = ~0;

		for (size_t iFaces = 0; iFaces < pMesh->GetNumFaces(); iFaces++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iFaces);

			CConversionFace* pFace = pMesh->GetFace(iFaces);

			if (pFace->m != iLastMaterial)
			{
				iLastMaterial = pFace->m;

				CConversionSceneNode* pNode = m_pScene->GetDefaultSceneMeshInstance(pScene, pMesh, false);
				if (!pNode || pNode->GetNumMeshInstances() != 1)
					sOBJFile << "usemtl " << iLastMaterial << std::endl;
				else
				{
					CConversionMaterialMap* pMap = pNode->GetMeshInstance(0)->GetMappedMaterial(iLastMaterial);
					if (!pMap)
						sOBJFile << "usemtl " << iLastMaterial << std::endl;
					else
						sOBJFile << "usemtl " << m_pScene->GetMaterial(pMap->m_iMaterial)->GetName().c_str() << std::endl;
				}
			}

			sOBJFile << "f";
			for (size_t iVertsInFace = 0; iVertsInFace < pFace->GetNumVertices(); iVertsInFace++)
			{
				CConversionVertex* pVertex = pFace->GetVertex(iVertsInFace);
				sOBJFile << " " << pVertex->v+1 << "/" << pVertex->vu+1 << "/" << pVertex->vn+1;
			}
			sOBJFile << std::endl;
		}

		sOBJFile << std::endl;

		sOBJFile.close();
	}

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}
