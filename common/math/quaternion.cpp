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

#include "quaternion.h"

#include "vector.h"
#include "matrix.h"

Quaternion::Quaternion()
{
	x = 0;
	y = 0;
	z = 0;
	w = 1;
}

Quaternion::Quaternion(float X, float Y, float Z, float W)
{
	x = X;
	y = Y;
	z = Z;
	w = W;
}

Quaternion::Quaternion(const Quaternion& q)
{
	x = q.x;
	y = q.y;
	z = q.z;
	w = q.w;
}

Quaternion::Quaternion(const Matrix4x4& m)
{
	// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
	// article "Quaternion Calculus and Fast Animation".

	float flTrace = m.Trace();
	float flRoot;

	if ( flTrace > 0 )
	{
		// |w| > 1/2, may as well choose w > 1/2
		flRoot = sqrt(flTrace + 1);  // 2w

		w = flRoot/2;

		flRoot = 0.5f/flRoot;  // 1/(4w)
		x = (m.m[1][2]-m.m[2][1])*flRoot;
		y = (m.m[2][0]-m.m[0][2])*flRoot;
		z = (m.m[0][1]-m.m[1][0])*flRoot;
	}
	else
	{
		const int NEXT[3] = { 1, 2, 0 };

		// |w| <= 1/2
		int i = 0;
		if ( m.m[1][1] > m.m[0][0] )
			i = 1;
		if ( m.m[2][2] > m.m[i][i] )
			i = 2;
		int j = NEXT[i];
		int k = NEXT[j];

		flRoot = sqrt(m.m[i][i]-m.m[j][j]-m.m[k][k]+1);
		float* apflQuat[3] = { &x, &y, &z };
		*apflQuat[i] = flRoot/2;
		flRoot = 0.5f/flRoot;
		w = (m.m[j][k]-m.m[k][j])*flRoot;
		*apflQuat[j] = (m.m[i][j]+m.m[j][i])*flRoot;
		*apflQuat[k] = (m.m[i][k]+m.m[k][i])*flRoot;
	}
}

EAngle Quaternion::GetAngles() const
{
	float sqx = x*x;
	float sqy = y*y;
	float sqz = z*z;
	float sqw = w*w;
	
	EAngle angResult;
	angResult.p = atan2(2*(y*z + x*w), (-sqx-sqy+sqz+sqw)) *180/M_PI;
	angResult.y = asin(-2*(x*z - y*w)) *180/M_PI;
	angResult.r = atan2(2*(x*y + z*w), (sqx-sqy-sqz+sqw)) *180/M_PI;
	return angResult;
}

void Quaternion::SetAngles(const EAngle& a)
{
	float c1 = cos(-a.y/2 *M_PI/180);
	float s1 = sin(-a.y/2 *M_PI/180);
	float c2 = cos(a.p/2 *M_PI/180);
	float s2 = sin(a.p/2 *M_PI/180);
	float c3 = cos(a.r/2 *M_PI/180);
	float s3 = sin(a.r/2 *M_PI/180);

	float c1c2 = c1*c2;
	float s1s2 = s1*s2;

	x = c1c2*s3 + s1s2*c3;
	y = s1*c2*c3 + c1*s2*s3;
	z = c1*s2*c3 - s1*c2*s3;
	w = c1c2*c3 - s1s2*s3;
}

void Quaternion::SetRotation(float flAngle, const Vector& v)
{
	float flHalfAngle = flAngle/2;
	float flSin = sin(flHalfAngle);

	x = flSin*v.x;
	y = flSin*v.y;
	z = flSin*v.z;

	w = cos(flHalfAngle);
}

void Quaternion::Normalize()
{
	float m = sqrt(w*w + x*x + y*y + z*z);
	w = w/m;
	x = x/m;
	y = y/m;
	z = z/m;
}

Quaternion Quaternion::operator*(const Quaternion& q)
{
	Quaternion r;
	r.w = w*q.w - x*q.x - y*q.y - z*q.z;
	r.x = w*q.x + x*q.w + y*q.z - z*q.y;
	r.y = w*q.y + y*q.w + z*q.x - x*q.z;
	r.z = w*q.z + z*q.w + x*q.y - y*q.x;
    return r;
}

const Quaternion& Quaternion::operator*=(const Quaternion& q)
{
	w = w*q.w - x*q.x - y*q.y - z*q.z;
	x = w*q.x + x*q.w + y*q.z - z*q.y;
	y = w*q.y + y*q.w + z*q.x - x*q.z;
	z = w*q.z + z*q.w + x*q.y - y*q.x;
    return *this;
}
