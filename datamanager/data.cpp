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

#include "data.h"

#include <strutils.h>

CData::CData()
{
	m_pParent = NULL;
}

CData::CData(tstring sKey, tstring sValue)
{
	m_pParent = NULL;
	m_sKey = sKey;
	m_sValue = sValue;
}

CData::~CData()
{
	for (size_t i = 0; i < m_apChildren.size(); i++)
	{
		CData* pData = m_apChildren[i];
		delete pData;
	}
}

CData* CData::AddChild(tstring sKey)
{
	return AddChild(sKey, "");
}

CData* CData::AddChild(tstring sKey, tstring sValue)
{
	CData* pData = new CData(sKey, sValue);
	m_apChildren.push_back(pData);
	pData->m_pParent = this;
	return pData;
}

size_t CData::FindChildIndex(const tstring& sKey) const
{
	for (size_t i = 0; i < m_apChildren.size(); i++)
		if (m_apChildren[i]->GetKey() == sKey)
			return i;

	return ~0;
}

CData* CData::FindChild(const tstring& sKey) const
{
	size_t iIndex = FindChildIndex(sKey);

	if (iIndex == ~0)
		return NULL;

	return m_apChildren[iIndex];
}

tstring CData::FindChildValueString(const tstring& sKey, const tstring& sDefault) const
{
	CData* pChild = FindChild(sKey);
	if (!pChild)
		return sDefault;

	return pChild->GetValueString();
}

bool CData::FindChildValueBool(const tstring& sKey, bool bDefault) const
{
	CData* pChild = FindChild(sKey);
	if (!pChild)
		return bDefault;

	return pChild->GetValueBool();
}

int CData::FindChildValueInt(const tstring& sKey, int iDefault) const
{
	CData* pChild = FindChild(sKey);
	if (!pChild)
		return iDefault;

	return pChild->GetValueInt();
}

size_t CData::FindChildValueUInt(const tstring& sKey, size_t iDefault) const
{
	CData* pChild = FindChild(sKey);
	if (!pChild)
		return iDefault;

	return pChild->GetValueUInt();
}

float CData::FindChildValueFloat(const tstring& sKey, float flDefault) const
{
	CData* pChild = FindChild(sKey);
	if (!pChild)
		return flDefault;

	return pChild->GetValueFloat();
}

Vector2D CData::FindChildValueVector2D(const tstring& sKey, Vector2D vecDefault) const
{
	CData* pChild = FindChild(sKey);
	if (!pChild)
		return vecDefault;

	return pChild->GetValueVector2D();
}

EAngle CData::FindChildValueEAngle(const tstring& sKey, EAngle angDefault) const
{
	CData* pChild = FindChild(sKey);
	if (!pChild)
		return angDefault;

	return pChild->GetValueEAngle();
}

bool CData::GetValueBool() const
{
	tstring sValue = GetValueString();

	for( tstring::iterator p = sValue.begin(); p != sValue.end(); ++p )
		*p = toupper(*p);  // make string all caps

	if( sValue == tstring("FALSE") || sValue == tstring("F") ||
	    sValue == tstring("NO") || sValue == tstring("N") ||
	    sValue == tstring("0") || sValue == tstring("NONE") || sValue == tstring("OFF") )
		return false;

	return true;
}

int CData::GetValueInt() const
{
	return (int)stoi(GetValueString().c_str());
}

size_t CData::GetValueUInt() const
{
	return (size_t)stoi(GetValueString().c_str());
}

float CData::GetValueFloat() const
{
	return (float)stof(GetValueString().c_str());
}

Vector2D CData::GetValueVector2D() const
{
	tvector<tstring> asTokens;
	tstrtok(GetValueString(), asTokens);

	Vector2D vecResult;
	if (asTokens.size() > 0)
		vecResult.x = (float)stof(asTokens[0].c_str());
	if (asTokens.size() > 1)
		vecResult.y = (float)stof(asTokens[1].c_str());

	return vecResult;
}

Vector CData::GetValueVector() const
{
	tvector<tstring> asTokens;
	tstrtok(GetValueString(), asTokens);

	Vector vecResult;
	if (asTokens.size() > 0)
		vecResult.x = (float)stof(asTokens[0].c_str());
	if (asTokens.size() > 1)
		vecResult.y = (float)stof(asTokens[1].c_str());
	if (asTokens.size() > 2)
		vecResult.z = (float)stof(asTokens[2].c_str());

	return vecResult;
}

Vector4D CData::GetValueVector4D() const
{
	tvector<tstring> asTokens;
	tstrtok(GetValueString(), asTokens);

	Vector4D vecResult;
	if (asTokens.size() > 0)
		vecResult.x = (float)stof(asTokens[0].c_str());
	if (asTokens.size() > 1)
		vecResult.y = (float)stof(asTokens[1].c_str());
	if (asTokens.size() > 2)
		vecResult.z = (float)stof(asTokens[2].c_str());
	if (asTokens.size() > 3)
		vecResult.w = (float)stof(asTokens[3].c_str());

	return vecResult;
}

EAngle CData::GetValueEAngle() const
{
	tvector<tstring> asTokens;
	tstrtok(GetValueString(), asTokens);

	EAngle vecResult;
	if (asTokens.size() > 0)
		vecResult.p = (float)stof(asTokens[0].c_str());
	if (asTokens.size() > 1)
		vecResult.y = (float)stof(asTokens[1].c_str());
	if (asTokens.size() > 2)
		vecResult.r = (float)stof(asTokens[2].c_str());

	return vecResult;
}

TRS CData::GetValueTRS() const
{
	tvector<tstring> asTokens;
	tstrtok(GetValueString(), asTokens);
	TAssertNoMsg(asTokens.size() == 9);
	if (asTokens.size() != 9)
		return TRS();

	TRS trsResult;

	trsResult.m_vecTranslation.x = (float)stof(asTokens[0].c_str());
	trsResult.m_vecTranslation.y = (float)stof(asTokens[1].c_str());
	trsResult.m_vecTranslation.z = (float)stof(asTokens[2].c_str());
	trsResult.m_angRotation.p = (float)stof(asTokens[3].c_str());
	trsResult.m_angRotation.y = (float)stof(asTokens[4].c_str());
	trsResult.m_angRotation.r = (float)stof(asTokens[5].c_str());
	trsResult.m_vecScaling.x = (float)stof(asTokens[6].c_str());
	trsResult.m_vecScaling.y = (float)stof(asTokens[7].c_str());
	trsResult.m_vecScaling.z = (float)stof(asTokens[8].c_str());

	return trsResult;
}

void CData::SetValue(bool bValue)
{
	m_sValue = bValue?"true":"false";
}

void CData::SetValue(int iValue)
{
	m_sValue = sprintf(tstring("%d"), iValue);
}

void CData::SetValue(size_t iValue)
{
	m_sValue = sprintf(tstring("%u"), iValue);
}

void CData::SetValue(float flValue)
{
	m_sValue = sprintf(tstring("%f"), flValue);
}

void CData::SetValue(Vector2D vecValue)
{
	m_sValue = sprintf(tstring("%f, %f"), vecValue.x, vecValue.y);
}

void CData::SetValue(EAngle angValue)
{
	m_sValue = sprintf(tstring("%f, %f, %f"), angValue.p, angValue.y, angValue.r);
}
