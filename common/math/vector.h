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

#ifndef LW_VECTOR_H
#define LW_VECTOR_H

#include <math.h>

// for size_t
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265f
#endif

template <class unit_t>
class TemplateVector2D;

template <class unit_t>
class TemplateVector
{
public:
			TemplateVector();
			TemplateVector(class Color c);
			TemplateVector(unit_t x, unit_t y, unit_t z);
			TemplateVector(const unit_t* xyz);

			// Conversions
			TemplateVector(const TemplateVector<float>& v);
			TemplateVector(const TemplateVector<double>& v);
			TemplateVector(const class TemplateVector2D<float>& v);
			TemplateVector(const class TemplateVector2D<double>& v);

public:
	const TemplateVector<unit_t>	operator-(void) const;

	const TemplateVector<unit_t>	operator+(const TemplateVector<unit_t>& v) const;
	const TemplateVector<unit_t>	operator-(const TemplateVector<unit_t>& v) const;
	const TemplateVector<unit_t>	operator*(unit_t s) const;
	const TemplateVector<unit_t>	operator/(unit_t s) const;

	void	operator+=(const TemplateVector<unit_t> &v);
	void	operator-=(const TemplateVector<unit_t> &v);
	void	operator*=(unit_t s);
	void	operator/=(unit_t s);

	const TemplateVector<unit_t>	operator*(const TemplateVector<unit_t>& v) const;
	const TemplateVector<unit_t>	operator/(const TemplateVector<unit_t>& v) const;

	friend const TemplateVector<unit_t> operator*( unit_t f, const TemplateVector<unit_t>& v )
	{
		return TemplateVector( v.x*f, v.y*f, v.z*f );
	}

	friend const TemplateVector<unit_t> operator/( unit_t f, const TemplateVector<unit_t>& v )
	{
		return TemplateVector( f/v.x, f/v.y, f/v.z );
	}

	bool	operator==(const TemplateVector<unit_t>& v) const
	{
		float flEp = 0.000001f;
		if (fabs(v.x - x) < flEp && fabs(v.y - y) < flEp && fabs(v.z - z) < flEp)
			return true;
		return false;
	}

	bool	operator!=(const TemplateVector<unit_t>& v) const
	{
		float flEp = 0.000001f;
		return fabs(v.x - x) > flEp || fabs(v.y - y) > flEp || fabs(v.z - z) > flEp;
	}

	unit_t	Length() const;
	unit_t	LengthSqr() const;
	unit_t	Length2D() const;
	unit_t	Length2DSqr() const;
	void	Normalize();
	const TemplateVector<unit_t>	Normalized() const;
	const TemplateVector<unit_t>	Flattened() const;
	unit_t	Average() const;

	unit_t	Distance(const TemplateVector<unit_t>& v) const;
	unit_t	DistanceSqr(const TemplateVector<unit_t>& v) const;

	unit_t	Dot(const TemplateVector<unit_t>& v) const;
	const TemplateVector<unit_t>	Cross(const TemplateVector<unit_t>& v) const;

	bool	IsZero() const
	{
		return x == 0 && y == 0 && z == 0;
	}

	operator unit_t*()
	{
		return(&x);
	}

	operator const unit_t*() const
	{
		return(&x);
	}

	unit_t	operator[](int i) const;
	unit_t&	operator[](int i);

	unit_t	operator[](size_t i) const;
	unit_t&	operator[](size_t i);

	unit_t	x, y, z;
};

typedef TemplateVector<float> Vector;
typedef TemplateVector<double> DoubleVector;

#include <color.h>

template <class unit_t>
inline TemplateVector<unit_t>::TemplateVector()
	: x(0), y(0), z(0)
{
}

template <class unit_t>
inline TemplateVector<unit_t>::TemplateVector(Color c)
{
	x = (float)c.r()/255.0f;
	y = (float)c.g()/255.0f;
	z = (float)c.b()/255.0f;
}

template <class unit_t>
inline TemplateVector<unit_t>::TemplateVector(unit_t X, unit_t Y, unit_t Z)
	: x(X), y(Y), z(Z)
{
}

template <class unit_t>
inline TemplateVector<unit_t>::TemplateVector(const unit_t* xyz)
	: x(*xyz), y(*(xyz+1)), z(*(xyz+2))
{
}

template <class unit_t>
inline TemplateVector<unit_t>::TemplateVector(const TemplateVector<float>& v)
	: x((unit_t)v.x), y((unit_t)v.y), z((unit_t)v.z)
{
}

template <class unit_t>
inline TemplateVector<unit_t>::TemplateVector(const TemplateVector<double>& v)
	: x((unit_t)v.x), y((unit_t)v.y), z((unit_t)v.z)
{
}

