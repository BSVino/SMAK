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

#include "raytracer.h"

#include <common.h>
#include <geometry.h>

//#define DEBUG_WITH_GL
#ifdef DEBUG_WITH_GL
//#include <../../gl3w/include/GL3/gl3w.h>

void DrawBox(AABB& b, float c);
void DrawTri(Vector v1, Vector v2, Vector v3, float r, float g, float b);
#endif

using namespace raytrace;

#define MIN_TRIS_NODE 3

CRaytracer::CRaytracer(CConversionScene* pScene, size_t iMaxDepth)
{
	m_pScene = pScene;
	m_pTree = NULL;

	m_iMaxDepth = iMaxDepth;
}

CRaytracer::~CRaytracer()
{
	if (m_pTree)
		delete m_pTree;
}

bool CRaytracer::Raytrace(const Ray& rayTrace, CTraceResult* pTR)
{
	if (!m_pTree)
		return RaytraceBruteForce(rayTrace, pTR);

	return m_pTree->Raytrace(rayTrace, pTR);
}

bool CRaytracer::Raytrace(const Vector& vecStart, const Vector& vecEnd, CTraceResult* pTR)
{
	if (!m_pTree)
		return false;

	return m_pTree->Raytrace(vecStart, vecEnd, pTR);
}

bool CRaytracer::RaytraceBruteForce(const Ray& rayTrace, CTraceResult* pTR)
{
	// Brute force method.
	Vector vecClosest;
	CConversionFace* pFaceClosest;
	bool bFound = false;

	for (size_t m = 0; m < m_pScene->GetNumMeshes(); m++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(m);
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);
			for (size_t t = 0; t < pFace->GetNumVertices()-2; t++)
			{
				CConversionVertex* pV1 = pFace->GetVertex(0);
				CConversionVertex* pV2 = pFace->GetVertex(t+1);
				CConversionVertex* pV3 = pFace->GetVertex(t+2);

				Vector v1 = pMesh->GetVertex(pV1->v);
				Vector v2 = pMesh->GetVertex(pV2->v);
				Vector v3 = pMesh->GetVertex(pV3->v);

				Vector vecHit;
				if (RayIntersectsTriangle(rayTrace, v1, v2, v3, &vecHit))
				{
					if (!bFound || (vecHit - rayTrace.m_vecPos).LengthSqr() < (vecClosest - rayTrace.m_vecPos).LengthSqr())
					{
						bFound = true;
						vecClosest = vecHit;
						pFaceClosest = pFace;
					}
				}
			}
		}
	}

	if (bFound)
	{
		if (pTR)
		{
			pTR->m_vecHit = vecClosest;
			pTR->m_pFace = pFaceClosest;
			pTR->m_pMeshInstance = NULL;
		}
		return true;
	}

	return false;
}

float CRaytracer::Closest(const Vector& vecPoint)
{
	if (!m_pTree)
		return -1;

	return m_pTree->Closest(vecPoint);
}

void CRaytracer::BuildTree()
{
	if (!m_pTree)
		m_pTree = new CKDTree(m_iMaxDepth);

	m_pTree->BuildTree();
}

void CRaytracer::RemoveArea(const AABB& oBox)
{
	if (!m_pTree)
		return;

	m_pTree->RemoveArea(oBox);
}

void CRaytracer::AddMeshesFromNode(CConversionSceneNode* pNode)
{
	if (!m_pTree)
		m_pTree = new CKDTree(m_iMaxDepth);

	for (size_t c = 0; c < pNode->GetNumChildren(); c++)
		AddMeshesFromNode(pNode->GetChild(c));

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
	{
		CConversionMeshInstance* pMeshInstance = pNode->GetMeshInstance(m);
		AddMeshInstance(pMeshInstance);
	}
}

