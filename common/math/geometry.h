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

#ifndef LW_GEOMETRY_H
#define LW_GEOMETRY_H

#include "../common.h"
#include "../tstring.h"
#include "vector.h"
#include <tvector.h>

template <class T>
class TRect
{
public:
	TRect()
	{
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}

	TRect(T X, T Y, T W, T H)
	{
		x = X;
		y = Y;
		w = W;
		h = H;
	}

public:
	T		Size() const { return w*h; }

	T		Right() const { return x + w; }
	T		Bottom() const { return y + h; }

	void	SetRight(float r);
	void	SetBottom(float b);

	bool	Intersects(const TRect<T>& F) const;
	bool	Union(const TRect<T>& r);

public:
	T x, y, w, h;
};

template <class T>
void TRect<T>::SetRight(float r)
{
	w = r - x;
}

template <class T>
void TRect<T>::SetBottom(float b)
{
	h = b - y;
}

template <class T>
bool TRect<T>::Intersects(const TRect<T>& r) const
{
	if (x > r.Right())
		return false;

	if (r.x > Right())
		return false;

	if (y > r.Bottom())
		return false;

	if (r.y > Bottom())
		return false;

	return true;
}

template <class T>
bool TRect<T>::Union(const TRect<T>& r)
{
	if (!Intersects(r))
		return false;

	if (r.x > x)
	{
		T right = Right();
		x = r.x;
		SetRight(right);
	}

	if (r.y > y)
	{
		T bottom = Bottom();
		y = r.y;
		SetBottom(bottom);
	}

	if (Right() > r.Right())
		SetRight(r.Right());

	if (Bottom() > r.Bottom())
		SetBottom(r.Bottom());

	return true;
}

typedef TRect<int> Rect;
typedef TRect<float> FRect;

class Ray
{
public:
				Ray(Vector vecPos, Vector vecDir);

	Vector		m_vecPos;
	Vector		m_vecDir;
};

inline Ray::Ray(Vector vecPos, Vector vecDir)
{
	m_vecPos = vecPos;
	m_vecDir = vecDir;
}

class AABB
{
public:
				AABB() {};
				AABB(Vector vecMins, Vector vecMaxs);

public:
	Vector		Center() const;
	Vector		Size() const;

	bool		Inside(const AABB& oBox) const;
	bool		Inside(const Vector& vecPoint) const;
	bool		Inside2D(const Vector& vecPoint) const;
	bool		Intersects(const AABB& oBox) const;

	AABB		operator+(const AABB& oBox) const;
	AABB		operator*(float s) const;
	bool		operator==(const AABB& o) const;

public:
	Vector		m_vecMins;
	Vector		m_vecMaxs;
};

inline AABB::AABB(Vector vecMins, Vector vecMaxs)
{
	m_vecMins = vecMins;
	m_vecMaxs = vecMaxs;
}

inline Vector AABB::Center() const
{
	return (m_vecMins + m_vecMaxs)/2;
}

inline Vector AABB::Size() const
{
	return m_vecMaxs - m_vecMins;
}

inline bool AABB::Inside(const AABB& oBox) const
{
	if (m_vecMins.x < oBox.m_vecMins.x)
		return false;

	if (m_vecMins.y < oBox.m_vecMins.y)
		return false;

	if (m_vecMins.z < oBox.m_vecMins.z)
		return false;

	if (m_vecMins.x > oBox.m_vecMins.x)
		return false;

	if (m_vecMins.y > oBox.m_vecMins.y)
		return false;

	if (m_vecMins.z > oBox.m_vecMins.z)
		return false;

	return true;
}

inline bool AABB::Inside(const Vector& vecPoint) const
{
	const float flEpsilon = 1e-4f;

	for (size_t i = 0; i < 3; i++)
	{
		float flVI = vecPoint[i];

		if (flVI < m_vecMins[i] - flEpsilon || flVI > m_vecMaxs[i] + flEpsilon)
			return false;
	}

	return true;
}

inline bool AABB::Inside2D(const Vector& vecPoint) const
{
	const float flEpsilon = 1e-4f;

	if (vecPoint.x < m_vecMins.x - flEpsilon || vecPoint.x > m_vecMaxs.x + flEpsilon)
		return false;

	if (vecPoint.z < m_vecMins.z - flEpsilon || vecPoint.z > m_vecMaxs.z + flEpsilon)
		return false;

	return true;
}

