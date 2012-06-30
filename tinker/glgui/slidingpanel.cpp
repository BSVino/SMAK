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

#include "slidingpanel.h"

#include <tinker/shell.h>

#include "label.h"

using namespace glgui;

float CSlidingPanel::SLIDER_COLLAPSED_HEIGHT = 20;

CSlidingPanel::CInnerPanel::CInnerPanel(CControl<CSlidingContainer> hMaster)
	: CPanel(0, 0, 100, SLIDER_COLLAPSED_HEIGHT)
{
	m_hMaster = hMaster;
}

bool CSlidingPanel::CInnerPanel::IsVisible()
{
	if (!m_hMaster->IsCurrent(m_hParent.Downcast<CSlidingPanel>()))
		return false;

	return CPanel::IsVisible();
}

CSlidingPanel::CSlidingPanel(CControl<CSlidingContainer> hParent, char* pszTitle)
	: CPanel(0, 0, 100, 5)
{
	SetBorder(BT_SOME);

	TAssert(hParent);

	m_bCurrent = false;

	m_hTitle = AddControl(new CLabel(0, 0, 100, SLIDER_COLLAPSED_HEIGHT, pszTitle));

	m_hInnerPanel = AddControl(new CInnerPanel(hParent));
	m_hInnerPanel->SetBorder(CPanel::BT_NONE);
	m_hInnerPanel->SetDefaultMargin(2);

	// Add to tail so that panels appear in the order they are added.
	hParent->AddControl(this, true);
}

void CSlidingPanel::Layout()
{
	m_hTitle->SetSize(m_hParent->GetWidth(), SLIDER_COLLAPSED_HEIGHT);

	m_hInnerPanel->SetPos(5, SLIDER_COLLAPSED_HEIGHT);
	m_hInnerPanel->SetSize(GetWidth() - 10, GetHeight() - 5 - SLIDER_COLLAPSED_HEIGHT);

	CPanel::Layout();
}

void CSlidingPanel::Paint(float x, float y, float w, float h)
{
	if (!IsVisible())
		return;

	CPanel::Paint(x, y, w, h);
}

bool CSlidingPanel::MousePressed(int code, int mx, int my)
{
	CSlidingContainer* pParent = m_hParent.DowncastStatic<CSlidingContainer>();

	if (pParent->IsCurrent(this))
		return CPanel::MousePressed(code, mx, my);
	else
	{
		pParent->SetCurrent(this);
		return true;
	}
}

CControlHandle CSlidingPanel::AddControl(CBaseControl* pControl, bool bToTail)
{
	// The title and inner panel should be added to this panel.
	// All other controls should be added to the inner panel.
	// This way the inner panel can be set not visible in order
	// to set all children not visible at once.

	if (pControl != m_hTitle && pControl != m_hInnerPanel)
		return m_hInnerPanel->AddControl(pControl, bToTail);

	return CPanel::AddControl(pControl, bToTail);
}

void CSlidingPanel::SetCurrent(bool bCurrent)
{
	m_bCurrent = bCurrent;

	m_hInnerPanel->SetVisible(bCurrent);
}

CSlidingContainer::CSlidingContainer()
	: CPanel(0, 0, 100, 100)
{
	m_iCurrent = 0;

	SetBorder(BT_NONE);
	SetCurrent(0);
}

CSlidingContainer::CSlidingContainer(float x, float y, float w, float h)
	: CPanel(x, y, w, h)
{
	m_iCurrent = 0;

	SetBorder(BT_NONE);
	SetCurrent(0);
}

void CSlidingContainer::Layout()
{
	float y = 0;
	size_t iCount = m_apControls.size();
	float flCurrentHeight = GetHeight() - CSlidingPanel::SLIDER_COLLAPSED_HEIGHT * (VisiblePanels()-1);

	for (size_t i = 0; i < iCount; i++)
	{
		m_apControls[i]->SetPos(0, y);
		m_apControls[i]->SetSize(GetWidth(), (i == m_iCurrent)?flCurrentHeight:CSlidingPanel::SLIDER_COLLAPSED_HEIGHT);

		y += (i == m_iCurrent)?flCurrentHeight:CSlidingPanel::SLIDER_COLLAPSED_HEIGHT;
	}

	CPanel::Layout();
}

CControlHandle CSlidingContainer::AddControl(CBaseControl* pControl, bool bToTail)
{
	if (!pControl)
		return CControlHandle();

	TAssert(dynamic_cast<CSlidingPanel*>(pControl));

	CControlHandle hControl = CPanel::AddControl(pControl, bToTail);

	// Re-layout now that we've added some. Maybe this one is the current one!
	SetCurrent(m_iCurrent);

	return hControl;
}

bool CSlidingContainer::IsCurrent(int iPanel)
{
	return iPanel == m_iCurrent;
}

void CSlidingContainer::SetCurrent(int iPanel)
{
	if (m_iCurrent < (int)m_apControls.size())
		dynamic_cast<CSlidingPanel*>(m_apControls[m_iCurrent].get())->SetCurrent(false);

	m_iCurrent = iPanel;

	// iPanel may be invalid, for example if the container is empty and being initialized to 0.
	if (m_iCurrent < (int)m_apControls.size())
	{
		dynamic_cast<CSlidingPanel*>(m_apControls[m_iCurrent].get())->SetCurrent(true);

		Layout();
	}
}

bool CSlidingContainer::IsCurrent(CSlidingPanel* pPanel)
{
	for (size_t i = 0; i < m_apControls.size(); i++)
		if (m_apControls[i] == pPanel)
			return IsCurrent((int)i);

	return false;
}

void CSlidingContainer::SetCurrent(CSlidingPanel* pPanel)
{
	for (size_t i = 0; i < m_apControls.size(); i++)
		if (m_apControls[i] == pPanel)
			SetCurrent((int)i);
}

bool CSlidingContainer::IsCurrentValid()
{
	if (m_iCurrent >= (int)m_apControls.size())
		return false;

	if (!m_apControls[m_iCurrent]->IsVisible())
		return false;

	return true;
}

int CSlidingContainer::VisiblePanels()
{
	int iResult = 0;
	size_t iCount = m_apControls.size();
	for (size_t i = 0; i < iCount; i++)
	{
		if (m_apControls[i]->IsVisible())
			iResult++;
	}
	return iResult;
}