void CRaytracer::AddMeshInstance(CConversionMeshInstance* pMeshInstance)
{
	if (!m_pTree)
		m_pTree = new CKDTree(m_iMaxDepth);

	tvector<Vector> avecPoints;

	size_t iFaces = pMeshInstance->GetMesh()->GetNumFaces();

	m_pTree->ReserveTriangles(iFaces*2);

	for (size_t f = 0; f < iFaces; f++)
	{
		avecPoints.clear();

		CConversionFace* pFace = pMeshInstance->GetMesh()->GetFace(f);

		size_t iVertices = pFace->GetNumVertices();
		for (size_t t = 0; t < iVertices; t++)
			avecPoints.push_back(pMeshInstance->GetVertex(pFace->GetVertex(t)->v));

		while (avecPoints.size() > 3)
		{
			size_t iEar = FindEar(avecPoints);
			size_t iLast = iEar==0?avecPoints.size()-1:iEar-1;
			size_t iNext = iEar==avecPoints.size()-1?0:iEar+1;
			m_pTree->AddTriangle(avecPoints[iLast], avecPoints[iEar], avecPoints[iNext], pFace, pMeshInstance);
			avecPoints.erase(avecPoints.begin()+iEar);
		}
		m_pTree->AddTriangle(avecPoints[0], avecPoints[1], avecPoints[2], pFace, pMeshInstance);
	}
}

void CRaytracer::AddTriangle(Vector v1, Vector v2, Vector v3)
{
	if (!m_pTree)
		m_pTree = new CKDTree(m_iMaxDepth);

	m_pTree->AddTriangle(v1, v2, v3);
}

CKDTree::CKDTree(size_t iMaxDepth)
{
	m_pTop = new CKDNode(NULL, AABB(), this);
	m_bBuilt = false;

	m_iMaxDepth = iMaxDepth;
}

CKDTree::~CKDTree()
{
	delete m_pTop;
}

void CKDTree::ReserveTriangles(size_t iEstimatedTriangles)
{
	m_pTop->ReserveTriangles(iEstimatedTriangles);
}

void CKDTree::AddTriangle(Vector v1, Vector v2, Vector v3, CConversionFace* pFace, CConversionMeshInstance* pMeshInstance)
{
	m_pTop->AddTriangle(v1, v2, v3, pFace, pMeshInstance);
}

void CKDTree::RemoveArea(const AABB& oBox)
{
	m_pTop->RemoveArea(oBox);
}

void CKDTree::BuildTree()
{
	if (IsBuilt())
		return;

	m_pTop->CalcBounds();
	m_pTop->Build();

	m_bBuilt = true;
}

bool CKDTree::Raytrace(const Ray& rayTrace, CTraceResult* pTR)
{
	if (!m_bBuilt)
		BuildTree();

	if (!RayIntersectsAABB(rayTrace, m_pTop->GetBounds()))
		return false;

	return m_pTop->Raytrace(rayTrace, pTR);
}

bool CKDTree::Raytrace(const Vector& vecStart, const Vector& vecEnd, CTraceResult* pTR)
{
	if (!m_bBuilt)
		BuildTree();

	if (!SegmentIntersectsAABB(vecStart, vecEnd, m_pTop->GetBounds()))
		return false;

	return m_pTop->Raytrace(vecStart, vecEnd, pTR);
}

float CKDTree::Closest(const Vector& vecPoint)
{
	if (!m_bBuilt)
		BuildTree();

	return m_pTop->Closest(vecPoint);
}

CKDNode::CKDNode(CKDNode* pParent, AABB oBounds, CKDTree* pTree)
{
	if (pTree)
		m_pTree = pTree;
	else
		m_pTree = pParent->m_pTree;

	m_pParent = pParent;
	m_oBounds = oBounds;

	if (m_pParent)
		m_iDepth = m_pParent->m_iDepth + 1;
	else
		m_iDepth = 0;

	m_pLeft = NULL;
	m_pRight = NULL;

	m_iTriangles = 0;
}

CKDNode::~CKDNode()
{
	if (m_pLeft)
		delete m_pLeft;
	if (m_pRight)
		delete m_pRight;
}

