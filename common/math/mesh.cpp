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

#include "mesh.h"

#include <common.h>
#include <tstring.h>
#include <geometry.h>

#include <tinker/shell.h>

CConvexHullGenerator::CConvexHullGenerator(const tvector<Vector>& avecPoints)
	: m_avecPoints(avecPoints)
{
	// Reserve memory up front to avoid costly allocations
	m_aiTriangles.reserve(m_avecPoints.size());
}

void CConvexHullGenerator::CreateConvex()
{
	size_t i1 = FindLowestPoint();
	size_t i2 = FindNextPoint(i1, -1);

	AddEdge(i2, i1);

#if defined(_DEBUG) && defined(DEBUG_CONVEXNESS)
	Vector vecCenter;
	for (size_t i = 0; i < m_avecPoints.size(); i++)
		vecCenter += m_avecPoints[i];
	vecCenter /= (float)m_avecPoints.size();
#endif

	while (m_aOpenEdges.size())
	{
		i1 = m_aOpenEdges.back().p1;
		i2 = m_aOpenEdges.back().p2;
		m_aOpenEdges.pop_back();

		if (!EdgeExists(i1, i2))
		{
			size_t i3 = FindNextPoint(i1, i2);

#if defined(_DEBUG) && defined(DEBUG_CONVEXNESS)
			Vector v1 = m_avecPoints[i1];
			Vector v2 = m_avecPoints[i2];
			Vector v3 = m_avecPoints[i3];
			Vector n = (v2-v1).Cross(v3-v1).Normalized();
			TAssert(n.Dot(vecCenter-v1) < 0);
#endif

			m_aiTriangles.push_back(i1);
			m_aiTriangles.push_back(i2);
			m_aiTriangles.push_back(i3);

			AddEdge(i1, i2);
			AddEdge(i2, i3);
			AddEdge(i3, i1);
		}
	}
}

size_t CConvexHullGenerator::FindLowestPoint()
{
	size_t iIndex = 0;
	for (size_t i = 1; i < m_avecPoints.size(); i++)
	{
		if (m_avecPoints[i].z < m_avecPoints[iIndex].z)
			iIndex = i;
		else if (m_avecPoints[i].z == m_avecPoints[iIndex].z)
		{
			if (m_avecPoints[i].y < m_avecPoints[iIndex].y)
				iIndex = i;
			else if (m_avecPoints[i].x < m_avecPoints[iIndex].x)
				iIndex = i;

			else
				TAssert(m_avecPoints[i].y != m_avecPoints[iIndex].y || m_avecPoints[i].x != m_avecPoints[iIndex].x);	// duplicate point
		}
	}

	return iIndex;
}

size_t CConvexHullGenerator::FindNextPoint(size_t p1, size_t p2)
{
	Vector v1 = m_avecPoints[p1];
	Vector v2;
	if (p2 == ~0)
		v2 = v1 - Vector(1, 0, 1);
	else
		v2 = m_avecPoints[p2];

	Vector vecEdge = (v2 - v1).Normalized();

	size_t iHighest = -1;
	Vector vecHighestNormal, vecHighest;
	bool bFirst = true;

	for (size_t i = 0; i < m_avecPoints.size(); i++)
	{
		if (i == p1 || i == p2)
			continue;

		iHighest = i;
		vecHighest = m_avecPoints[iHighest];
		vecHighestNormal = (vecHighest - v1).Cross(vecEdge);
		break;
	}

	tvector<size_t> aZeros;
	for (size_t i = 0; i < m_avecPoints.size(); i++)
	{
		if (i == p1 || i == p2 || i == iHighest)
			continue;

		Vector vecCandidate = m_avecPoints[i];

		float flDot = vecHighestNormal.Dot(vecCandidate - v1);

		if (flDot < -0.0001f)
		{
			iHighest = i;
			vecHighest = m_avecPoints[iHighest];
			vecHighestNormal = (vecHighest - v1).Cross(vecEdge);
			bFirst = false;
		}
		else if (bFirst && flDot <= 0 && flDot >= -0.0001f)
			aZeros.push_back(i);
	}

	// Any points coplanar with the first should be reconsidered.
	for (size_t i = 0; i < aZeros.size(); i++)
	{
		Vector vecCandidate = m_avecPoints[aZeros[i]];

		float flDot = vecHighestNormal.Dot(vecCandidate - v1);

		if (flDot < -0.0001f)
		{
			iHighest = aZeros[i];
			vecHighest = m_avecPoints[iHighest];
			vecHighestNormal = (vecHighest - v1).Cross(vecEdge);
		}
	}

	return iHighest;
}

