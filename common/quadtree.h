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

#ifndef LW_QUADTREE_H
#define LW_QUADTREE_H

#include <tvector.h>
#include <vector.h>

template <class T, typename F> class CQuadTree;
template <class T, typename F> class CQuadTreeBranch;

template <class T, typename F=float>
class CQuadTreeDataSource
{
public:
	// Quad trees aren't always flat, they're curvy and wonky sometimes. This function should translate a 3d coordinate to a quadtree coordinate
	// The output should be ([0, 1], [0, 1])
	// If the point is not in the quad tree then return a value less than 0 or greater than 1 for x or y
	virtual TemplateVector2D<F>		WorldToQuadTree(const CQuadTree<T, F>* pTree, const TemplateVector<F>& vecWorld) const=0;
	// This is the opposite conversion from the above.
	virtual TemplateVector<F>		QuadTreeToWorld(const CQuadTree<T, F>* pTree, const TemplateVector2D<F>& vecTree) const=0;

	virtual TemplateVector2D<F>		WorldToQuadTree(CQuadTree<T, F>* pTree, const TemplateVector<F>& vecWorld)=0;
	virtual TemplateVector<F>		QuadTreeToWorld(CQuadTree<T, F>* pTree, const TemplateVector2D<F>& vecTree)=0;

	virtual bool				ShouldBuildBranch(CQuadTreeBranch<T, F>* pBranch, bool& bDelete)=0;
};

template <class T, typename F=float>
class CQuadTreeBranch
{
public:
	CQuadTreeBranch(CQuadTreeDataSource<T, F>* pSource, CQuadTreeBranch<T, F>* pParent, CQuadTree<T, F>* pTree, TemplateVector2D<F> vecMin, TemplateVector2D<F> vecMax, unsigned short iDepth, const T& oData = T())
	{
		m_pDataSource = pSource;
		m_pParent = pParent;
		m_pTree = pTree;

		m_pBranches[0] = NULL;
		m_pBranches[1] = NULL;
		m_pBranches[2] = NULL;
		m_pBranches[3] = NULL;

		m_vecMin = vecMin;
		m_vecMax = vecMax;

		m_iDepth = iDepth;

		m_oData = oData;

		m_bCenterCalculated = false;
	}

public:
	void						BuildBranch(bool bAndChildren = true);

	void						InitPathfinding();
	void						FindNeighbors(const CQuadTreeBranch<T, F>* pLeaf, tvector<CQuadTreeBranch<T, F>*>& apNeighbors);
	CQuadTreeBranch<T, F>*		FindLeaf(const TemplateVector<F>& vecPoint);
	void						SetGScore(float flGScore);
	float						GetFScore();
	TemplateVector<F>			GetCenter();
	TemplateVector<F>			GetCenter() const;

	void						DebugRender();

public:
	CQuadTreeDataSource<T, F>*	m_pDataSource;
	CQuadTreeBranch<T, F>*		m_pParent;
	CQuadTree<T, F>*			m_pTree;

	TemplateVector2D<F>			m_vecMin;
	TemplateVector2D<F>			m_vecMax;

	unsigned short				m_iDepth;

	union
	{
		struct
		{
			CQuadTreeBranch<T, F>*	m_pBranchxy;
			CQuadTreeBranch<T, F>*	m_pBranchxY;
			CQuadTreeBranch<T, F>*	m_pBranchXy;
			CQuadTreeBranch<T, F>*	m_pBranchXY;
		};
		CQuadTreeBranch<T, F>*		m_pBranches[4];
	};

	T							m_oData;

	// Pathfinding stuff. SO not thread-safe!
	bool						m_bClosed;
	bool						m_bOpen;
	bool						m_bHCalculated;
	bool						m_bFValid;
	float						m_flGScore;
	float						m_flHScore;
	float						m_flFScore;
	bool						m_bCenterCalculated;
	TemplateVector<F>			m_vecCenter;

	CQuadTreeBranch<T, F>*		m_pPathParent;
};

template <class T, typename F=float>
class CQuadTree
{
public:
	CQuadTree()
	{
		m_pDataSource = NULL;
		m_pQuadTreeHead = NULL;
	};
	
	CQuadTree(CQuadTreeDataSource<T, F>* pSource, const T& oData)
	{
		Init(pSource, oData);
	};

public:
	// NOTE: There's no pathfinding code because this code was ripped from a previous project and the next project didn't need pathfinding
	// If you want pathfinding code you'll need to port it from terrain.cpp in the digitanks project

	void						Init(CQuadTreeDataSource<T, F>* pSource, const T& oData);

	class CQuadTreeBranch<T, F>*FindLeaf(const TemplateVector<F>& vecPoint);
	void						FindNeighbors(const CQuadTreeBranch<T, F>* pLeaf, tvector<CQuadTreeBranch<T, F>*>& apNeighbors);

protected:
	CQuadTreeDataSource<T, F>*	m_pDataSource;
	class CQuadTreeBranch<T, F>*m_pQuadTreeHead;
};