void CKDNode::ReserveTriangles(size_t iEstimatedTriangles)
{
	m_aTris.reserve(iEstimatedTriangles);
}

void CKDNode::AddTriangle(Vector v1, Vector v2, Vector v3, CConversionFace* pFace, CConversionMeshInstance* pMeshInstance)
{
	if (m_pTree->IsBuilt())
	{
		// Keep the tree built.

		if (!m_pLeft)
		{
			m_aTris.push_back(CKDTri(v1, v2, v3, pFace, pMeshInstance));
			m_iTriangles = m_aTris.size();
		}

		if (m_aTris.size() == MIN_TRIS_NODE+1)
		{
			// We just grew to 4 tris, so just call build since <= 3 tris hasn't been built before.
			TAssertNoMsg(!m_pLeft);
			Build();
			return;
		}

		if (!m_pLeft)
			return;

		if (TriangleIntersectsAABB(m_pLeft->m_oBounds, v1, v2, v3))
			m_pLeft->AddTriangle(v1, v2, v3, pFace, pMeshInstance);

		if (TriangleIntersectsAABB(m_pRight->m_oBounds, v1, v2, v3))
			m_pRight->AddTriangle(v1, v2, v3, pFace, pMeshInstance);
	}
	else
	{
		m_aTris.push_back();
		CKDTri* pTri = &m_aTris[m_aTris.size()-1];
		pTri->v[0] = v1;
		pTri->v[1] = v2;
		pTri->v[2] = v3;
		pTri->m_pFace = pFace;
		pTri->m_pMeshInstance = pMeshInstance;
	}

	if (m_pLeft)
		m_iTriangles = m_pLeft->m_iTriangles + m_pRight->m_iTriangles;
}

void CKDNode::RemoveArea(const AABB& oBox)
{
	if (GetBounds().Inside(oBox) && m_pLeft)
	{
		size_t iTrianglesDeleted = m_pLeft->m_iTriangles + m_pRight->m_iTriangles;

		CKDNode* pParent = m_pParent;
		while (pParent)
		{
			TAssertNoMsg(iTrianglesDeleted <= pParent->m_iTriangles);
			pParent->m_iTriangles -= iTrianglesDeleted;
			pParent = pParent->m_pParent;
		}

		m_iTriangles = 0;

		delete m_pLeft;
		delete m_pRight;
		m_pLeft = m_pRight = NULL;
		return;
	}

	if (!m_pLeft)
	{
		size_t iTrianglesDeleted = 0;
		for (int i = (int)m_aTris.size()-1; i >= 0; i--)
		{
			if (TriangleIntersectsAABB(oBox, m_aTris[i].v[0], m_aTris[i].v[1], m_aTris[i].v[2]))
			{
				m_aTris.erase(m_aTris.begin()+i);
				iTrianglesDeleted++;
			}
		}

		// If we're the parent then sometimes this is called when the tree isn't built yet and there's no problem then if the asserts fail.
		if (m_pParent)
		{
			TAssertNoMsg(iTrianglesDeleted == m_iTriangles - m_aTris.size());
			TAssertNoMsg(iTrianglesDeleted <= m_iTriangles);
		}

		m_iTriangles = m_aTris.size();
	}

	if (!m_pLeft)
		return;

	if (m_pLeft->GetBounds().Intersects(oBox))
		m_pLeft->RemoveArea(oBox);

	if (m_pRight->GetBounds().Intersects(oBox))
		m_pRight->RemoveArea(oBox);

	m_iTriangles = m_pLeft->m_iTriangles + m_pRight->m_iTriangles;

	if (m_iTriangles <= MIN_TRIS_NODE)
	{
		m_pLeft->PassTriList();
		m_pRight->PassTriList();
		delete m_pLeft;
		delete m_pRight;
		m_pLeft = m_pRight = NULL;
	}
}