inline bool AABB::Intersects(const AABB& oBox) const
{
	if (m_vecMins.x > oBox.m_vecMaxs.x)
		return false;

	if (oBox.m_vecMins.x > m_vecMaxs.x)
		return false;

	if (m_vecMins.y > oBox.m_vecMaxs.y)
		return false;

	if (oBox.m_vecMins.y > m_vecMaxs.y)
		return false;

	if (m_vecMins.z > oBox.m_vecMaxs.z)
		return false;

	if (oBox.m_vecMins.z > m_vecMaxs.z)
		return false;

	return true;
}

inline AABB AABB::operator+(const AABB& oBox) const
{
	AABB r(*this);

	r.m_vecMaxs += oBox.m_vecMaxs;
	r.m_vecMins += oBox.m_vecMins;

	return r;
}

inline AABB AABB::operator*(float s) const
{
	AABB r(*this);

	r.m_vecMaxs *= s;
	r.m_vecMins *= s;

	return r;
}

inline bool AABB::operator==(const AABB& o) const
{
	return (m_vecMins == o.m_vecMins) && (m_vecMaxs == o.m_vecMaxs);
}

// Geometry-related functions

inline bool SameSide(const Vector& p1, const Vector& p2, const Vector& a, const Vector& b)
{
	Vector ba = b-a;
    Vector cp1 = ba.Cross(p1-a);
    Vector cp2 = ba.Cross(p2-a);
    return (cp1.Dot(cp2) > 0);
}

inline bool PointInTriangle(const Vector& p, const Vector& a, const Vector& b, const Vector& c)
{
	return (SameSide(p, a, b, c) && SameSide(p, b, a, c) && SameSide(p, c, a, b));
}

inline float DistanceToLine(const Vector& p, const Vector& v1, const Vector& v2)
{
	Vector v = v2 - v1;
	Vector w = p - v1;

	float c1 = w.Dot(v);
	float c2 = v.Dot(v);

	float b = c1/c2;

	Vector vb = v1 + v*b;
	return (vb - p).Length();
}

inline float DistanceToLineSegment(const Vector& p, const Vector& v1, const Vector& v2, Vector* i = NULL)
{
	Vector v = v2 - v1;
	Vector w = p - v1;

	float c1 = w.Dot(v);

	if (c1 < 0)
	{
		if (i)
			*i = v1;
		return (v1-p).Length();
	}

	float c2 = v.Dot(v);
	if (c2 < c1)
	{
		if (i)
			*i = v2;
		return (v2-p).Length();
	}

	if (c2 < 0.001f)
		return 0;

	float b = c1/c2;

	Vector vb = v1 + v*b;

	if (i)
		*i = vb;

	return (vb - p).Length();
}

inline float DistanceToPlane(const Vector& p, const Vector& v, const Vector& n)
{
	float sb, sn, sd;

	sn = -n.Dot(p - v);
	sd = n.Dot(n);
	sb = sn/sd;

	Vector b = p + n * sb;
	return (p - b).Length();
}

inline float DistanceToPolygon(const Vector& p, tvector<Vector>& v, Vector n)
{
	float flPlaneDistance = DistanceToPlane(p, v[0], n);

	size_t i;

	bool bFoundPoint = false;

	for (i = 0; i < v.size()-2; i++)
	{
		if (PointInTriangle(p, v[0], v[i+1], v[i+2]))
		{
			bFoundPoint = true;
			break;
		}
	}

	if (bFoundPoint)
		return flPlaneDistance;

	float flClosestPoint = -1;
	for (i = 0; i < v.size(); i++)
	{
		float flPointDistance = (v[i] - p).Length();
		if (flClosestPoint == -1 || (flPointDistance < flClosestPoint))
			flClosestPoint = flPointDistance;

		float flLineDistance;
		if (i == v.size() - 1)
			flLineDistance = DistanceToLineSegment(p, v[i], v[0]);
		else
			flLineDistance = DistanceToLineSegment(p, v[i], v[i+1]);

		if (flClosestPoint == -1 || (flLineDistance < flClosestPoint))
			flClosestPoint = flLineDistance;
	}

	return flClosestPoint;
}

