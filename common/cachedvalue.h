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

#include "common.h"

template <class C, class E>
class CCachedValue
{
public:
	typedef C (*CalculateCallback)(E*);

public:
	CCachedValue()
	{
		m_bDirty = true;
		m_pfnCalculate = nullptr;
		m_pCalculateEntity = nullptr;
	}

	CCachedValue(const C& oData)
	{
		m_bDirty = false;
		m_oData = oData;
		m_pfnCalculate = nullptr;
		m_pCalculateEntity = nullptr;
	}

public:
	void		SetCallbacks(CalculateCallback pfnCalculate, E* p)
	{
		m_pfnCalculate = pfnCalculate;
		m_pCalculateEntity = p;
	}

	void		Dirtify()	// Strunk would cry
	{
		m_bDirty = true;
	}

	void		Calculate()
	{
		if (!m_bDirty)
			return;

		if (!m_pfnCalculate || !m_pCalculateEntity)
			return;

		m_oData = m_pfnCalculate(m_pCalculateEntity);
		m_bDirty = false;
	}

	const C&	Get()
	{
		Calculate();

		return m_oData;
	}

	const C& operator=(const C& c);
	const C& operator=(const CCachedValue<C, E>& c);

	inline bool operator==(const C& c) const
	{
		Calculate();

		TAssertNoMsg(!m_bDirty);
		if (m_bDirty)
			return false;

		return c == m_oData;
	}

	inline bool operator!=(const C& c) const
	{
		Calculate();

		TAssertNoMsg(!m_bDirty);
		if (m_bDirty)
			return false;

		return c != m_oData;
	}

	inline bool operator!() const
	{
		Calculate();

		TAssertNoMsg(!m_bDirty);
		if (m_bDirty)
			return false;

		return !m_oData;
	}

	inline C operator+(const C& c) const
	{
		Calculate();

		TAssertNoMsg(!m_bDirty);
		if (m_bDirty)
			return C();

		return m_oData + c;
	}

	inline C operator-(const C& c) const
	{
		Calculate();

		TAssertNoMsg(!m_bDirty);
		if (m_bDirty)
			return C();

		return m_oData - c;
	}

	inline const C& operator+=(const C& c)
	{
		Calculate();

		TAssertNoMsg(!m_bDirty);
		if (m_bDirty)
		{
			static C c;
			return c;
		}

		m_oData += c;
		return m_oData;
	}

	inline const C& operator-=(const C& c)
	{
		Calculate();

		TAssertNoMsg(!m_bDirty);
		if (m_bDirty)
		{
			static C c;
			return c;
		}

		m_oData -= c;
		return m_oData;
	}

	// Suffix
	inline C operator++(int)
	{
		Calculate();

		TAssertNoMsg(!m_bDirty);
		if (m_bDirty)
			return C();

		C oReturn = m_oData;
		m_oData++;
		return oReturn;
	}

	// Prefix
	inline const C& operator++()
	{
		Calculate();

		TAssertNoMsg(!m_bDirty);
		if (m_bDirty)
		{
			static C c;
			return c;
		}

		m_oData++;
		return m_oData;
	}

	// Suffix
	inline C operator--(int)
	{
		Calculate();

		TAssertNoMsg(!m_bDirty);
		if (m_bDirty)
			return C();

		C oReturn = m_oData;
		m_oData--;
		return oReturn;
	}

	// Prefix
	inline const C& operator--()
	{
		Calculate();

		TAssertNoMsg(!m_bDirty);
		if (m_bDirty)
		{
			static C c;
			return c;
		}

		m_oData--;
		return m_oData;
	}

	inline operator const C&()
	{
		Calculate();

		TAssertNoMsg(!m_bDirty);
		if (m_bDirty)
		{
			static C c;
			return c;
		}

		return m_oData;
	}

private:
	bool				m_bDirty;
	C					m_oData;
	CalculateCallback	m_pfnCalculate;
	E*					m_pCalculateEntity;
};

template <class C, class E>
inline const C& CCachedValue<C, E>::operator=(const C& c)
{
	m_oData = c;
	m_bDirty = false;
	return m_oData;
}

template <class C, class E>
inline const C& CCachedValue<C, E>::operator=(const CCachedValue<C, E>& c)
{
	m_oData = c.m_oData;
	m_bDirty = c.m_bDirty;

	if (m_bDirty)
	{
		static C c;
		return c;
	}

	return m_oData;
}