void CKDNode::CalcBounds()
{
	if (!m_aTris.size())
	{
		m_oBounds = AABB();
		return;
	}

	// Initialize to the first value just so that it doesn't use the origin.
	// If it uses the origin and all of the tris have high values it will
	// wrongly include the origin in the bounds.
	Vector vecLowest(m_aTris[0].v[0]);
	Vector vecHighest(m_aTris[0].v[0]);

	for (size_t i = 0; i < m_aTris.size(); i++)
	{
		CKDTri* pTri = &m_aTris[i];

		for (size_t j = 0; j < 3; j++)
		{
			for (size_t k = 0; k < 3; k++)
			{
				float flValue = pTri->v[j][k];

				if (flValue < vecLowest[k])
					vecLowest[k] = flValue;

				if (flValue > vecHighest[k])
					vecHighest[k] = flValue;
			}
		}
	}

	if (vecLowest.x == vecHighest.x)
		vecHighest.x += 1e-4f;
	if (vecLowest.y == vecHighest.y)
		vecHighest.y += 1e-4f;
	if (vecLowest.z == vecHighest.z)
		vecHighest.z += 1e-4f;

	m_oBounds = AABB(vecLowest, vecHighest);
}

void CKDNode::BuildTriList()
{
	m_aTris.clear();

	if (m_pParent)
	{
		size_t iParentTris = m_pParent->m_aTris.size();

		m_aTris.reserve(iParentTris*3/5);

		// Copy all tris from my parent that intersect my bounding box
		for (size_t i = 0; i < iParentTris; i++)
		{
			CKDTri* oTri = &m_pParent->m_aTris[i];
			if (TriangleIntersectsAABB(m_oBounds, oTri->v[0], oTri->v[1], oTri->v[2]))
				m_aTris.push_back(m_pParent->m_aTris[i]);
		}

		m_aTris.set_capacity(m_aTris.size());
	}

	m_iTriangles = m_aTris.size();
}

void CKDNode::PassTriList()
{
	if (m_pLeft)
	{
		m_pLeft->PassTriList();
		m_pRight->PassTriList();
	}

	for (size_t i = 0; i < m_aTris.size(); i++)
		m_pParent->m_aTris.push_back(m_aTris[i]);

	m_aTris.clear();
}

void CKDNode::Build()
{
	// Find a better number!
	if (m_aTris.size() <= MIN_TRIS_NODE)
		return;

	if (m_iDepth > m_pTree->GetMaxDepth())
		return;

	Vector vecBoundsSize = m_oBounds.Size();

	// Find the largest axis.
	if (vecBoundsSize.x > vecBoundsSize.y)
		m_iSplitAxis = 0;
	else
		m_iSplitAxis = 1;
	if (vecBoundsSize.z > vecBoundsSize[m_iSplitAxis])
		m_iSplitAxis = 2;

	// Use better algorithm!
	m_flSplitPos = m_oBounds.m_vecMins[m_iSplitAxis] + vecBoundsSize[m_iSplitAxis]/2;

	AABB oBoundsLeft, oBoundsRight;
	oBoundsLeft = oBoundsRight = m_oBounds;

	oBoundsLeft.m_vecMaxs[m_iSplitAxis] = m_flSplitPos;
	oBoundsRight.m_vecMins[m_iSplitAxis] = m_flSplitPos;

	m_pLeft = new CKDNode(this, oBoundsLeft);
	m_pRight = new CKDNode(this, oBoundsRight);

	m_pLeft->BuildTriList();
	m_pRight->BuildTriList();

	m_aTris.clear();
	m_aTris.set_capacity(0);

	m_pLeft->Build();
	m_pRight->Build();

	m_iTriangles = m_pLeft->m_iTriangles + m_pRight->m_iTriangles;
}