inline float TriangleArea(const Vector& a, const Vector& b, const Vector& c)
{
	return (a-b).Cross(a-c).Length()/2;
}

inline bool RayIntersectsTriangle(const Ray& vecRay, const Vector& v0, const Vector& v1, const Vector& v2, Vector* pvecHit = NULL)
{
	Vector u = v1 - v0;
	Vector v = v2 - v0;
	Vector n = u.Cross(v);

	Vector w0 = vecRay.m_vecPos - v0;

	float a = -n.Dot(w0);
	float b = n.Dot(vecRay.m_vecDir);

	float ep = 1e-4f;

	if (fabs(b) < ep)
	{
		if (a == 0)			// Ray is parallel
			return false;	// Ray is inside plane
		else
			return false;	// Ray is somewhere else
	}

	float r = a/b;
	if (r < 0)
		return false;		// Ray goes away from the triangle

	Vector vecPoint = vecRay.m_vecPos + vecRay.m_vecDir*r;
	if (pvecHit)
		*pvecHit = vecPoint;

	float uu = u.Dot(u);
	float uv = u.Dot(v);
	float vv = v.Dot(v);
	Vector w = vecPoint - v0;
	float wu = w.Dot(u);
	float wv = w.Dot(v);

	float D = uv * uv - uu * vv;

	float s, t;

	s = (uv * wv - vv * wu) / D;
	if (s < 0 || s > 1)		// Intersection point is outside the triangle
		return false;

	t = (uv * wu - uu * wv) / D;
	if (t < 0 || (s+t) > 1)	// Intersection point is outside the triangle
		return false;

	return true;
}

inline bool RayIntersectsPlane(const Ray& vecRay, const Vector& v0, const Vector& v1, const Vector& v2, Vector* pvecHit = NULL)
{
	Vector u = v1 - v0;
	Vector v = v2 - v0;
	Vector n = u.Cross(v);

	Vector w0 = vecRay.m_vecPos - v0;

	float a = -n.Dot(w0);
	float b = n.Dot(vecRay.m_vecDir);

	float ep = 1e-4f;

	if (fabs(b) < ep)
	{
		if (a == 0)			// Ray is parallel
			return false;	// Ray is inside plane
		else
			return false;	// Ray is somewhere else
	}

	float r = a/b;
	if (r < 0)
		return false;		// Ray goes away from the plane

	Vector vecPoint = vecRay.m_vecPos + vecRay.m_vecDir*r;
	if (pvecHit)
		*pvecHit = vecPoint;

	return true;
}

inline bool RayIntersectsPlane(const Ray& vecRay, const Vector& p, const Vector& n, Vector* pvecHit = NULL)
{
	Vector w0 = vecRay.m_vecPos - p;

	float a = -n.Dot(w0);
	float b = n.Dot(vecRay.m_vecDir);

	float ep = 1e-4f;

	if (fabs(b) < ep)
	{
		if (a == 0)			// Ray is parallel
			return false;	// Ray is inside plane
		else
			return false;	// Ray is somewhere else
	}

	float r = a/b;
	if (r < 0)
		return false;		// Ray goes away from the plane

	Vector vecPoint = vecRay.m_vecPos + vecRay.m_vecDir*r;
	if (pvecHit)
		*pvecHit = vecPoint;

	return true;
}

inline bool ClipRay(float flMin, float flMax, float a, float d, float& tmin, float& tmax)
{
	const float flEpsilon = 1e-5f;

	if (fabs(d) < flEpsilon)
	{
		if (d >= 0.0f)
			return (a <= flMax);
		else
			return (a >= flMin);
	}

	float umin = (flMin - a)/d;
	float umax = (flMax - a)/d;

	if (umin > umax)
	{
		float yar = umin;
		umin = umax;
		umax = yar;
	}

	if (umax < tmin || umin > tmax)
		return false;

	tmin = (umin>tmin)?umin:tmin;
	tmax = (umax<tmax)?umax:tmax;

	return (tmax>tmin);
}

