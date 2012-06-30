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

#include "rootpanel.h"

#include <tinker/shell.h>
#include <renderer/renderingcontext.h>

#include "menu.h"

using namespace glgui;

CControlResource CRootPanel::s_pRootPanel;
bool CRootPanel::s_bRootPanelValid;

CRootPanel::CRootPanel() :
	CPanel(0, 0, 800, 600)
{
	TAssert(!s_pRootPanel);

	CPanel::SetBorder(BT_NONE);

	m_pDragging = nullptr;

	m_flFrameTime = 0;
	m_flTime = 0;

	m_flNextGCSweep = 5;

	m_bUseLighting = true;
	m_bGarbageCollecting = false;
	m_bDrawingDraggable = false;

	s_bRootPanelValid = true;

	m_hMenuBar = AddControl(new CMenuBar(), true);
}

CRootPanel::~CRootPanel( )
{
	s_bRootPanelValid = false;

	TAssert(s_pRootPanel == this);
}

CRootPanel*	CRootPanel::Get()
{
	if (!s_pRootPanel)
		s_pRootPanel = (new CRootPanel())->shared_from_this();

	if (!s_bRootPanelValid)
		return nullptr;

	return s_pRootPanel.DowncastStatic<CRootPanel>();
}

void CRootPanel::Think(double flNewTime)
{
	if (m_flTime == flNewTime)
		return;

	m_flFrameTime = (flNewTime - m_flTime);

	// Time running backwards? Maybe the server restarted.
	if (m_flFrameTime < 0)
		m_flFrameTime = 0;

	CPanel::Think();

	m_flTime = flNewTime;

	if (m_flTime > m_flNextGCSweep)
	{
		m_bGarbageCollecting = true;

		auto it = CBaseControl::GetControls().begin();
		while (it != CBaseControl::GetControls().end())
		{
			if (it->second.use_count() == 1)
				it->second.reset();

			if (!it->second.get())
				CBaseControl::GetControls().erase(it++);
			else
				it++;
		}

		m_bGarbageCollecting = false;
		m_flNextGCSweep = m_flTime + 5;
	}
}

void CRootPanel::UpdateScene()
{
	m_pDragging = NULL;

	CPanel::UpdateScene();
}

void CRootPanel::Paint(float x, float y, float w, float h)
{
	SetSize(w, h);

	Matrix4x4 mProjection = Matrix4x4::ProjectOrthographic(x, x+w, y+h, y, -1000, 1000);

	::CRenderingContext c;
	m_pRenderingContext = &c;

	c.SetProjection(mProjection);
	c.UseProgram("gui");
	c.SetDepthTest(false);
	c.UseFrameBuffer(NULL);

	CPanel::Paint(x, y, w, h);
	CPanel::PostPaint();

	if (m_pDragging)
	{
		m_bDrawingDraggable = true;

		int mx, my;
		CRootPanel::GetFullscreenMousePos(mx, my);

		float iWidth = m_pDragging->GetCurrentDraggable()->GetWidth();
		float iHeight = m_pDragging->GetCurrentDraggable()->GetHeight();
		m_pDragging->GetCurrentDraggable()->Paint(mx-iWidth/2, my-iHeight/2, iWidth, iHeight, true);

		m_bDrawingDraggable = false;
	}

	m_pRenderingContext = nullptr;
}

void CRootPanel::Layout()
{
	// Don't layout if 
	if (m_pDragging)
		return;

	CPanel::Layout();
}

void CRootPanel::SetButtonDown(CControl<CButton> hButton)
{
	m_hButtonDown = hButton;
}

CControl<CButton> CRootPanel::GetButtonDown() const
{
	return m_hButtonDown;
}

bool CRootPanel::MousePressed(int code, int mx, int my, bool bInsideControl)
{
	TAssert(!m_pDragging);

	if (CPanel::MousePressed(code, mx, my))
		return true;

	if (!bInsideControl)
		return false;

	int iCount = (int)m_apControls.size();
	for (int i = 0; i < iCount; i++)
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
			// If we were inside any visible elements, don't rotate the screen.
			return true;
		}
	}

	return false;
}

bool CRootPanel::MouseReleased(int code, int mx, int my)
{
	if (m_pDragging)
	{
		if (DropDraggable())
			return true;
	}

	bool bUsed = CPanel::MouseReleased(code, mx, my);

	if (!bUsed)
	{
		// Nothing caught the mouse release, so lets try to pop some buttons.
//		if (m_pButtonDown)
//			return m_pButtonDown->Pop(false, true);
	}

	return bUsed;
}

bool CRootPanel::MouseDoubleClicked(int code, int mx, int my)
{
	TAssert(!m_pDragging);

	if (CPanel::MouseDoubleClicked(code, mx, my))
		return true;

	return false;
}

void CRootPanel::CursorMoved(int x, int y)
{
	m_iMX = x;
	m_iMY = y;

	if (!m_pDragging)
	{
		CPanel::CursorMoved(x, y);
	}
}

void CRootPanel::DragonDrop(IDroppable* pDroppable)
{
	if (!pDroppable->IsVisible())
		return;

	if (!pDroppable->GetCurrentDraggable()->IsDraggable())
		return;

	TAssert(pDroppable);

	m_pDragging = pDroppable;
}

void CRootPanel::AddDroppable(IDroppable* pDroppable)
{
	TAssert(pDroppable);
	m_apDroppables.push_back(pDroppable);
}

void CRootPanel::RemoveDroppable(IDroppable* pDroppable)
{
	TAssert(pDroppable);
	for (size_t i = 0; i < m_apDroppables.size(); i++)
		if (m_apDroppables[i] == pDroppable)
			m_apDroppables.erase(remove(m_apDroppables.begin(), m_apDroppables.end(), pDroppable), m_apDroppables.end());
}

bool CRootPanel::DropDraggable()
{
	TAssert(m_pDragging);

	int mx, my;
	CRootPanel::GetFullscreenMousePos(mx, my);

	// Drop that shit like a bad habit.

	size_t iCount = m_apDroppables.size();
	for (size_t i = 0; i < iCount; i++)
	{
		IDroppable* pDroppable = m_apDroppables[i];

		TAssert(pDroppable);

		if (!pDroppable)
			continue;

		if (!pDroppable->IsVisible())
			continue;

		if (!pDroppable->CanDropHere(m_pDragging->GetCurrentDraggable()))
			continue;

		FRect r = pDroppable->GetHoldingRect();
		if (mx >= r.x &&
			my >= r.y &&
			mx < r.x + r.w &&
			my < r.y + r.h)
		{
			pDroppable->SetDraggable(m_pDragging->GetCurrentDraggable());

			m_pDragging = NULL;

			// Layouts during dragging are blocked. Do a Layout() here to do any updates that need doing since the thing was dropped.
			Layout();

			return true;
		}
	}

	// Layouts during dragging are blocked. Do a Layout() here to do any updates that need doing since the thing was dropped.
	Layout();

	// Couldn't find any places to drop? Whatever nobody cares about that anyways.
	m_pDragging = NULL;

	return false;
}

bool CRootPanel::SetFocus(CControlHandle hFocus)
{
	if (m_hFocus)
		m_hFocus->SetFocus(false);

	m_hFocus = hFocus;

	if (hFocus)
		return hFocus->SetFocus(true);

	return false;
}

void CRootPanel::GetFullscreenMousePos(int& mx, int& my)
{
	mx = Get()->m_iMX;
	my = Get()->m_iMY;
}
