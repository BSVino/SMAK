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

#include "frustum.h"

void Frustum::CreateFrom(const Matrix4x4& m)
{
	p[FRUSTUM_RIGHT].n.x = m.m[0][3] - m.m[0][0];
	p[FRUSTUM_RIGHT].n.y = m.m[1][3] - m.m[1][0];
	p[FRUSTUM_RIGHT].n.z = m.m[2][3] - m.m[2][0];
	p[FRUSTUM_RIGHT].d = m.m[3][3] - m.m[3][0];

	p[FRUSTUM_LEFT].n.x = m.m[0][3] + m.m[0][0];
	p[FRUSTUM_LEFT].n.y = m.m[1][3] + m.m[1][0];
	p[FRUSTUM_LEFT].n.z = m.m[2][3] + m.m[2][0];
	p[FRUSTUM_LEFT].d = m.m[3][3] + m.m[3][0];

	p[FRUSTUM_DOWN].n.x = m.m[0][3] + m.m[0][1];
	p[FRUSTUM_DOWN].n.y = m.m[1][3] + m.m[1][1];
	p[FRUSTUM_DOWN].n.z = m.m[2][3] + m.m[2][1];
	p[FRUSTUM_DOWN].d = m.m[3][3] + m.m[3][1];

	p[FRUSTUM_UP].n.x = m.m[0][3] - m.m[0][1];
	p[FRUSTUM_UP].n.y = m.m[1][3] - m.m[1][1];
	p[FRUSTUM_UP].n.z = m.m[2][3] - m.m[2][1];
	p[FRUSTUM_UP].d = m.m[3][3] - m.m[3][1];

	p[FRUSTUM_FAR].n.x = m.m[0][3] - m.m[0][2];
	p[FRUSTUM_FAR].n.y = m.m[1][3] - m.m[1][2];
	p[FRUSTUM_FAR].n.z = m.m[2][3] - m.m[2][2];
	p[FRUSTUM_FAR].d = m.m[3][3] - m.m[3][2];

	p[FRUSTUM_NEAR].n.x = m.m[0][3] + m.m[0][2];
	p[FRUSTUM_NEAR].n.y = m.m[1][3] + m.m[1][2];
	p[FRUSTUM_NEAR].n.z = m.m[2][3] + m.m[2][2];
	p[FRUSTUM_NEAR].d = m.m[3][3] + m.m[3][2];

	// Normalize all plane normals
	for(int i = 0; i < 6; i++)
		p[i].Normalize();
}

bool Frustum::TouchesSphere(const Vector& vecCenter, float flRadius)
{
	for (size_t i = 0; i < 6; i++)
	{
		float flDistance = p[i].n.x*vecCenter.x + p[i].n.y*vecCenter.y + p[i].n.z*vecCenter.z + p[i].d;
		if (flDistance + flRadius < 0)
			return false;
	}

	return true;
}

bool Frustum::TouchesSphereSidesOnly(const Vector& vecCenter, float flRadius)
{
	for (size_t i = 2; i < 6; i++)
	{
		float flDistance = p[i].n.x*vecCenter.x + p[i].n.y*vecCenter.y + p[i].n.z*vecCenter.z + p[i].d;
		if (flDistance + flRadius < 0)
			return false;
	}

	return true;
}

bool Frustum::ContainsSphereSidesOnly(const Vector& vecCenter, float flRadius)
{
	for (size_t i = 2; i < 6; i++)
	{
		float flDistance = p[i].n.x*vecCenter.x + p[i].n.y*vecCenter.y + p[i].n.z*vecCenter.z + p[i].d;
		if (flDistance + flRadius < 0)
			return false;

		if (flDistance < flRadius || -flDistance < flRadius)
			return false;
	}

	return true;
}
