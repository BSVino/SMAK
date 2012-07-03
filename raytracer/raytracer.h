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

#ifndef LW_RAYTRACE_H
#define LW_RAYTRACE_H

#include <modelconverter/convmesh.h>
#include <geometry.h>

namespace raytrace {

class CTraceResult
{
public:
	Vector						m_vecHit;
	size_t						m_iFace;
	CConversionMeshInstance*	m_pMeshInstance;
};

class CKDTri
{
public:
								CKDTri();
								CKDTri(size_t v1, size_t v2, size_t v3, size_t iMeshInstanceVertsIndex, size_t iFace, CConversionMeshInstance* pMeshInstance = nullptr);

public:
	size_t						v[3];

	size_t						m_iMeshInstanceVertsIndex;
	size_t						m_iFace;
	CConversionMeshInstance*	m_pMeshInstance;
};

class CKDNode
{
public:
								CKDNode(CKDNode* pParent = NULL, AABB oBounds = AABB(), class CKDTree* pTree = NULL);
								~CKDNode();

public:
	// Reserves memory for triangles all at once, for faster allocation
	void						ReserveTriangles(size_t iEstimatedTriangles);
	void						AddTriangle(size_t v1, size_t v2, size_t v3, size_t iMeshInstanceVertsIndex, size_t iFace, CConversionMeshInstance* pMeshInstance);

	void						RemoveArea(const AABB& oBox);

	void						CalcBounds();
	void						BuildTriList();
	void						PassTriList();
	void						Build();

	bool						Raytrace(const Ray& rayTrace, CTraceResult* pTR = NULL);
	bool						Raytrace(const Vector& vecStart, const Vector& vecEnd, CTraceResult* pTR = NULL);
	float						Closest(const Vector& vecPoint);

	const CKDNode*				GetLeftChild() const { return m_pLeft; };
	const CKDNode*				GetRightChild() const { return m_pRight; };
	AABB						GetBounds() const { return m_oBounds; };
	size_t						GetSplitAxis() const { return m_iSplitAxis; };
	float						GetSplitPos() const { return m_flSplitPos; };

protected:
	CKDNode*					m_pParent;

	CKDNode*					m_pLeft;
	CKDNode*					m_pRight;

	CKDTree*					m_pTree;

	size_t						m_iDepth;

	tvector<CKDTri>				m_aTris;
	size_t						m_iTriangles;	// This node and all child nodes

	AABB						m_oBounds;

	size_t						m_iSplitAxis;
	float						m_flSplitPos;
};

class CKDTree
{
	friend class CKDNode;

public:
								CKDTree(class CRaytracer* pTracer, size_t iMaxDepth = 15);
								~CKDTree();

public:
	// Reserves memory for triangles all at once, for faster allocation
	void						ReserveTriangles(size_t iEstimatedTriangles);
	void						AddTriangle(size_t v1, size_t v2, size_t v3, size_t iMeshInstanceVertsIndex, size_t iFace, CConversionMeshInstance* pMeshInstance);

	void						RemoveArea(const AABB& oBox);

	void						BuildTree();

	bool						Raytrace(const Ray& rayTrace, CTraceResult* pTR = NULL);
	bool						Raytrace(const Vector& vecStart, const Vector& vecEnd, CTraceResult* pTR = NULL);
	float						Closest(const Vector& vecPoint);

	const CKDNode*				GetTopNode() const { return m_pTop; };

	bool						IsBuilt() { return m_bBuilt; };

	size_t						GetMaxDepth() { return m_iMaxDepth; };

protected:
	class CRaytracer*			m_pRaytracer;
	CKDNode*					m_pTop;

	bool						m_bBuilt;

	size_t						m_iMaxDepth;
};

class CRaytracer
{
	friend class CKDNode;

public:
								CRaytracer(CConversionScene* pScene = NULL, size_t iMaxDepth = 15);
								~CRaytracer();

public:
	bool						Raytrace(const Ray& rayTrace, CTraceResult* pTR = NULL);
	bool						Raytrace(const Vector& vecStart, const Vector& vecEnd, CTraceResult* pTR = NULL);
	bool						RaytraceBruteForce(const Ray& rayTrace, CTraceResult* pTR = NULL);

	float						Closest(const Vector& vecPoint);

	void						AddMeshesFromNode(CConversionSceneNode* pNode);
	void						AddMeshInstance(CConversionMeshInstance* pMeshInstance);
	void						BuildTree();

	void						RemoveArea(const AABB& oBox);

	const CKDTree*				GetTree() const { return m_pTree; };

protected:
	CConversionScene*			m_pScene;

	CKDTree*					m_pTree;

	size_t						m_iMaxDepth;

	tvector<tvector<Vector>>	m_aaMeshInstanceVerts;
};

};

#endif