template <class T, typename F>
void CQuadTree<T, F>::Init(CQuadTreeDataSource<T, F>* pSource, const T& oData)
{
	m_pDataSource = pSource;
	m_pQuadTreeHead = new CQuadTreeBranch<T, F>(pSource, NULL, this, TemplateVector2D<F>(0, 0), TemplateVector2D<F>(1, 1), 0, oData);
}

template <class T, typename F>
CQuadTreeBranch<T, F>* CQuadTree<T, F>::FindLeaf(const TemplateVector<F>& vecPoint)
{
	if (!m_pQuadTreeHead)
		return NULL;

	if (!m_pDataSource)
		return NULL;

	TemplateVector2D<F> vecQuadTreePoint = m_pDataSource->WorldToQuadTree(vecPoint);

	if (vecQuadTreePoint.x < 0)
		return NULL;

	if (vecQuadTreePoint.y < 0)
		return NULL;

	if (vecQuadTreePoint.x > 1)
		return NULL;

	if (vecQuadTreePoint.y > 1)
		return NULL;

	CQuadTreeBranch<T, F>* pCurrent = m_pQuadTreeHead;
	while (pCurrent->m_pBranches[0])
	{
		for (size_t i = 0; i < 4; i++)
		{
			if (vecQuadTreePoint.x < pCurrent->m_pBranches[i]->m_vecMin.x)
				continue;

			if (vecQuadTreePoint.y < pCurrent->m_pBranches[i]->m_vecMin.y)
				continue;

			if (vecQuadTreePoint.x > pCurrent->m_pBranches[i]->m_vecMax.x)
				continue;

			if (vecQuadTreePoint.y > pCurrent->m_pBranches[i]->m_vecMax.y)
				continue;

			pCurrent = pCurrent->m_pBranches[i];
			break;
		}
	}

	return pCurrent;
}

template <class T, typename F>
void CQuadTree<T, F>::FindNeighbors(const CQuadTreeBranch<T, F>* pLeaf, tvector<CQuadTreeBranch<T, F>*>& apNeighbors)
{
	if (!pLeaf)
		return;

	apNeighbors.set_capacity(20);
	m_pQuadTreeHead->FindNeighbors(pLeaf, apNeighbors);
}

template <class T, typename F>
void CQuadTreeBranch<T, F>::BuildBranch(bool bAndChildren)
{
	bool bDelete;
	bool bBuildBranch = m_pDataSource->ShouldBuildBranch(this, bDelete);

	if (bBuildBranch)
	{
		if (!m_pBranches[0])
		{
			F flSize = (m_vecMax.x - m_vecMin.x)/2;
			m_pBranchxy = new CQuadTreeBranch<T, F>(m_pDataSource, this, m_pTree, m_vecMin + TemplateVector2D<F>(0, 0), m_vecMin + TemplateVector2D<F>(flSize, flSize), m_iDepth+1);
			m_pBranchxY = new CQuadTreeBranch<T, F>(m_pDataSource, this, m_pTree, m_vecMin + TemplateVector2D<F>(0, flSize), m_vecMin + TemplateVector2D<F>(flSize, flSize+flSize), m_iDepth+1);
			m_pBranchXy = new CQuadTreeBranch<T, F>(m_pDataSource, this, m_pTree, m_vecMin + TemplateVector2D<F>(flSize, 0), m_vecMin + TemplateVector2D<F>(flSize+flSize, flSize), m_iDepth+1);
			m_pBranchXY = new CQuadTreeBranch<T, F>(m_pDataSource, this, m_pTree, m_vecMin + TemplateVector2D<F>(flSize, flSize), m_vecMin + TemplateVector2D<F>(flSize+flSize, flSize+flSize), m_iDepth+1);
		}

		if (bAndChildren)
		{
			for (size_t i = 0; i < 4; i++)
				m_pBranches[i]->BuildBranch();
		}
	}
	else
	{
		if (m_pBranches[0] && bDelete)
		{
			for (size_t i = 0; i < 4; i++)
			{
				delete m_pBranches[i];
				m_pBranches[i] = NULL;
			}
		}
	}
}

template <class T, typename F>
void CQuadTreeBranch<T, F>::InitPathfinding()
{
	if (m_pBranches[0])
	{
		for (size_t i = 0; i < 4; i++)
			m_pBranches[i]->InitPathfinding();
	}
	else
	{
		m_bClosed = false;
		m_bOpen = false;
		m_bFValid = false;
		m_bHCalculated = false;
		m_flGScore = 0;
		m_pPathParent = NULL;
		m_bCenterCalculated = false;	// Re-calculated centers every pathfind in case of terrain height changes.
	}
}