bool CKDNode::Raytrace(const Ray& rayTrace, CTraceResult* pTR)
{
	CKDNode* pThis = this;

	while (pThis->m_pLeft)
	{
		bool bHitsLeft = RayIntersectsAABB(rayTrace, pThis->m_pLeft->m_oBounds);
		bool bHitsRight = RayIntersectsAABB(rayTrace, pThis->m_pRight->m_oBounds);

		// If it hit this node then it's got to hit one of our child nodes since both child nodes add up to this one.
		TAssertNoMsg(bHitsRight || bHitsLeft);

		if (bHitsLeft && !bHitsRight)
		{
			pThis = pThis->m_pLeft;
			continue;
		}

		if (bHitsRight && !bHitsLeft)
		{
			pThis = pThis->m_pRight;
			continue;
		}

		// Hit a poly in both cases, return the closer one.
		float flDistanceToLeft = (pThis->m_pLeft->m_oBounds.Center() - rayTrace.m_vecPos).LengthSqr();
		float flDistanceToRight = (pThis->m_pRight->m_oBounds.Center() - rayTrace.m_vecPos).LengthSqr();

		CKDNode* pCloser = pThis->m_pLeft;
		CKDNode* pFarther = pThis->m_pRight;
		if (flDistanceToRight < flDistanceToLeft)
		{
			pCloser = pThis->m_pRight;
			pFarther = pThis->m_pLeft;
		}

		if (pCloser->Raytrace(rayTrace, pTR))
			return true;
		else
		{
			pThis = pFarther;
			continue;
		}
	}

	TAssertNoMsg(!pThis->m_pLeft);

#ifdef DEBUG_WITH_GL
	DrawBox(pThis->m_oBounds, 0.6f);
#endif

	// No children. Test all triangles in this node.

	Vector vecClosest;
	CKDTri* pClosestTri;
	bool bFound = false;

	for (size_t i = 0; i < pThis->m_aTris.size(); i++)
	{
		CKDTri& oTri = pThis->m_aTris[i];

		Vector vecHit;
		if (RayIntersectsTriangle(rayTrace, oTri.v[0], oTri.v[1], oTri.v[2], &vecHit))
		{
			// Sometimes a try will touch a closer leaf node,
			// but actually intersect the ray in the back, so
			// only accept hits inside this box.
			if (!pThis->m_oBounds.Inside(vecHit))
			{
#ifdef DEBUG_WITH_GL
				DrawTri(oTri.v[0], oTri.v[1], oTri.v[2], 0.0f, 1.0f, 0.0f);
#endif
				continue;
			}

			if (!bFound || (vecHit - rayTrace.m_vecPos).LengthSqr() < (vecClosest - rayTrace.m_vecPos).LengthSqr())
			{
				bFound = true;
				vecClosest = vecHit;
				pClosestTri = &oTri;
			}

#ifdef DEBUG_WITH_GL
			DrawTri(oTri.v[0], oTri.v[1], oTri.v[2], 1.0f, 0.0f, 0.0f);
#endif
		}
#ifdef DEBUG_WITH_GL
		else
			DrawTri(oTri.v[0], oTri.v[1], oTri.v[2], 0.0f, 0.0f, 1.0f);
#endif
	}

	if (bFound)
	{
		if (pTR)
		{
			pTR->m_vecHit = vecClosest;
			pTR->m_pFace = pClosestTri->m_pFace;
			pTR->m_pMeshInstance = pClosestTri->m_pMeshInstance;
		}
		return true;
	}

	return false;
}

