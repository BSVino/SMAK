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

#ifndef LW_COLOR_H
#define LW_COLOR_H

#include "maths.h"

template <class unit_t>
class TemplateVector;

class Color
{
public:
	inline			Color();
	inline			Color(class TemplateVector<float> v);
	inline			Color(int _r, int _g, int _b);
	inline			Color(int _r, int _g, int _b, int _a);
	inline			Color(float _r, float _g, float _b);
	inline			Color(float _r, float _g, float _b, float _a);

	inline void		SetColor(int _r, int _g, int _b, int _a);
	inline void		SetColor(float _r, float _g, float _b, float _a);
	inline void		SetRed(int _r);
	inline void		SetGreen(int _g);
	inline void		SetBlue(int _b);
	inline void		SetAlpha(int _a);
	inline void		SetAlpha(float f);

	inline void		SetHSL(float h, float s, float l);

	int				r() const { return red; };
	int				g() const { return green; };
	int				b() const { return blue; };
	int				a() const { return alpha; };

	Color	operator-(void) const;

	Color	operator+(const Color& v) const;
	Color	operator-(const Color& v) const;
	Color	operator*(float s) const;
	Color	operator/(float s) const;

	void	operator+=(const Color &v);
	void	operator-=(const Color &v);
	void	operator*=(float s);
	void	operator/=(float s);

	Color	operator*(const Color& v) const;

	friend Color operator*( float f, const Color& v )
	{
		return Color( v.red*f, v.green*f, v.blue*f, v.alpha*f );
	}

	friend Color operator/( float f, const Color& v )
	{
		return Color( f/v.red, f/v.green, f/v.blue, f/v.alpha );
	}

	operator unsigned char*()
	{
		return(&red);
	}

	operator const unsigned char*() const
	{
		return(&red);
	}

private:
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
	unsigned char	alpha;
};

#include "vector.h"

Color::Color()
{
	SetColor(0, 0, 0, 255);
}

Color::Color(Vector v)
{
	SetColor((int)(v.x*255), (int)(v.y*255), (int)(v.z*255), 255);
}

Color::Color(int _r, int _g, int _b)
{
	SetColor(_r, _g, _b, 255);
}

Color::Color(int _r, int _g, int _b, int _a)
{
	SetColor(_r, _g, _b, _a);
}

Color::Color(float _r, float _g, float _b)
{
	SetColor(_r, _g, _b, 1.0f);
}

Color::Color(float _r, float _g, float _b, float _a)
{
	SetColor(_r, _g, _b, _a);
}

void Color::SetColor(int _r, int _g, int _b, int _a)
{
	red = _r;
	green = _g;
	blue = _b;
	alpha = _a;
}

void Color::SetColor(float _r, float _g, float _b, float _a)
{
	red = (int)(_r*255);
	green = (int)(_g*255);
	blue = (int)(_b*255);
	alpha = (int)(_a*255);
}

void Color::SetRed(int _r)
{
	red = _r;
}

void Color::SetGreen(int _g)
{
	green = _g;
}

void Color::SetBlue(int _b)
{
	blue = _b;
}

void Color::SetAlpha(int _a)
{
	alpha = _a;
}

void Color::SetAlpha(float f)
{
	alpha = (int)(f*255);
}

void Color::SetHSL(float flHue, float flSaturation, float flLightness)
{
	float flChroma = (1 - fabs(2 * flLightness - 1)) * flSaturation;
	float flX = flChroma * (1 - fabs(fmod(flHue, 2) - 1));

	float flR1, flG1, flB1;
	if (flHue < 1)
	{
		flR1 = flChroma;
		flG1 = flX;
		flB1 = 0;
	}
	else if (flHue < 2)
	{
		flR1 = flX;
		flG1 = flChroma;
		flB1 = 0;
	}
	else if (flHue < 3)
	{
		flR1 = 0;
		flG1 = flChroma;
		flB1 = flX;
	}
	else if (flHue < 4)
	{
		flR1 = 0;
		flG1 = flX;
		flB1 = flChroma;
	}
	else if (flHue < 5)
	{
		flR1 = flX;
		flG1 = 0;
		flB1 = flChroma;
	}
	else
	{
		flR1 = flChroma;
		flG1 = 0;
		flB1 = flX;
	}

	float flM = flLightness - flChroma/2;

	*this = Color(flR1+flM, flG1+flM, flB1+flM);
}

inline Color Color::operator-() const
{
	return Color(255-red, 255-green, 255-blue, alpha);
}

inline Color Color::operator+(const Color& v) const
{
	return Color(red+v.red, green+v.green, blue+v.blue, alpha+v.alpha);
}

inline Color Color::operator-(const Color& v) const
{
	return Color(red-v.red, green-v.green, blue-v.blue, alpha-v.alpha);
}

inline Color Color::operator*(float s) const
{
	return Color((int)(red*s), (int)(green*s), (int)(blue*s), (int)alpha);
}

inline Color Color::operator/(float s) const
{
	return Color((int)(red/s), (int)(green/s), (int)(blue/s), (int)alpha);
}

inline void Color::operator+=(const Color& v)
{
	red += v.red;
	green += v.green;
	blue += v.blue;
}

inline void Color::operator-=(const Color& v)
{
	red -= v.red;
	green -= v.green;
	blue -= v.blue;
}

inline void Color::operator*=(float s)
{
	red = (int)(s*red);
	green = (int)(s*green);
	blue = (int)(s*blue);
}

inline void Color::operator/=(float s)
{
	red = (int)(red/s);
	green = (int)(green/s);
	blue = (int)(blue/s);
}

inline Color Color::operator*(const Color& v) const
{
	return Color(red*(float)(v.red/255), green*(float)(v.green/255), blue*(float)(v.blue/255), alpha*(float)(v.alpha/255));
}

#endif
