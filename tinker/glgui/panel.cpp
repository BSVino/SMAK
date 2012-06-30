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

#include "panel.h"

#include <tinker/shell.h>
#include <renderer/renderingcontext.h>
#include <tinker/keys.h>

#include "rootpanel.h"
#include "scrollbar.h"

using namespace glgui;

CPanel::CPanel()
	: CBaseControl(0, 0, 100, 100)
{
	m_flMargin = 15;

	m_bHighlight = false;
	m_bScissoring = false;
}

CPanel::CPanel(float x, float y, float w, float h)
	: CBaseControl(x, y, w, h)
{
	m_flMargin = 15;

	SetBorder(BT_NONE);
	SetBackgroundColor(Color(0, 0, 0, 0));
	m_bHighlight = false;
	m_bScissoring = false;
}

CPanel::~CPanel()
{
	while (m_apControls.size())
		RemoveControl(m_apControls[0]);
}

bool CPanel::KeyPressed(int code, bool bCtrlDown)
{
	int iCount = (int)m_apControls.size();

	// Start at the end of the list so that items drawn last are tested for keyboard events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		CBaseControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		if (pControl->TakesFocus() && !pControl->HasFocus())
			continue;

		if (pControl->KeyPressed(code, bCtrlDown))
			return true;

		if (pControl->HasFocus() && code == TINKER_KEY_TAB)
		{
			NextTabStop();
			return true;
		}
	}
	return false;
}

bool CPanel::KeyReleased(int code)
{
	int iCount = (int)m_apControls.size();

	// Start at the end of the list so that items drawn last are tested for keyboard events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		CBaseControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		if (pControl->KeyReleased(code))
			return true;
	}
	return false;
}

bool CPanel::CharPressed(int code)
{
	int iCount = (int)m_apControls.size();

	// Start at the end of the list so that items drawn last are tested for keyboard events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		CBaseControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		if (pControl->CharPressed(code))
			return true;
	}

	return false;
}

bool CPanel::MousePressed(int code, int mx, int my)
{
	int iCount = (int)m_apControls.size();
	// Start at the end of the list so that items drawn last are tested for mouse events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		CBaseControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		float x = 0, y = 0, w = 0, h = 0;
		pControl->GetAbsDimensions(x, y, w, h);
		if (mx >= x &&
			my >= y &&
			mx < x + w &&
			my < y + h)
		{
			if (pControl->MousePressed(code, mx, my))
				return true;
		}
	}
	return false;
}

bool CPanel::MouseReleased(int code, int mx, int my)
{
	int iCount = (int)m_apControls.size();
	// Start at the end of the list so that items drawn last are tested for mouse events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		CBaseControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		float x, y, w, h;
		pControl->GetAbsDimensions(x, y, w, h);
		if (mx >= x &&
			my >= y &&
			mx < x + w &&
			my < y + h)
		{
			if (pControl->MouseReleased(code, mx, my))
				return true;
		}
	}
	return false;
}

bool CPanel::MouseDoubleClicked(int code, int mx, int my)
{
	int iCount = (int)m_apControls.size();
	// Start at the end of the list so that items drawn last are tested for mouse events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		CBaseControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		float x = 0, y = 0, w = 0, h = 0;
		pControl->GetAbsDimensions(x, y, w, h);
		if (mx >= x &&
			my >= y &&
			mx < x + w &&
			my < y + h)
		{
			if (pControl->MouseDoubleClicked(code, mx, my))
				return true;
		}
	}
	return false;
}

void CPanel::CursorMoved(int mx, int my)
{
	bool bFoundControlWithCursor = false;

	int iCount = (int)m_apControls.size();
	// Start at the end of the list so that items drawn last are tested for mouse events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		CBaseControl* pControl = m_apControls[i];

		if (!pControl->IsVisible() || !pControl->IsCursorListener())
			continue;

		float x, y, w, h;
		pControl->GetAbsDimensions(x, y, w, h);

		if (m_hVerticalScrollBar && pControl != m_hVerticalScrollBar)
		{
			if (mx >= m_hVerticalScrollBar->GetLeft())
				continue;
		}

		if (m_hHorizontalScrollBar && pControl != m_hHorizontalScrollBar)
		{
			if (my >= m_hHorizontalScrollBar->GetTop())
				continue;
		}

		if (mx >= x &&
			my >= y &&
			mx < x + w &&
			my < y + h)
		{
			if (m_hHasCursor != pControl)
			{
				if (m_hHasCursor)
					m_hHasCursor->CursorOut();
				m_hHasCursor = pControl->GetHandle();
				m_hHasCursor->CursorIn();
			}

			pControl->CursorMoved(mx, my);

			bFoundControlWithCursor = true;
			break;
		}
	}

	if (!bFoundControlWithCursor && m_hHasCursor)
	{
		m_hHasCursor->CursorOut();
		m_hHasCursor.reset();
	}
}