template <class T, typename F>
void CQuadTreeBranch<T, F>::FindNeighbors(const CQuadTreeBranch<T, F>* pLeaf, tvector<CQuadTreeBranch<T, F>*>& apNeighbors)
{
	if (!pLeaf)
		return;

	if (m_pBranches[0])
	{
		for (size_t i = 0; i < 4; i++)
		{
			if (pLeaf->m_vecMin.x > m_pBranches[i]->m_vecMax.x)
				continue;

			if (pLeaf->m_vecMin.y > m_pBranches[i]->m_vecMax.y)
				continue;

			if (pLeaf->m_vecMax.x < m_pBranches[i]->m_vecMin.x)
				continue;

			if (pLeaf->m_vecMax.y < m_pBranches[i]->m_vecMin.y)
				continue;

			m_pBranches[i]->FindNeighbors(pLeaf, apNeighbors);
		}
	}
	else
	{
		if (pLeaf->m_vecMin.x > m_vecMax.x)
			return;

		if (pLeaf->m_vecMin.y > m_vecMax.y)
			return;

		if (pLeaf->m_vecMax.x < m_vecMin.x)
			return;

		if (pLeaf->m_vecMax.y < m_vecMin.y)
			return;

		apNeighbors.push_back(this);
	}
}

template <class T, typename F>
CQuadTreeBranch<T, F>* CQuadTreeBranch<T, F>::FindLeaf(const TemplateVector<F>& vecPoint)
{
	TemplateVector2D<F> vecQuadTreePoint = m_pDataSource->WorldToQuadTree(vecPoint);

	if (vecQuadTreePoint.x < m_vecMin.x)
		return NULL;

	if (vecQuadTreePoint.y < m_vecMin.y)
		return NULL;

	if (vecQuadTreePoint.x > m_vecMax.x)
		return NULL;

	if (vecQuadTreePoint.y > m_vecMax.y)
		return NULL;

	if (!m_pBranches[0])
		return this;

	for (size_t i = 0; i < 4; i++)
	{
		CQuadTreeBranch<T, F>* pResult = m_pBranches[i]->FindLeaf(vecPoint);
		if (pResult)
			return pResult;
	}

	TAssert(!"Should never get here.");
	return NULL;
}

template <class T, typename F>
void CQuadTreeBranch<T, F>::SetGScore(float flGScore)
{
	m_bFValid = false;
	m_flGScore = flGScore;
}

template <class T, typename F>
float CQuadTreeBranch<T, F>::GetFScore()
{
	if (m_bFValid)
		return m_flFScore;

	if (!m_bHCalculated)
	{
		m_flHScore = m_pTerrain->WeightedLeafDistance(this, m_pTerrain->m_pPathEnd, true);
		m_bHCalculated = true;
	}

	m_flFScore = m_flHScore + m_flGScore;
	m_bFValid = true;

	return m_flFScore;
}

template <class T, typename F>
TemplateVector<F> CQuadTreeBranch<T, F>::GetCenter()
{
	if (!m_bCenterCalculated)
	{
		m_vecCenter = (m_pDataSource->QuadTreeToWorld(m_pTree, m_vecMin) + m_pDataSource->QuadTreeToWorld(m_pTree, m_vecMax))/2;
		m_bCenterCalculated = true;
	}

	return m_vecCenter;
}

template <class T, typename F>
TemplateVector<F> CQuadTreeBranch<T, F>::GetCenter() const
{
	TAssert(m_bCenterCalculated);

	if (!m_bCenterCalculated)
		return (m_pDataSource->QuadTreeToWorld(m_pTree, m_vecMin) + m_pDataSource->QuadTreeToWorld(m_pTree, m_vecMax))/2;

	return m_vecCenter;
}

template <class T, typename F>
void CQuadTreeBranch<T, F>::DebugRender()
{
	if (m_pBranches[0])
	{
		for (size_t i = 0; i < 4; i++)
			m_pBranches[i]->DebugRender();
	}
	else
	{
		glBegin(GL_LINE_STRIP);
			glVertex3f(m_pTerrain->ArrayToWorldSpace(m_vecMin.x), m_pTerrain->GetRealHeight(m_vecMin.x, m_vecMin.y)+1, m_pTerrain->ArrayToWorldSpace(m_vecMin.y));
			glVertex3f(m_pTerrain->ArrayToWorldSpace(m_vecMax.x), m_pTerrain->GetRealHeight(m_vecMax.x, m_vecMin.y)+1, m_pTerrain->ArrayToWorldSpace(m_vecMin.y));
			glVertex3f(m_pTerrain->ArrayToWorldSpace(m_vecMax.x), m_pTerrain->GetRealHeight(m_vecMax.x, m_vecMax.y)+1, m_pTerrain->ArrayToWorldSpace(m_vecMax.y));
		glEnd();
	}
}

#endif
