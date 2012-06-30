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

#include "menu.h"

#include "rootpanel.h"
#include "scrollbar.h"

using namespace glgui;

CControl<CMenu> CRootPanel::AddMenu(const tstring& sText)
{
	if (!m_hMenuBar)
		return CControl<CMenu>();

	if (m_hMenuBar->GetControls().size() == 0)
		m_hMenuBar->SetVisible(true);

	CControl<CMenu> hMenu(m_hMenuBar->AddControl(new CMenu(sText), true));
	hMenu->SetWrap(false);

	return hMenu;
}

CMenuBar::CMenuBar()
	: CPanel(0, 0, 1024, MENU_HEIGHT)
{
	SetVisible(false);
}

void CMenuBar::Layout( void )
{
	if (GetParent())
	{
		SetSize(GetParent()->GetWidth(), MENU_HEIGHT);
		SetPos(MENU_SPACING, MENU_SPACING);
	}

	CPanel::Layout();

	float x = 0;
	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		m_apControls[i]->SetPos(x, 0);
		x += m_apControls[i]->GetWidth() + MENU_SPACING;
	}
}

void CMenuBar::SetActive( CMenu* pActiveMenu )
{
	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		CMenu* pCurrentMenu = dynamic_cast<CMenu*>(m_apControls[i].get());

		if (!pCurrentMenu)
			continue;

		if (pCurrentMenu != pActiveMenu)
			pCurrentMenu->Pop(true, true);
	}
}

CMenu::CMenu(const tstring& sText, bool bSubmenu)
	: CButton(0, 0, 41, MENU_HEIGHT, sText, true)
{
	m_bSubmenu = bSubmenu;

	m_flHighlightGoal = m_flHighlight = m_flMenuHighlightGoal = m_flMenuHighlight = m_flMenuHeightGoal = m_flMenuHeight
		= m_flMenuSelectionHighlightGoal = m_flMenuSelectionHighlight = 0;

	m_MenuSelection = FRect(0, 0, 0, 0);
	m_MenuSelectionGoal = FRect(0, 0, 0, 0);

	SetClickedListener(this, Open);
	SetUnclickedListener(this, Close);

	m_pfnMenuCallback = NULL;
	m_pMenuListener = NULL;

	m_hMenu = RootPanel()->AddControl(new CSubmenuPanel(m_hThis), true);

	m_hMenu->SetVisible(false);
}

CMenu::~CMenu()
{
}

void CMenu::Think()
{
	// Make a copy so that the below logic doesn't clobber CursorOut()
	float flHightlightGoal = m_flHighlightGoal;

	// If our menu is open always stay highlighted.
	if (m_hMenu->IsVisible())
		flHightlightGoal = 1;

	if (!IsVisible())
		CloseMenu();

	m_flHighlight = Approach(flHightlightGoal, m_flHighlight, (float)CRootPanel::Get()->GetFrameTime()*3);
	m_flMenuHighlight = Approach(m_flMenuHighlightGoal, m_flMenuHighlight, (float)CRootPanel::Get()->GetFrameTime()*3);
	m_flMenuHeight = Approach(m_flMenuHeightGoal, m_flMenuHeight, (float)CRootPanel::Get()->GetFrameTime()*3);
	m_hMenu->SetFakeHeight(m_flMenuHeight);

	m_hMenu->SetVisible(m_flMenuHighlight > 0 && m_flMenuHeight > 0);

	m_flMenuSelectionHighlightGoal = 0;

	for (size_t i = 0; i < m_hMenu->GetControls().size(); i++)
	{
		CBaseControl* pControl = m_hMenu->GetControls()[i];

		if (pControl == m_hMenu->m_hVerticalScrollBar.Get())
			continue;

		if (pControl == m_hMenu->m_hHorizontalScrollBar.Get())
			continue;

		float cx, cy, cw, ch;
		int mx, my;

		pControl->GetAbsDimensions(cx, cy, cw, ch);
		CRootPanel::GetFullscreenMousePos(mx, my);
		if (mx >= cx &&
			my >= cy &&
			mx < cx + cw &&
			my < cy + ch)
		{
			m_flMenuSelectionHighlightGoal = 1;
			m_MenuSelectionGoal = FRect((float)cx, (float)cy, (float)cw, (float)ch);
			break;
		}
	}

	if (m_flMenuSelectionHighlight < 0.01f)
		m_MenuSelection = m_MenuSelectionGoal;
	else
	{
		m_MenuSelection.x = Approach(m_MenuSelectionGoal.x, m_MenuSelection.x, (float)CRootPanel::Get()->GetFrameTime()*1800);
		m_MenuSelection.y = Approach(m_MenuSelectionGoal.y, m_MenuSelection.y, (float)CRootPanel::Get()->GetFrameTime()*1800);
		m_MenuSelection.w = Approach(m_MenuSelectionGoal.w, m_MenuSelection.w, (float)CRootPanel::Get()->GetFrameTime()*1800);
		m_MenuSelection.h = Approach(m_MenuSelectionGoal.h, m_MenuSelection.h, (float)CRootPanel::Get()->GetFrameTime()*1800);
	}

	m_flMenuSelectionHighlight = Approach(m_flMenuSelectionHighlightGoal, m_flMenuSelectionHighlight, (float)CRootPanel::Get()->GetFrameTime()*3);
}