void CPanel::CursorOut()
{
	if (m_hHasCursor)
	{
		m_hHasCursor->CursorOut();
		m_hHasCursor.reset();
	}

	BaseClass::CursorOut();
}

CControlHandle CPanel::GetHasCursor()
{
	if (!m_hHasCursor)
		return m_hThis;

	return m_hHasCursor->GetHasCursor();
}

void CPanel::NextTabStop()
{
	size_t iOriginalFocus = 0;

	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		CBaseControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		if (pControl->HasFocus())
		{
			iOriginalFocus = i;
			break;
		}
	}

	for (size_t i = 0; i < m_apControls.size()-1; i++)
	{
		size_t iControl = (iOriginalFocus + i + 1)%m_apControls.size();
		CBaseControl* pControl = m_apControls[iControl];

		if (!pControl->IsVisible())
			continue;

		if (pControl->TakesFocus())
		{
			CBaseControl* pBaseControl = dynamic_cast<CBaseControl*>(pControl);
			if (!pBaseControl)
				continue;

			CRootPanel::Get()->SetFocus(pBaseControl->GetHandle());
			return;
		}
	}
}

CControlHandle CPanel::AddControl(CBaseControl* pControl, bool bToTail)
{
	TAssertNoMsg(pControl);
	if (!pControl)
		return CControlHandle();

	return AddControl(pControl->shared_from_this(), bToTail);
}

CControlHandle CPanel::AddControl(CControlResource pControl, bool bToTail)
{
	TAssertNoMsg(pControl.get());
#ifdef _DEBUG
	auto it = CBaseControl::GetControls().find(pControl);
	TAssertNoMsg(it != CBaseControl::GetControls().end());
	if (it != CBaseControl::GetControls().end())
		// Make sure no errant accidental implicit conversions to a CResource are accidentally added here.
		// If that happens then there would be two CResource (shared_ptr) objects pointing two the same
		// memory, which would cause tons of bad stuff to happen. CControlResource is supposed to prevent
		// this from happening, so if you get this assert then there's big trouble in Little China.
		TAssertNoMsg(it->second.use_count() == pControl.use_count());
#endif

	if (!pControl.get())
		return CControlHandle();

	TAssertNoMsg(pControl != this);

#ifdef _DEBUG
	for (size_t i = 0; i < m_apControls.size(); i++)
		TAssertNoMsg(m_apControls[i] != pControl);	// You're adding a control to the panel twice! Quit it!
#endif

	TAssertNoMsg(m_hThis);

	pControl->SetParent(m_hThis);

	if (bToTail)
		m_apControls.push_back(pControl);
	else
		m_apControls.insert(m_apControls.begin(), pControl);

	return CControlHandle(pControl);
}

void CPanel::RemoveControl(CBaseControl* pControl)
{
	pControl->SetParent(CControlHandle());

	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		if (m_apControls[i] == pControl)
		{
			m_apControls.erase(m_apControls.begin()+i);
			break;
		}
	}

	if (m_hHasCursor == pControl)
		m_hHasCursor.reset();
}

void CPanel::MoveToTop(CBaseControl* pControl)
{
	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		if (m_apControls[i] == pControl)
		{
			CControlResource pControlResource = m_apControls[i];
			m_apControls.erase(m_apControls.begin()+i);
			m_apControls.push_back(pControlResource);
			return;
		}
	}
}

void CPanel::Layout( void )
{
	FRect rPanelBounds = GetAbsDimensions();
	FRect rAllBounds = GetAbsDimensions();

	size_t iCount = m_apControls.size();
	for (size_t i = 0; i < iCount; i++)
	{
		m_apControls[i]->Layout();

		FRect rControlBounds = m_apControls[i]->GetAbsDimensions();

		if (rControlBounds.x < rAllBounds.x)
			rAllBounds.x = rControlBounds.x;

		if (rControlBounds.y < rAllBounds.y)
			rAllBounds.y = rControlBounds.y;

		if (rControlBounds.Right() > rAllBounds.Right())
			rAllBounds.w = rControlBounds.Right() - rAllBounds.x;

		if (rControlBounds.Bottom() > rAllBounds.Bottom())
			rAllBounds.h = rControlBounds.Bottom() - rAllBounds.y;
	}

	m_rControlBounds = rAllBounds;

	if (m_hVerticalScrollBar)
	{
		m_hVerticalScrollBar->SetVisible((rAllBounds.y < rPanelBounds.y) || (rAllBounds.Bottom() > rPanelBounds.Bottom()));
	}

	if (m_hHorizontalScrollBar)
	{
		m_hHorizontalScrollBar->SetVisible((rAllBounds.x < rPanelBounds.x) || (rAllBounds.Right() > rPanelBounds.Right()));
	}
}

void CPanel::UpdateScene( void )
{
	size_t iCount = m_apControls.size();
	for (size_t i = 0; i < iCount; i++)
		m_apControls[i]->UpdateScene();
}

