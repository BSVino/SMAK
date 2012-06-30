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

#ifndef _SIMPLEX_H
#define _SIMPLEX_H

#include <mtrand.h>

class CSimplexNoise
{
public:
					CSimplexNoise(size_t iSeed);

public:
	float			Noise(float x, float y);

private:
	inline float	Grad(int hash, float x, float y)
	{
		int h = hash&7;
		float u = h<4?x:y;
		float v = h<4?y:x;
		return ((h&1)?-u:u) + ((h&2)? -2.0f*v : 2.0f*v);
	}

	inline float	Fade(float t)
	{
		return t*t*t*(t*(t*6-15)+10);
	}

	inline int		Floor(float x)
	{
		if (x >= 0)
			return (int)x;
		else
			return (int)x-1;
	}

	inline float	Lerp(float t, float a, float b)
	{
		return a + t*(b-a);
	}

private:
	unsigned char	m_aiRand[512];
};

inline CSimplexNoise::CSimplexNoise(size_t iSeed)
{
	mtsrand(iSeed);
	for (size_t i = 0; i < 256; i++)
		m_aiRand[i] = (unsigned char)(mtrand()%255);
	memcpy(&m_aiRand[256], &m_aiRand[0], 256);
}

inline float CSimplexNoise::Noise(float x, float y)
{
    int ix0, iy0, ix1, iy1;
    float fx0, fy0, fx1, fy1;
    float s, t, nx0, nx1, n0, n1;

    ix0 = Floor(x);
    iy0 = Floor(y);
    fx0 = x - ix0;
    fy0 = y - iy0;
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    ix1 = (ix0 + 1) & 0xff;
    iy1 = (iy0 + 1) & 0xff;
    ix0 = ix0 & 0xff;
    iy0 = iy0 & 0xff;
    
    t = Fade( fy0 );
    s = Fade( fx0 );

    nx0 = Grad(m_aiRand[ix0 + m_aiRand[iy0]], fx0, fy0);
    nx1 = Grad(m_aiRand[ix0 + m_aiRand[iy1]], fx0, fy1);
    n0 = Lerp( t, nx0, nx1 );

    nx0 = Grad(m_aiRand[ix1 + m_aiRand[iy0]], fx1, fy0);
    nx1 = Grad(m_aiRand[ix1 + m_aiRand[iy1]], fx1, fy1);
    n1 = Lerp(t, nx0, nx1);

    return 0.507f * ( Lerp( s, n0, n1 ) );
}

#endif