bool CKDNode::Raytrace(const Vector& vecStart, const Vector& vecEnd, CTraceResult* pTR)
{
	CKDNode* pThis = this;

	while (pThis->m_pLeft)
	{
		bool bHitsLeft = SegmentIntersectsAABB(vecStart, vecEnd, pThis->m_pLeft->m_oBounds);
		bool bHitsRight = SegmentIntersectsAABB(vecStart, vecEnd, pThis->m_pRight->m_oBounds);

#ifdef _DEBUG
		// If it hit this node then it's got to hit one of our child nodes since both child nodes add up to this one.
		TAssertNoMsg(bHitsRight || bHitsLeft);
#endif

		if (bHitsLeft && !bHitsRight)
		{
			pThis = pThis->m_pLeft;
			continue;
		}

		if (bHitsRight && !bHitsLeft)
		{
			pThis = pThis->m_pRight;
			continue;
		}

		// Hit a poly in both cases, return the closer one.
		float flDistanceToLeft = (pThis->m_pLeft->m_oBounds.Center() - vecStart).LengthSqr();
		float flDistanceToRight = (pThis->m_pRight->m_oBounds.Center() - vecStart).LengthSqr();

		CKDNode* pCloser = pThis->m_pLeft;
		CKDNode* pFarther = pThis->m_pRight;
		if (flDistanceToRight < flDistanceToLeft)
		{
			pCloser = pThis->m_pRight;
			pFarther = pThis->m_pLeft;
		}

		if (pCloser->Raytrace(vecStart, vecEnd, pTR))
			return true;
		else
		{
			pThis = pFarther;
			continue;
		}
	}

	TAssertNoMsg(!pThis->m_pLeft);

#ifdef DEBUG_WITH_GL
	DrawBox(m_oBounds, 0.6f);
#endif

	// No children. Test all triangles in this node.

	Vector vecClosest;
	CKDTri* pClosestTri;
	bool bFound = false;

	for (size_t i = 0; i < pThis->m_aTris.size(); i++)
	{
		CKDTri& oTri = pThis->m_aTris[i];

		Vector vecHit;
		if (LineSegmentIntersectsTriangle(vecStart, vecEnd, oTri.v[0], oTri.v[1], oTri.v[2], &vecHit))
		{
			// Sometimes a try will touch a closer leaf node,
			// but actually intersect the ray in the back, so
			// only accept hits inside this box.
			if (!pThis->m_oBounds.Inside(vecHit))
			{
#ifdef DEBUG_WITH_GL
				DrawTri(oTri.v[0], oTri.v[1], oTri.v[2], 0.0f, 1.0f, 0.0f);
#endif
				continue;
			}

			if (!bFound || (vecHit - vecStart).LengthSqr() < (vecClosest - vecStart).LengthSqr())
			{
				bFound = true;
				vecClosest = vecHit;
				pClosestTri = &oTri;
			}

#ifdef DEBUG_WITH_GL
			DrawTri(oTri.v[0], oTri.v[1], oTri.v[2], 1.0f, 0.0f, 0.0f);
#endif
		}
#ifdef DEBUG_WITH_GL
		else
			DrawTri(oTri.v[0], oTri.v[1], oTri.v[2], 0.0f, 0.0f, 1.0f);
#endif
	}

	if (bFound)
	{
		if (pTR)
		{
			pTR->m_vecHit = vecClosest;
			pTR->m_pFace = pClosestTri->m_pFace;
			pTR->m_pMeshInstance = pClosestTri->m_pMeshInstance;
		}
		return true;
	}

	return false;
}