void CPanel::Paint()
{
	float x = 0, y = 0;
	GetAbsPos(x, y);
	Paint(x, y);
}

void CPanel::Paint(float x, float y)
{
	Paint(x, y, m_flW, m_flH);
}

void CPanel::Paint(float x, float y, float w, float h)
{
	if (!IsVisible())
		return;

	bool bScissor = m_bScissoring;
	FRect rScissor;
	if (bScissor)
	{
		float sx, sy;
		GetAbsPos(sx, sy);

		rScissor = FRect(sx, sy, GetWidth(), GetHeight());

		//CRootPanel::PaintRect(rScissor.x, rScissor.y, rScissor.w, rScissor.h, Color(0, 0, 100, 50));
	}

	size_t iCount = m_apControls.size();
	for (size_t i = 0; i < iCount; i++)
	{
		CBaseControl* pControl = m_apControls[i];
		if (!pControl->IsVisible())
			continue;

		if (bScissor)
		{
			CRootPanel::GetContext()->SetUniform("bScissor", true);
			CRootPanel::GetContext()->SetUniform("vecScissor", Vector4D(&rScissor.x));
		}

		// Translate this location to the child's local space.
		float cx, cy, ax, ay;
		pControl->GetAbsPos(cx, cy);
		GetAbsPos(ax, ay);
		pControl->PaintBackground(cx+x-ax, cy+y-ay, pControl->GetWidth(), pControl->GetHeight());
		pControl->Paint(cx+x-ax, cy+y-ay);
	}

	if (bScissor)
		CRootPanel::GetContext()->SetUniform("bScissor", false);

	BaseClass::Paint(x, y, w, h);
}

void CPanel::PostPaint()
{
	if (!IsVisible())
		return;

	bool bScissor = m_bScissoring;
	float sx, sy;
	if (bScissor)
	{
		GetAbsPos(sx, sy);

		//CRootPanel::PaintRect(sx, sy, GetWidth(), GetHeight(), Color(0, 0, 100, 50));

		CRootPanel::GetContext()->SetUniform("bScissor", true);
		CRootPanel::GetContext()->SetUniform("vecScissor", Vector4D(sx, sy, GetWidth(), GetHeight()));
	}

	size_t iCount = m_apControls.size();
	for (size_t i = 0; i < iCount; i++)
	{
		CBaseControl* pControl = m_apControls[i];
		if (!pControl->IsVisible())
			continue;

		if (bScissor)
		{
			CRootPanel::GetContext()->SetUniform("bScissor", true);
			CRootPanel::GetContext()->SetUniform("vecScissor", Vector4D(sx, sy, GetWidth(), GetHeight()));
		}

		pControl->PostPaint();
	}

	if (bScissor)
		CRootPanel::GetContext()->SetUniform("bScissor", false);

	BaseClass::PostPaint();
}

bool CPanel::ShouldControlOffset(const CBaseControl* pControl) const
{
	if (pControl == m_hVerticalScrollBar || pControl == m_hHorizontalScrollBar)
		return false;

	return true;
}

void CPanel::Think()
{
	size_t iCount = m_apControls.size();
	for (size_t i = iCount-1; i < iCount; i--)
	{
		m_apControls[i]->Think();
	}

	m_rControlOffset = FRect(0, 0, 0, 0);

	if (m_hVerticalScrollBar && m_hVerticalScrollBar->IsVisible())
	{
		float flScrollable = m_rControlBounds.h - GetHeight();
		m_rControlOffset.y = -GetVerticalScrollBar()->GetHandlePosition() * flScrollable;
	}

	if (m_hHorizontalScrollBar && m_hHorizontalScrollBar->IsVisible())
	{
		float flScrollable = m_rControlBounds.w - GetWidth();
		m_rControlOffset.x = -GetHorizontalScrollBar()->GetHandlePosition() * flScrollable;
	}
}

void CPanel::SetVerticalScrollBarEnabled(bool b)
{
	if (m_hVerticalScrollBar && b)
		return;

	if (!m_hVerticalScrollBar && !b)
		return;

	if (b)
	{
		m_hVerticalScrollBar = AddControl(new CScrollBar(false));
		Layout();
	}
	else
	{
		RemoveControl(m_hVerticalScrollBar);
	}
}

void CPanel::SetHorizontalScrollBarEnabled(bool b)
{
	if (m_hHorizontalScrollBar && b)
		return;

	if (!m_hHorizontalScrollBar && !b)
		return;

	if (b)
	{
		m_hHorizontalScrollBar = AddControl(new CScrollBar(false));
		Layout();
	}
	else
	{
		RemoveControl(m_hHorizontalScrollBar);
	}
}

CControl<CScrollBar> CPanel::GetVerticalScrollBar() const
{
	return m_hVerticalScrollBar;
}

CControl<CScrollBar> CPanel::GetHorizontalScrollBar() const
{
	return m_hHorizontalScrollBar;
}