const tvector<size_t>& CConvexHullGenerator::GetConvexTriangles()
{
	if (!m_aiTriangles.size())
		CreateConvex();

	return m_aiTriangles;
}

bool CConvexHullGenerator::EdgeExists(size_t p1, size_t p2)
{
	return m_aaCreatedEdges[p1][p2];
}

void CConvexHullGenerator::AddEdge(size_t p1, size_t p2)
{
	m_aaCreatedEdges[p1][p2] = true;

	if (!EdgeExists(p2, p1))
		m_aOpenEdges.push_back(EdgePair(p2, p1));
}

void CCoplanarPointOptimizer::OptimizeMesh(const tvector<Vector>& avecPoints, tvector<size_t>& aiTriangles, float flTolerance)
{
	TAssert(aiTriangles.size() % 3 == 0);

	tvector<size_t> aiPointTriangles;
	tvector<size_t> aiRemoveTriangles;

	bool bFound;
	do
	{
		bFound = false;

		for (size_t i = 0; i < avecPoints.size(); i++)
		{
			aiPointTriangles.clear();

			// aiNumberOfEdges[x] is the number of triangles which have an edge on i, x
			tmap<size_t, size_t> aiTrisWithEdge;

			// Find all triangles with this point.
			for (size_t j = 0; j < aiTriangles.size(); j += 3)
			{
				if (aiTriangles[j] == i || aiTriangles[j+1] == i || aiTriangles[j+2] == i)
				{
					aiPointTriangles.push_back(j);

					if (aiTriangles[j] == i)
					{
						aiTrisWithEdge[aiTriangles[j+1]]++;
						aiTrisWithEdge[aiTriangles[j+2]]++;
					}
					else if (aiTriangles[j+1] == i)
					{
						aiTrisWithEdge[aiTriangles[j]]++;
						aiTrisWithEdge[aiTriangles[j+2]]++;
					}
					else
					{
						aiTrisWithEdge[aiTriangles[j+1]]++;
						aiTrisWithEdge[aiTriangles[j]]++;
					}
				}
			}

			// We need at least two triangles to remove a point and still have at triangle left over
			if (aiPointTriangles.size() < 3)
				continue;

			// We need each edge to have two triangles coming from it.
			// Otherwise reject this as not being a continuous mesh.
			bool bContinue = false;
			for (auto it = aiTrisWithEdge.begin(); it != aiTrisWithEdge.end(); it++)
			{
				if (it->second != 2)
				{
					bContinue = true;
					break;
				}
			}

			if (bContinue)
				continue;

			bool bRemovable = true;

			Vector v1 = avecPoints[aiTriangles[aiPointTriangles[0]]];
			Vector v2 = avecPoints[aiTriangles[aiPointTriangles[0]+1]];
			Vector v3 = avecPoints[aiTriangles[aiPointTriangles[0]+2]];
			Vector vecFirstNormal = (v2 - v1).Cross(v3 - v1).Normalized();

			for (size_t j = 1; j < aiPointTriangles.size(); j += 1)
			{
				size_t iTriangle = aiPointTriangles[j];
				v1 = avecPoints[aiTriangles[iTriangle]];
				v2 = avecPoints[aiTriangles[iTriangle+1]];
				v3 = avecPoints[aiTriangles[iTriangle+2]];

				Vector vecNormal = (v2 - v1).Cross(v3 - v1);

				// Degenerate triangles are coplanar.
				if (vecNormal.LengthSqr() == 0)
					continue;

				vecNormal.Normalize();

				if (vecFirstNormal.Dot(vecNormal) < 1-flTolerance)
				{
					bRemovable = false;
					break;
				}
			}

			if (!bRemovable)
				continue;

			bFound = true;

			size_t iReplaceWith = aiTriangles[aiPointTriangles[0]];
			if (i == iReplaceWith)
				iReplaceWith = aiTriangles[aiPointTriangles[0]+1];

			aiRemoveTriangles.clear();
			aiRemoveTriangles.push_back(aiPointTriangles[0]);

			for (size_t j = 1; j < aiPointTriangles.size(); j += 1)
			{
				size_t iTriangle = aiPointTriangles[j];
				size_t iV1 = aiTriangles[iTriangle];
				size_t iV2 = aiTriangles[iTriangle+1];
				size_t iV3 = aiTriangles[iTriangle+2];

				if (iV1 == iReplaceWith || iV2 == iReplaceWith || iV3 == iReplaceWith)
				{
					aiRemoveTriangles.push_back(iTriangle);
					continue;
				}

				if (iV1 == i)
				{
					aiTriangles[iTriangle] = iReplaceWith;
					continue;
				}
				else if (iV2 == i)
				{
					aiTriangles[iTriangle+1] = iReplaceWith;
					continue;
				}
				else
				{
					aiTriangles[iTriangle+2] = iReplaceWith;
					continue;
				}
			}

			while (aiRemoveTriangles.size())
			{
				auto itBack = aiRemoveTriangles.back();
				aiTriangles.erase(aiTriangles.begin() + itBack, aiTriangles.begin() + (itBack+3));
				TAssert(aiTriangles.size() % 3 == 0);
				aiRemoveTriangles.pop_back();
			}

			// We removed a triangle, now break the loop and start from the beginning to make sure there's not another
			// point coplanar with the same triangles that me may miss.
			break;
		}
	}
	while (bFound);
}