inline bool RayIntersectsAABB(const Ray& r, const AABB& b)
{
	float tmin = 0;
	float tmax = b.Size().LengthSqr();	// It's a ray so make tmax effectively infinite.
	if (tmax < 1)
		tmax = 100;
	float flDistToBox = (r.m_vecPos - b.Center()).LengthSqr();
	if (flDistToBox < 1)
		flDistToBox = 100;
	tmax *= flDistToBox * 100;

	if (!ClipRay(b.m_vecMins.x, b.m_vecMaxs.x, r.m_vecPos.x, r.m_vecDir.x, tmin, tmax))
		return false;

	if (!ClipRay(b.m_vecMins.y, b.m_vecMaxs.y, r.m_vecPos.y, r.m_vecDir.y, tmin, tmax))
		return false;

	if (!ClipRay(b.m_vecMins.z, b.m_vecMaxs.z, r.m_vecPos.z, r.m_vecDir.z, tmin, tmax))
		return false;

	return true;
}

inline bool ClipSegment(float flMin, float flMax, float a, float b, float d, float& tmin, float& tmax)
{
	const float flEpsilon = 1e-5f;

	if (fabs(d) < flEpsilon)
	{
		if (d >= 0.0f)
			return !(b < flMin || a > flMax);
		else
			return !(a < flMin || b > flMax);
	}

	float umin = (flMin - a)/d;
	float umax = (flMax - a)/d;

	if (umin > umax)
	{
		float yar = umin;
		umin = umax;
		umax = yar;
	}

	if (umax < tmin || umin > tmax)
		return false;

	tmin = (umin>tmin)?umin:tmin;
	tmax = (umax<tmax)?umax:tmax;

	return (tmax>tmin);
}

inline bool SegmentIntersectsAABB(const Vector& v1, const Vector& v2, const AABB& b)
{
	float tmin = 0;
	float tmax = 1;

	Vector vecDir = v2 - v1;

	if (!ClipSegment(b.m_vecMins.x, b.m_vecMaxs.x, v1.x, v2.x, vecDir.x, tmin, tmax))
		return false;

	if (!ClipSegment(b.m_vecMins.y, b.m_vecMaxs.y, v1.y, v2.y, vecDir.y, tmin, tmax))
		return false;

	if (!ClipSegment(b.m_vecMins.z, b.m_vecMaxs.z, v1.z, v2.z, vecDir.z, tmin, tmax))
		return false;

	return true;
}

inline bool LineSegmentIntersectsTriangle(const Vector& s0, const Vector& s1, const Vector& v0, const Vector& v1, const Vector& v2, Vector* pvecHit = NULL)
{
	Vector u = v1 - v0;
	Vector v = v2 - v0;
	Vector n = u.Cross(v);

	Vector w0 = s0 - v0;

	float a = -n.Dot(w0);
	float b = n.Dot(s1-s0);

	float ep = 1e-4f;

	if (fabs(b) < ep)
	{
		if (a == 0)			// Segment is parallel
			return true;	// Segment is inside plane
		else
			return false;	// Segment is somewhere else
	}

	float r = a/b;
	if (r < 0)
		return false;		// Segment goes away from the triangle
	if (r > 1)
		return false;		// Segment goes away from the triangle

	Vector vecPoint = s0 + (s1-s0)*r;
	if (pvecHit)
		*pvecHit = vecPoint;

	float uu = u.Dot(u);
	float uv = u.Dot(v);
	float vv = v.Dot(v);
	Vector w = vecPoint - v0;
	float wu = w.Dot(u);
	float wv = w.Dot(v);

	float D = uv * uv - uu * vv;

	float s, t;

	s = (uv * wv - vv * wu) / D;
	if (s <= ep || s >= 1)		// Intersection point is outside the triangle
		return false;

	t = (uv * wu - uu * wv) / D;
	if (t <= ep || (s+t) >= 1)	// Intersection point is outside the triangle
		return false;

	return true;
}

