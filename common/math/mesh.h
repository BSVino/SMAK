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

#pragma once

#include <tmap.h>
#include <tvector.h>
#include "vector.h"

class CConvexHullGenerator
{
public:
	CConvexHullGenerator(const tvector<Vector>& avecPoints);

public:
	const tvector<size_t>&	GetConvexTriangles();

protected:
	void			CreateConvex();
	size_t			FindLowestPoint();
	size_t			FindNextPoint(size_t p1, size_t p2);

	bool			EdgeExists(size_t p1, size_t p2);
	void			AddEdge(size_t p1, size_t p2);

protected:
	struct EdgePair
	{
		EdgePair(size_t P1, size_t P2)
		{
			p1 = P1;
			p2 = P2;
		}

		size_t p1;
		size_t p2;
	};

	const tvector<Vector>&		m_avecPoints;
	tvector<size_t>				m_aiTriangles;

	tvector<EdgePair>			m_aOpenEdges;
	tmap<size_t, tmap<size_t, bool> >	m_aaCreatedEdges;
};

// Removes verts in a mesh which are coplanar
class CCoplanarPointOptimizer
{
public:
	// Does not remove the optimized verts. See CUnusedPointOptimizer
	static void						OptimizeMesh(const tvector<Vector>& avecPoints, tvector<size_t>& aiTriangles, float flTolerance = 0.001f);
};

// Removes unused points from a mesh, ie points that are not referred to by any triangle.
class CUnusedPointOptimizer
{
public:
	static void						OptimizeMesh(tvector<Vector>& avecPoints, tvector<size_t>& aiTriangles);
};