void CMenu::Layout()
{
	m_hMenu->m_hMenu = m_hThis;

	float iHeight = 0;
	float iWidth = 0;
	tvector<CControlResource> apControls = m_hMenu->GetControls();
	size_t iControls = 0;
	for (size_t i = 0; i < apControls.size(); i++)
	{
		CBaseControl* pControl = apControls[i];

		if (pControl == m_hMenu->m_hVerticalScrollBar.Get())
			continue;

		if (pControl == m_hMenu->m_hHorizontalScrollBar.Get())
			continue;

		pControl->SetPos(5, (float)(iControls*MENU_HEIGHT));
		iHeight = pControl->GetBottom() + 5;
		if (pControl->GetWidth()+10 > iWidth)
			iWidth = pControl->GetWidth()+10;

		iControls++;
	}

	float x, y;
	GetAbsPos(x, y);

	m_hMenu->SetSize(iWidth, iHeight);

	if (y + GetHeight() + 5 + iHeight < RootPanel()->GetHeight())
		m_hMenu->SetPos(x, y + 5 + GetHeight());
	else if (y - 5 - iHeight > 0)
		m_hMenu->SetPos(x, y - 5 - iHeight);
	else
	{
		m_hMenu->SetPos(x, 0);
		if (iHeight > RootPanel()->GetHeight())
		{
			m_hMenu->SetVerticalScrollBarEnabled(true);
			m_hMenu->SetHeight(RootPanel()->GetHeight());
		}
	}

	m_hMenu->Layout();
}

void CMenu::Paint(float x, float y, float w, float h)
{
	if (!m_bSubmenu)
	{
		Color clrBox = m_clrButton;
		clrBox.SetAlpha((int)RemapVal(m_flHighlight, 0, 1, 125, 255));
		CRootPanel::PaintRect(x, y, w, h, clrBox, 1);
	}

	CLabel::Paint(x, y, w, h);
}

void CMenu::PostPaint()
{
	if (m_hMenu->IsVisible())
	{
		float mx, my, mw, mh;
		m_hMenu->GetAbsDimensions(mx, my, mw, mh);

		float flMenuHeight = Lerp(m_flMenuHeight, 0.6f);
		if (flMenuHeight > 0.99f)
			flMenuHeight = 0.99f;	// When it hits 1 it jerks.

		Color clrBox = g_clrBox;
		clrBox.SetAlpha((int)RemapVal(m_flMenuHighlight, 0, 1, 0, 255));
		CRootPanel::PaintRect(mx, (float)(my), mw, (float)(mh*flMenuHeight), clrBox);

		if (m_flMenuSelectionHighlight > 0)
		{
			clrBox = g_clrBoxHi;
			clrBox.SetAlpha((int)(255 * m_flMenuSelectionHighlight * flMenuHeight));
			CRootPanel::PaintRect((float)m_MenuSelection.x, (float)m_MenuSelection.y+1, (float)m_MenuSelection.w, (float)m_MenuSelection.h-2, clrBox);
		}
	}
}

void CMenu::CursorIn()
{
	m_flHighlightGoal = 1;

	CButton::CursorIn();
}

void CMenu::CursorOut()
{
	m_flHighlightGoal = 0;

	CButton::CursorOut();
}

void CMenu::SetMenuListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	m_pfnMenuCallback = pfnCallback;
	m_pMenuListener = pListener;
}

void CMenu::OpenCallback(const tstring& sArgs)
{
	CRootPanel::Get()->GetMenuBar()->SetActive(this);

	if (m_hMenu->GetControls().size())
	{
		OpenMenu();
		Layout();
	}
}