void CUnusedPointOptimizer::OptimizeMesh(tvector<Vector>& avecPoints, tvector<size_t>& aiTriangles)
{
	TAssert(aiTriangles.size() % 3 == 0);

	tmap<size_t, size_t> aiUsedVertsMap; // Maps input vertices to the optimized mesh. eg aiUsedVerts[aiUsedVertsMap[aiTriangles[x]]]
	tvector<size_t> aiUsedVerts;
	tvector<size_t> aiUsedVertTriangles;
	aiUsedVertTriangles.reserve(aiTriangles.size());
	aiUsedVerts.reserve(aiTriangles.size()*3);

	for (size_t i = 0; i < aiTriangles.size(); i += 3)
	{
		size_t iV1, iV2, iV3;

		if (aiUsedVertsMap.find(aiTriangles[i]) == aiUsedVertsMap.end())
		{
			aiUsedVerts.push_back(aiTriangles[i]);
			size_t iVert = aiUsedVerts.size()-1;
			aiUsedVertsMap[aiTriangles[i]] = iVert;
			iV1 = iVert;
		}
		else
			iV1 = aiUsedVertsMap[aiTriangles[i]];

		if (aiUsedVertsMap.find(aiTriangles[i+1]) == aiUsedVertsMap.end())
		{
			aiUsedVerts.push_back(aiTriangles[i+1]);
			size_t iVert = aiUsedVerts.size()-1;
			aiUsedVertsMap[aiTriangles[i+1]] = iVert;
			iV2 = iVert;
		}
		else
			iV2 = aiUsedVertsMap[aiTriangles[i+1]];

		if (aiUsedVertsMap.find(aiTriangles[i+2]) == aiUsedVertsMap.end())
		{
			aiUsedVerts.push_back(aiTriangles[i+2]);
			size_t iVert = aiUsedVerts.size()-1;
			aiUsedVertsMap[aiTriangles[i+2]] = iVert;
			iV3 = iVert;
		}
		else
			iV3 = aiUsedVertsMap[aiTriangles[i+2]];

		aiUsedVertTriangles.push_back(iV1);
		aiUsedVertTriangles.push_back(iV2);
		aiUsedVertTriangles.push_back(iV3);
	}

	tvector<Vector> avecUsedPoints;
	for (size_t v = 0; v < aiUsedVerts.size(); v++)
		avecUsedPoints.push_back(avecPoints[aiUsedVerts[v]]);

	avecPoints = avecUsedPoints;
	aiTriangles = aiUsedVertTriangles;
}
