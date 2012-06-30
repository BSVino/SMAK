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

#ifndef LW_DATA_H
#define LW_DATA_H

#include <tvector.h>
#include <vector.h>
#include <tstring.h>
#include <trs.h>

class CData
{
public:
							CData();
							CData(tstring sKey, tstring sValue);
							~CData();

public:
	CData*					AddChild(tstring sKey);
	CData*					AddChild(tstring sKey, tstring sValue);

	CData*					GetParent() const { return m_pParent; }

	size_t					FindChildIndex(const tstring& sKey) const;
	CData*					FindChild(const tstring& sKey) const;
	tstring					FindChildValueString(const tstring& sKey, const tstring& sDefault="") const;
	bool					FindChildValueBool(const tstring& sKey, bool bDefault=false) const;
	int						FindChildValueInt(const tstring& sKey, int iDefault=0) const;
	size_t					FindChildValueUInt(const tstring& sKey, size_t iDefault=0) const;
	float					FindChildValueFloat(const tstring& sKey, float flDefault=0) const;
	Vector2D				FindChildValueVector2D(const tstring& sKey, Vector2D vecDefault=Vector()) const;
	EAngle					FindChildValueEAngle(const tstring& sKey, EAngle angDefault=EAngle()) const;

	tstring					GetKey() const { return m_sKey; }
	tstring					GetValueString() const { return m_sValue; }
	bool					GetValueBool() const;
	int						GetValueInt() const;
	size_t					GetValueUInt() const;
	float					GetValueFloat() const;
	Vector2D				GetValueVector2D() const;
	Vector					GetValueVector() const;
	Vector4D				GetValueVector4D() const;
	EAngle					GetValueEAngle() const;
	TRS						GetValueTRS() const;

	void					SetKey(tstring sKey) { m_sKey = sKey; }
	void					SetValue(tstring sValue) { m_sValue = sValue; }
	void					SetValue(bool);
	void					SetValue(int);
	void					SetValue(size_t);
	void					SetValue(float);
	void					SetValue(Vector2D);
	void					SetValue(EAngle);

	size_t					GetNumChildren() const { return m_apChildren.size(); }
	CData*					GetChild(size_t i) const { return m_apChildren[i]; }

protected:
	CData*					m_pParent;

	tstring					m_sKey;
	tstring					m_sValue;
	tvector<CData*>			m_apChildren;
};

#endif