float CKDNode::Closest(const Vector& vecPoint)
{
	float flClosest = -1;

	if (m_pLeft)
	{
		// Maybe it's on the line and hits both?
		bool bHitsLeft = m_pLeft->m_oBounds.Inside(vecPoint);
		bool bHitsRight = m_pRight->m_oBounds.Inside(vecPoint);

		if (!bHitsLeft && !bHitsRight)
		{
			if ((vecPoint - m_pLeft->m_oBounds.Center()).LengthSqr() < (vecPoint - m_pRight->m_oBounds.Center()).LengthSqr())
				flClosest = m_pLeft->Closest(vecPoint);
			else
				flClosest = m_pRight->Closest(vecPoint);
		}
		else if (bHitsLeft && !bHitsRight)
			flClosest = m_pLeft->Closest(vecPoint);
		else if (bHitsRight && !bHitsLeft)
			flClosest = m_pRight->Closest(vecPoint);
		else
		{
			// Hit a poly in both cases, return the closer one.
			float flLeft = m_pLeft->Closest(vecPoint);
			float flRight = m_pRight->Closest(vecPoint);

			if (flLeft < 0)
				flClosest = flRight;
			else if (flRight < 0)
				flClosest = flLeft;
			else
				flClosest = flLeft<flRight?flLeft:flRight;
		}
	}

	if (flClosest >= 0)
		return flClosest;

	// No children found anything. Test all triangles in this node.

	bool bFound = false;

	for (size_t i = 0; i < m_aTris.size(); i++)
	{
		CKDTri oTri = m_aTris[i];

		tvector<Vector> avecTri;
		avecTri.push_back(oTri.v[0]);
		avecTri.push_back(oTri.v[1]);
		avecTri.push_back(oTri.v[2]);

		float flDistance = DistanceToPolygon(vecPoint, avecTri, (oTri.v[0]-oTri.v[1]).Cross(oTri.v[0]-oTri.v[2]).Normalized());
		if (!bFound)
			flClosest = flDistance;
		else if (flDistance < flClosest)
			flClosest = flDistance;
		bFound = true;
	}

	if (bFound)
		return flClosest;
	else
		return -1;
}

CKDTri::CKDTri()
{
	// Empty for speed reasons. It'll get filled with values by whoever pushed it onto the vector.
}

CKDTri::CKDTri(Vector v1, Vector v2, Vector v3, CConversionFace* pFace, CConversionMeshInstance* pMeshInstance)
{
	v[0] = v1;
	v[1] = v2;
	v[2] = v3;
	m_pFace = pFace;
	m_pMeshInstance = pMeshInstance;
}

#ifdef DEBUG_WITH_GL
void DrawBox(AABB& b, float c)
{
	glLineWidth(2);
	Vector vecSize = b.Size();
	glColor3f(c, c, c);
	glBegin(GL_LINE_STRIP);
		glVertex3fv(b.m_vecMins);
		glVertex3fv(b.m_vecMins + Vector(0, 0, vecSize.z));
		glVertex3fv(b.m_vecMins + Vector(vecSize.x, 0, vecSize.z));
		glVertex3fv(b.m_vecMins + Vector(vecSize.x, 0, 0));
		glVertex3fv(b.m_vecMins);
	glEnd();
	glBegin(GL_LINE_STRIP);
		glVertex3fv(b.m_vecMaxs);
		glVertex3fv(b.m_vecMaxs - Vector(0, 0, vecSize.z));
		glVertex3fv(b.m_vecMaxs - Vector(vecSize.x, 0, vecSize.z));
		glVertex3fv(b.m_vecMaxs - Vector(vecSize.x, 0, 0));
		glVertex3fv(b.m_vecMaxs);
	glEnd();
	glBegin(GL_LINES);
		glVertex3fv(b.m_vecMins);
		glVertex3fv(b.m_vecMins + Vector(0, vecSize.y, 0));
		glVertex3fv(b.m_vecMins + Vector(0, 0, vecSize.z));
		glVertex3fv(b.m_vecMins + Vector(0, vecSize.y, vecSize.z));
		glVertex3fv(b.m_vecMins + Vector(vecSize.x, 0, vecSize.z));
		glVertex3fv(b.m_vecMins + Vector(vecSize.x, vecSize.y, vecSize.z));
		glVertex3fv(b.m_vecMins + Vector(vecSize.x, 0, 0));
		glVertex3fv(b.m_vecMins + Vector(vecSize.x, vecSize.y, 0));
	glEnd();
}

void DrawTri(Vector v1, Vector v2, Vector v3, float r, float g, float b)
{
	glLineWidth(3);
	glColor3f(r, g, b);
	glBegin(GL_LINE_LOOP);
		glVertex3fv(v1);
		glVertex3fv(v2);
		glVertex3fv(v3);
	glEnd();
}
#endif