template <class unit_t>
inline const TemplateVector<unit_t> TemplateVector<unit_t>::operator-() const
{
	return TemplateVector(-x, -y, -z);
}

template <class unit_t>
inline const TemplateVector<unit_t> TemplateVector<unit_t>::operator+(const TemplateVector<unit_t>& v) const
{
	return TemplateVector(x+v.x, y+v.y, z+v.z);
}

template <class unit_t>
inline const TemplateVector<unit_t> TemplateVector<unit_t>::operator-(const TemplateVector<unit_t>& v) const
{
	return TemplateVector(x-v.x, y-v.y, z-v.z);
}

template <class unit_t>
inline const TemplateVector<unit_t> TemplateVector<unit_t>::operator*(unit_t s) const
{
	return TemplateVector(x*s, y*s, z*s);
}

template <class unit_t>
inline const TemplateVector<unit_t> TemplateVector<unit_t>::operator/(unit_t s) const
{
	return TemplateVector(x/s, y/s, z/s);
}

template <class unit_t>
inline void TemplateVector<unit_t>::operator+=(const TemplateVector<unit_t>& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}

template <class unit_t>
inline void TemplateVector<unit_t>::operator-=(const TemplateVector<unit_t>& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

template <class unit_t>
inline void TemplateVector<unit_t>::operator*=(unit_t s)
{
	x *= s;
	y *= s;
	z *= s;
}

template <class unit_t>
inline void TemplateVector<unit_t>::operator/=(unit_t s)
{
	x /= s;
	y /= s;
	z /= s;
}

template <class unit_t>
inline const TemplateVector<unit_t> TemplateVector<unit_t>::operator*(const TemplateVector<unit_t>& v) const
{
	return TemplateVector(x*v.x, y*v.y, z*v.z);
}

template <class unit_t>
inline const TemplateVector<unit_t> TemplateVector<unit_t>::operator/(const TemplateVector<unit_t>& v) const
{
	return TemplateVector(x/v.x, y/v.y, z/v.z);
}

template <class unit_t>
inline unit_t TemplateVector<unit_t>::Length() const
{
	return sqrt(x*x + y*y + z*z);
}

template <class unit_t>
inline unit_t TemplateVector<unit_t>::LengthSqr() const
{
	return x*x + y*y + z*z;
}

template <class unit_t>
inline unit_t TemplateVector<unit_t>::Length2D() const
{
	return sqrt(x*x + z*z);
}

template <class unit_t>
inline unit_t TemplateVector<unit_t>::Length2DSqr() const
{
	return x*x + z*z;
}

template <class unit_t>
inline void TemplateVector<unit_t>::Normalize()
{
	unit_t flLength = Length();
	if (!flLength)
		*this=TemplateVector(0,0,1);
	else
		*this/=flLength;
}

template <class unit_t>
inline const TemplateVector<unit_t> TemplateVector<unit_t>::Normalized() const
{
	unit_t flLength = Length();
	if (!flLength)
		return TemplateVector(0,0,1);
	else
		return *this/flLength;
}

template <class unit_t>
inline const TemplateVector<unit_t> TemplateVector<unit_t>::Flattened() const
{
	TemplateVector<unit_t> vecResult(*this);
	vecResult.y = 0;
	return vecResult;
}

template <class unit_t>
inline unit_t TemplateVector<unit_t>::Average() const
{
	return (x + y + z)/3;
}

template <class unit_t>
inline unit_t TemplateVector<unit_t>::Distance(const TemplateVector<unit_t>& v) const
{
	return (*this - v).Length();
}

template <class unit_t>
inline unit_t TemplateVector<unit_t>::DistanceSqr(const TemplateVector<unit_t>& v) const
{
	return (*this - v).LengthSqr();
}

template <class unit_t>
inline unit_t TemplateVector<unit_t>::Dot(const TemplateVector<unit_t>& v) const
{
	return x*v.x + y*v.y + z*v.z;
}

template <class unit_t>
inline const TemplateVector<unit_t> TemplateVector<unit_t>::Cross(const TemplateVector<unit_t>& v) const
{
	return TemplateVector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
}

template <class unit_t>
inline unit_t& TemplateVector<unit_t>::operator[](int i)
{
	return (&x)[i];
}

template <class unit_t>
inline unit_t TemplateVector<unit_t>::operator[](int i) const
{
	return (&x)[i];
}

template <class unit_t>
inline unit_t& TemplateVector<unit_t>::operator[](size_t i)
{
	return (&x)[i];
}

template <class unit_t>
inline unit_t TemplateVector<unit_t>::operator[](size_t i) const
{
	return (&x)[i];
}

// Euler angles
// Positive pitch looks up, negative looks down.
// Positive yaw rotates like a top to the right, negative to the left.
// Positive roll banks to the right (right wing down, left wing up) negative roll to the left.
class EAngle
{
public:
			EAngle();
			EAngle(float p, float y, float r);
			EAngle(float* pyr);

	const EAngle	operator+(const EAngle& v) const;
	const EAngle	operator-(const EAngle& v) const;

	const EAngle	operator*(float f) const;
	const EAngle	operator/(float f) const;

	bool	operator==(const EAngle& v) const
	{
		float flEp = 0.000001f;
		return fabs(AngleDifference(v.p, p)) < flEp && fabs(AngleDifference(v.y, y)) < flEp && fabs(AngleDifference(v.r, r)) < flEp;
	}

	bool	Equals(const EAngle& v, float flEp = 0.000001f) const
	{
		return fabs(AngleDifference(v.p, p)) < flEp && fabs(AngleDifference(v.y, y)) < flEp && fabs(AngleDifference(v.r, r)) < flEp;
	}

	// Can find equivalence even in dangerous situations such as Gimbal lock.
	bool	EqualsExhaustive(const EAngle& v, float flEp = 0.000001f) const;

	operator float*()
	{
		return(&p);
	}

	float	p, y, r;
};

inline EAngle::EAngle()
	: p(0), y(0), r(0)
{
}

inline EAngle::EAngle(float P, float Y, float R)
	: p(P), y(Y), r(R)
{
}

inline EAngle::EAngle(float* pyr)
	: p(*pyr), y(*(pyr+1)), r(*(pyr+2))
{
}

inline const EAngle EAngle::operator+(const EAngle& v) const
{
	return EAngle(p+v.p, y+v.y, r+v.r);
}

inline const EAngle EAngle::operator-(const EAngle& v) const
{
	return EAngle(p-v.p, y-v.y, r-v.r);
}

inline const EAngle EAngle::operator*(float f) const
{
	return EAngle(p*f, y*f, r*f);
}

inline const EAngle EAngle::operator/(float f) const
{
	return EAngle(p/f, y/f, r/f);
}

inline const Vector AngleVector(const EAngle& a)
{
	Vector vecResult;

	float p = (float)(a.p * (M_PI*2 / 360));
	float y = (float)(a.y * (M_PI*2 / 360));
	float r = (float)(a.r * (M_PI*2 / 360));

	float sp = sin(p);
	float cp = cos(p);
	float sy = sin(y);
	float cy = cos(y);

	vecResult.x = cp*cy;
	vecResult.y = sp;
	vecResult.z = cp*sy;

	return vecResult;
}

void AngleVectors(const EAngle& a, Vector* pvecF, Vector* pvecU, Vector* pvecR);

inline const EAngle VectorAngles( const Vector& vecForward )
{
	EAngle angReturn(0, 0, 0);

	angReturn.p = atan2(vecForward.y, sqrt(vecForward.x*vecForward.x + vecForward.z*vecForward.z)) * 180/M_PI;
	angReturn.y = atan2(vecForward.z, vecForward.x) * 180/M_PI;

	return angReturn;
}

template <class T> const T LerpValue(const T& from, const T& to, float flLerp);

template <>
inline const EAngle LerpValue(const EAngle& from, const EAngle& to, float flLerp)
{
	float p = from.p + (AngleDifference(to.p, from.p) * flLerp);
	float y = from.y + (AngleDifference(to.y, from.y) * flLerp);
	float r = from.r + (AngleDifference(to.r, from.r) * flLerp);
	return EAngle(p, y, r);
}

template <class unit_t>
class TemplateVector2D
{
public:
				TemplateVector2D();
				TemplateVector2D(unit_t x, unit_t y);
				TemplateVector2D(TemplateVector<unit_t> v);

				// Conversions
				TemplateVector2D(const TemplateVector2D<float>& v);
				TemplateVector2D(const TemplateVector2D<double>& v);

public:
	float	Length() const;
	float	LengthSqr() const;

	const TemplateVector2D<unit_t>	operator+(const TemplateVector2D<unit_t>& v) const;
	const TemplateVector2D<unit_t>	operator-(const TemplateVector2D<unit_t>& v) const;
	const TemplateVector2D<unit_t>	operator*(float s) const;
	const TemplateVector2D<unit_t>	operator/(float s) const;

	bool	operator==(const TemplateVector2D<unit_t>& v) const
	{
		float flEp = 0.000001f;
		if (fabs(v.x - x) < flEp && fabs(v.y - y) < flEp)
			return true;
		return false;
	}

	operator float*()
	{
		return(&x);
	}

	operator const float*() const
	{
		return(&x);
	}

	unit_t	x, y;
};

template <class unit_t>
inline TemplateVector<unit_t>::TemplateVector(const TemplateVector2D<float>& v)
	: x((unit_t)v.x), y((unit_t)v.y), z(0)
{
}

template <class unit_t>
inline TemplateVector<unit_t>::TemplateVector(const TemplateVector2D<double>& v)
	: x((unit_t)v.x), y((unit_t)v.y), z(0)
{
}

typedef TemplateVector2D<float> Vector2D;
typedef TemplateVector2D<double> DoubleVector2D;

template <class unit_t>
inline TemplateVector2D<unit_t>::TemplateVector2D()
	: x(0), y(0)
{
}

template <class unit_t>
inline TemplateVector2D<unit_t>::TemplateVector2D(unit_t X, unit_t Y)
	: x(X), y(Y)
{
}

template <class unit_t>
inline TemplateVector2D<unit_t>::TemplateVector2D(TemplateVector<unit_t> v)
	: x(v.x), y(v.y)
{
}

template <class unit_t>
inline TemplateVector2D<unit_t>::TemplateVector2D(const TemplateVector2D<float>& v)
	: x((unit_t)v.x), y((unit_t)v.y)
{
}

template <class unit_t>
inline TemplateVector2D<unit_t>::TemplateVector2D(const TemplateVector2D<double>& v)
	: x((unit_t)v.x), y((unit_t)v.y)
{
}

template <class unit_t>
inline float TemplateVector2D<unit_t>::Length() const
{
	return sqrt(x*x + y*y);
}

template <class unit_t>
inline float TemplateVector2D<unit_t>::LengthSqr() const
{
	return x*x + y*y;
}

template <class unit_t>
inline const TemplateVector2D<unit_t> TemplateVector2D<unit_t>::operator+(const TemplateVector2D<unit_t>& v) const
{
	return TemplateVector2D(x+v.x, y+v.y);
}

template <class unit_t>
inline const TemplateVector2D<unit_t> TemplateVector2D<unit_t>::operator-(const TemplateVector2D<unit_t>& v) const
{
	return TemplateVector2D(x-v.x, y-v.y);
}

template <class unit_t>
inline const TemplateVector2D<unit_t> TemplateVector2D<unit_t>::operator*(float s) const
{
	return TemplateVector2D(x*s, y*s);
}

template <class unit_t>
inline const TemplateVector2D<unit_t> TemplateVector2D<unit_t>::operator/(float s) const
{
	return TemplateVector2D(x/s, y/s);
}

class Vector4D
{
public:
				Vector4D();
				Vector4D(const Vector& v);
				Vector4D(const Vector& v, float w);
				Vector4D(const class Color& c);
				Vector4D(float x, float y, float z, float w);
				Vector4D(const float* xyzw);

public:
	const Vector4D	operator+(const Vector4D& v) const;
	const Vector4D	operator-(const Vector4D& v) const;

	bool	operator==(const Vector4D& v) const
	{
		float flEp = 0.000001f;
		return fabs(v.x - x) < flEp && fabs(v.y - y) < flEp && fabs(v.z - z) < flEp && fabs(v.w - w) < flEp;
	}

	operator float*()
	{
		return(&x);
	}

	operator const float*() const
	{
		return(&x);
	}

	float	x, y, z, w;
};

#include "color.h"

inline Vector4D::Vector4D()
	: x(0), y(0), z(0), w(0)
{
}

inline Vector4D::Vector4D(const Vector& v)
	: x(v.x), y(v.y), z(v.z), w(0)
{
}

inline Vector4D::Vector4D(const Vector& v, float W)
	: x(v.x), y(v.y), z(v.z), w(W)
{
}

inline Vector4D::Vector4D(const Color& c)
	: x(((float)c.r())/255), y(((float)c.g())/255), z(((float)c.b())/255), w(((float)c.a())/255)
{
}

inline Vector4D::Vector4D(float X, float Y, float Z, float W)
	: x(X), y(Y), z(Z), w(W)
{
}

inline Vector4D::Vector4D(const float* xyzw)
	: x(*xyzw), y(*(xyzw+1)), z(*(xyzw+2)), w(*(xyzw+3))
{
}

inline const Vector4D Vector4D::operator+(const Vector4D& v) const
{
	return Vector4D(x+v.x, y+v.y, z+v.z, w+v.w);
}

inline const Vector4D Vector4D::operator-(const Vector4D& v) const
{
	return Vector4D(x-v.x, y-v.y, z-v.z, w-v.w);
}

#endif
