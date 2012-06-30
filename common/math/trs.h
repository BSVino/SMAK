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

#include "vector.h"
#include "matrix.h"

class TRS
{
public:
				TRS();
				TRS(const Vector& T, const EAngle& R, const Vector& S);

public:
	Matrix4x4	GetMatrix4x4(bool bRotation = true, bool bScaling = true) const;

public:
	Vector		m_vecTranslation;
	EAngle		m_angRotation;
	Vector		m_vecScaling;
};

inline TRS::TRS()
{
	m_vecScaling = Vector(1, 1, 1);
}

inline TRS::TRS(const Vector& T, const EAngle& R, const Vector& S)
{
	m_vecTranslation = T;
	m_angRotation = R;
	m_vecScaling = S;
}

inline Matrix4x4 TRS::GetMatrix4x4(bool bRotation, bool bScaling) const
{
	Matrix4x4 m;
	m.SetTranslation(m_vecTranslation);

	if (bRotation)
		m.SetAngles(m_angRotation);

	if (bScaling)
		m.AddScale(m_vecScaling);

	return m;
}
