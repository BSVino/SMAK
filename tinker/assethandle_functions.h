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

template <class C, class L>
CAssetHandle<C, L>::CAssetHandle(const tstring& sName, C* pAsset)
{
	m_sName = sName;
	if (pAsset)
		m_pAsset = pAsset;
	else
	{
		CAssetHandle<C, L> hAsset = L::FindAsset(sName);
		if (hAsset.IsValid())
			m_pAsset = hAsset.m_pAsset;
		else
			m_pAsset = L::AddAsset(sName);
	}

	if (m_pAsset)
		m_pAsset->m_iReferences++;
}

template <class C, class L>
CAssetHandle<C, L>::CAssetHandle(const CAssetHandle& c)
{
	m_sName = c.m_sName;
	if (c.m_pAsset)
	{
		m_pAsset = c.m_pAsset;
		m_pAsset->m_iReferences++;
	}
	else
		m_pAsset = nullptr;
}

template <class C, class L>
CAssetHandle<C, L>::~CAssetHandle()
{
	Reset();
}

template <class C, class L>
bool CAssetHandle<C, L>::IsValid() const
{
	if (!m_pAsset)
		return false;

#ifdef DEBUG_ASSET_LOADING
	bool bLoaded = L::IsAssetLoaded(m_sName);
	TAssertNoMsg(bLoaded);
	if (!bLoaded)
		return false;
#endif

	return true;
}

template <class C, class L>
void CAssetHandle<C, L>::Reset()
{
	if (!IsValid())
		return;

	TAssertNoMsg(m_pAsset->m_iReferences);
	m_pAsset->m_iReferences--;

	m_pAsset = nullptr;
}

template <class C, class L>
const CAssetHandle<C, L>& CAssetHandle<C, L>::operator=(const CAssetHandle<C, L>& c)
{
	if (c == *this)
		return *this;

	Reset();

	// If c == *this, c.m_pAsset will be clobbered in Reset()
	C* pAsset = c.m_pAsset;

	m_sName = c.m_sName;
	if (pAsset)
	{
		m_pAsset = pAsset;
		m_pAsset->m_iReferences++;
	}
	else
		m_pAsset = nullptr;

	return *this;
}

template <class C, class L>
CAssetHandle<C, L>& CAssetHandle<C, L>::operator=(CAssetHandle<C, L>&& c)
{
	if (c == *this)
		return *this;

	Reset();

	m_pAsset = c.m_pAsset;
	swap(m_sName, c.m_sName);

	c.m_pAsset = nullptr;

	return *this;
}