void CMenu::CloseCallback(const tstring& sArgs)
{
	if (m_hMenu->GetControls().size())
		CloseMenu();
}

void CMenu::ClickedCallback(const tstring& sArgs)
{
	CRootPanel::Get()->GetMenuBar()->SetActive(NULL);

	if (m_pMenuListener)
		m_pfnMenuCallback(m_pMenuListener, sArgs);
}

void CMenu::OpenMenu()
{
	m_flMenuHighlightGoal = 1;
	m_flMenuHeightGoal = 1;
}

void CMenu::CloseMenu()
{
	m_flMenuHighlightGoal = 0;
	m_flMenuHeightGoal = 0;
	SetState(false, false);
}

void CMenu::AddSubmenu(const tstring& sTitle, IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	CControl<CMenu> hMenu = m_hMenu->AddControl(new CMenu(sTitle, true), true);
	hMenu->SetAlign(TA_LEFTCENTER);
	hMenu->SetWrap(false);
	hMenu->EnsureTextFits();
	hMenu->SetToggleButton(false);

	if (pListener)
		hMenu->SetMenuListener(pListener, pfnCallback);

	size_t iTotalControls = 0;
	for (size_t i = 0; i < m_hMenu->GetControls().size(); i++)
	{
		CBaseControl* pControl = m_hMenu->GetControls()[i];

		if (pControl == m_hMenu->m_hVerticalScrollBar.Get())
			continue;

		if (pControl == m_hMenu->m_hHorizontalScrollBar.Get())
			continue;

		iTotalControls++;
	}

	hMenu->SetClickedListener(hMenu, Clicked, sprintf("%d " + sTitle, iTotalControls-1));

	m_ahEntries.push_back(hMenu);
}

void CMenu::ClearSubmenus()
{
	for (size_t i = 0; i < m_ahEntries.size(); i++)
		m_hMenu->RemoveControl(m_ahEntries[i]);

	m_ahEntries.clear();
}

size_t CMenu::GetSelectedMenu()
{
	for (size_t i = 0; i < m_ahEntries.size(); i++)
	{
		float cx, cy, cw, ch;
		int mx, my;
		m_ahEntries[i]->GetAbsDimensions(cx, cy, cw, ch);
		CRootPanel::GetFullscreenMousePos(mx, my);
		if (mx >= cx &&
			my >= cy &&
			mx < cx + cw &&
			my < cy + ch)
		{
			return i;
		}
	}

	return ~0;
}

CMenu::CSubmenuPanel::CSubmenuPanel(CControl<CMenu> hMenu)
	: CPanel(0, 0, 100, 100)
{
	m_hMenu = hMenu;
}

void CMenu::CSubmenuPanel::Think()
{
	if (m_apControls.size() != m_aflControlHighlightGoal.size() || m_apControls.size() != m_aflControlHighlight.size())
	{
		m_aflControlHighlightGoal.clear();
		m_aflControlHighlight.clear();

		for (size_t i = 0; i < m_apControls.size(); i++)
		{
			m_aflControlHighlightGoal.push_back(0);
			m_aflControlHighlight.push_back(0);
		}
	}

	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		CBaseControl* pControl = m_apControls[i];

		if (pControl == m_hVerticalScrollBar)
			continue;

		if (pControl == m_hHorizontalScrollBar)
			continue;

		float x, y;
		pControl->GetPos(x, y);

		y += m_rControlOffset.y;

		if (y < m_flFakeHeight*GetHeight())
			m_aflControlHighlightGoal[i] = 1.0f;
		else
			m_aflControlHighlightGoal[i] = 0.0f;

		m_aflControlHighlight[i] = Approach(m_aflControlHighlightGoal[i], m_aflControlHighlight[i], (float)CRootPanel::Get()->GetFrameTime()*3);

		pControl->SetAlpha((int)(m_aflControlHighlight[i] * 255));
	}

	CPanel::Think();
}

void CMenu::CSubmenuPanel::Paint(float x, float y, float w, float h)
{
}

void CMenu::CSubmenuPanel::PostPaint()
{
	float x, y, w, h;
	GetAbsDimensions(x, y, w, h);
	BaseClass::Paint(x, y, w, h);
}

bool CMenu::CSubmenuPanel::IsVisible()
{
	if (!m_hMenu)
		return BaseClass::IsVisible();

	return m_hMenu->IsVisible() && BaseClass::IsVisible();
}