inline bool LineSegmentIntersectsSphere(const Vector& v1, const Vector& v2, const Vector& s, float flRadius, Vector& vecPoint, Vector& vecNormal)
{
	Vector vecLine = v2 - v1;
	Vector vecSphere = v1 - s;

	float flA = vecLine.LengthSqr();
	float flB = 2 * vecSphere.Dot(vecLine);
	float flC1 = s.LengthSqr() + v1.LengthSqr();
	float flC2 = (s.Dot(v1)*2);
	float flC = flC1 - flC2 - flRadius*flRadius;

	float flBB4AC = flB*flB - 4*flA*flC;
	if (flBB4AC < 0)
		return false;

	float flSqrt = sqrt(flBB4AC);
	float flPlus = (-flB + flSqrt)/(2*flA);
	float flMinus = (-flB - flSqrt)/(2*flA);

	return false;
	// Unimplemented: Doesn't clip the intersection to the segment only.
	// Also probably buggy and fully of bugs.
	// See sp_common.cpp for a better implementation that's actually used.

	float flDistance = vecLine.Length();

	Vector vecDirection = vecLine / flDistance;
	Vector vecPlus = v1 + vecDirection * (flPlus * flDistance);
	Vector vecMinus = v1 + vecDirection * (flMinus * flDistance);

	if ((vecPlus - v1).LengthSqr() < (vecMinus - v1).LengthSqr())
		vecPoint = vecPlus;
	else
		vecPoint = vecMinus;

	return true;
}

inline bool	TriangleIntersectsAABB(const AABB& oBox, const Vector& v0, const Vector& v1, const Vector& v2)
{
	// Trivial case rejection: If any of the points are inside the box, return true immediately.
	if (oBox.Inside(v0))
		return true;
	if (oBox.Inside(v1))
		return true;
	if (oBox.Inside(v2))
		return true;

	size_t i;

	// Trivial case rejection: If all three points are on one side of the box then the triangle must be outside of it.
	for (i = 0; i < 3; i++)
	{
		float flBoxMax = oBox.m_vecMaxs[i];
		float flBoxMin = oBox.m_vecMins[i];

		float flV0 = v0[i];
		float flV1 = v1[i];
		float flV2 = v2[i];

		if (flV0 > flBoxMax && flV1 > flBoxMax && flV2 > flBoxMax)
			return false;
		if (flV0 < flBoxMin && flV1 < flBoxMin && flV2 < flBoxMin)
			return false;
	}

	if (SegmentIntersectsAABB(v0, v1, oBox))
		return true;

	if (SegmentIntersectsAABB(v1, v2, oBox))
		return true;

	if (SegmentIntersectsAABB(v0, v2, oBox))
		return true;

	Vector c0 = oBox.m_vecMins;
	Vector c1 = Vector(oBox.m_vecMins.x, oBox.m_vecMins.y, oBox.m_vecMaxs.z);
	Vector c2 = Vector(oBox.m_vecMins.x, oBox.m_vecMaxs.y, oBox.m_vecMins.z);
	Vector c3 = Vector(oBox.m_vecMins.x, oBox.m_vecMaxs.y, oBox.m_vecMaxs.z);
	Vector c4 = Vector(oBox.m_vecMaxs.x, oBox.m_vecMins.y, oBox.m_vecMins.z);
	Vector c5 = Vector(oBox.m_vecMaxs.x, oBox.m_vecMins.y, oBox.m_vecMaxs.z);
	Vector c6 = Vector(oBox.m_vecMaxs.x, oBox.m_vecMaxs.y, oBox.m_vecMins.z);
	Vector c7 = oBox.m_vecMaxs;

	// Build a list of line segments in the cube to test against the triangle.
	Vector aLines[32];

	// Bottom four
	aLines[0] = c0;
	aLines[1] = c1;

	aLines[2] = c1;
	aLines[3] = c2;

	aLines[4] = c2;
	aLines[5] = c3;

	aLines[6] = c3;
	aLines[7] = c0;

	// Sides
	aLines[8] = c0;
	aLines[9] = c4;

	aLines[10] = c1;
	aLines[11] = c5;

	aLines[12] = c2;
	aLines[13] = c6;

	aLines[14] = c3;
	aLines[15] = c7;

	// Top
	aLines[16] = c4;
	aLines[17] = c5;

	aLines[18] = c5;
	aLines[19] = c6;

	aLines[20] = c6;
	aLines[21] = c7;

	aLines[22] = c7;
	aLines[23] = c4;

	// Diagonals
	aLines[24] = c0;
	aLines[25] = c6;

	aLines[26] = c1;
	aLines[27] = c7;

	aLines[28] = c2;
	aLines[29] = c4;

	aLines[30] = c3;
	aLines[31] = c5;

	// If any of the segments intersects with the triangle then we have a winner.
	for (i = 0; i < 32; i+=2)
	{
		if (LineSegmentIntersectsTriangle(aLines[i], aLines[i+1], v0, v1, v2))
			return true;
	}

	return false;
}

inline bool	ConvexHullIntersectsAABB(const AABB& oBox, const tvector<Vector>& avecPoints, const tvector<size_t>& aiTriangles)
{
	TAssertNoMsg(aiTriangles.size()%3 == 0);

	Vector vecCenter = oBox.Center();
	Vector n;

	for (size_t i = 0; i < aiTriangles.size(); i += 3)
	{
		const Vector& v1 = avecPoints[aiTriangles[i]];
		const Vector& v2 = avecPoints[aiTriangles[i+1]];
		const Vector& v3 = avecPoints[aiTriangles[i+2]];

		n = (v2-v1).Cross(v3-v1).Normalized();

		if (n.Dot(vecCenter-v1) < 0)
			continue;

		if (!TriangleIntersectsAABB(oBox, v1, v2, v3))
			return false;
	}

	return true;
}

inline size_t FindEar(const tvector<Vector>& avecPoints)
{
	size_t iPoints = avecPoints.size();

	// A triangle is always an ear.
	if (iPoints <= 3)
		return 0;

	size_t i;

	Vector vecFaceNormal;

	// Calculate the face normal.
	for (i = 0; i < iPoints; i++)
	{
		size_t iNext = (i+1)%iPoints;

		Vector vecPoint = avecPoints[i];
		Vector vecNextPoint = avecPoints[iNext];

		vecFaceNormal.x += (vecPoint.y - vecNextPoint.y) * (vecPoint.z + vecNextPoint.z);
		vecFaceNormal.y += (vecPoint.z - vecNextPoint.z) * (vecPoint.x + vecNextPoint.x);
		vecFaceNormal.z += (vecPoint.x - vecNextPoint.x) * (vecPoint.y + vecNextPoint.y);
	}

	vecFaceNormal.Normalize();

	for (i = 0; i < iPoints; i++)
	{
		size_t iLast = i==0?iPoints-1:i-1;
		size_t iNext = i==iPoints-1?0:i+1;

		Vector vecLast = avecPoints[iLast];
		Vector vecThis = avecPoints[i];
		Vector vecNext = avecPoints[iNext];

		// Concave ones can not be ears.
		if ((vecLast-vecThis).Cross(vecLast-vecNext).Dot(vecFaceNormal) < 0)
			continue;

		bool bFoundPoint = false;
		for (size_t j = 0; j < iPoints; j++)
		{
			if (j == i || j == iLast || j == iNext)
				continue;

			if (PointInTriangle(avecPoints[j], vecLast, vecThis, vecNext))
			{
				bFoundPoint = true;
				break;
			}
		}

		if (!bFoundPoint)
			return i;
	}

	return 0;
}

inline void FindLaunchVelocity(const Vector& vecOrigin, const Vector& vecTarget, float flGravity, Vector& vecForce, float& flTime, float flCurve = -0.03f)
{
	Vector vecDistance = vecTarget - vecOrigin;
	float flY = vecDistance.y;
	vecDistance.y = 0;
	float flX = vecDistance.Length();

	float flA = flCurve;
	float flH = (flX*flX - (flY/flA))/(2*flX);
	float flK = -flA*flH*flH;
	float flB = -2*flH*flA;

	float flForce = sqrt(2*-flGravity*flK);

	float flTimeToVertex = -flForce/flGravity;
	float flTimeToLand = sqrt(2*-(flK-flY)/flGravity);

	flTime = flTimeToVertex + flTimeToLand;

	Vector vecDirection = vecDistance.Normalized() * flX / flTime;
	vecDirection.y = flForce;

	vecForce = vecDirection;
}

#endif