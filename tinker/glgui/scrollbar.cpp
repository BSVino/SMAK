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

#include "scrollbar.h"

#include "rootpanel.h"

using namespace glgui;

#define HANDLE_SIZE 12

CScrollBar::CScrollBar(bool bHorizontal)
	: CBaseControl(0, 0, 100, 100)
{
	m_bHorizontal = bHorizontal;
	m_flHandlePosition = m_flHandlePositionGoal = 0;
	m_bMovingHandle = false;

	m_flHandleSize = HANDLE_SIZE;
	m_flHandleGrabOffset = 0;
}

void CScrollBar::Layout()
{
	BaseClass::Layout();

	if (!GetParent())
		return;

	if (m_bHorizontal)
	{
		SetSize(GetParent()->GetWidth(), HANDLE_SIZE);
		SetPos(0, GetParent()->GetHeight()-HANDLE_SIZE);
	}
	else
	{
		SetSize(HANDLE_SIZE, GetParent()->GetHeight());
		SetPos(GetParent()->GetWidth()-HANDLE_SIZE, 0);
	}
}

void CScrollBar::Paint(float x, float y, float w, float h)
{
	//CRootPanel::PaintRect(x, y, w, h, g_clrBoxHi);

	if (m_bHorizontal)
	{
		float flLeft = x+HANDLE_SIZE/2;
		float flWidth = w-HANDLE_SIZE;

		CRootPanel::PaintRect(flLeft, y+h/2, flWidth, 1, Color(200, 200, 200, 255));

		CRootPanel::PaintRect(flLeft, y+h/2-5, 1, 10, Color(200, 200, 200, 255));
		CRootPanel::PaintRect(flLeft + flWidth, y+h/2-5, 1, 10, Color(200, 200, 200, 255));

		CRootPanel::PaintRect(HandleX()+2, HandleY()+2, m_flHandleSize-4, HANDLE_SIZE-4, g_clrBoxHi, 2);

		CBaseControl::Paint(x, y, w, h);
	}
	else
	{
		float flTop = y+HANDLE_SIZE/2;
		float flHeight = h-HANDLE_SIZE;

		CRootPanel::PaintRect(x+w/2, flTop, 1, flHeight, Color(200, 200, 200, 255));

		CRootPanel::PaintRect(x+w/2-5, flTop, 10, 1, Color(200, 200, 200, 255));
		CRootPanel::PaintRect(x+w/2-5, flTop + flHeight, 10, 1, Color(200, 200, 200, 255));

		CRootPanel::PaintRect(HandleX()+2, HandleY()+2, HANDLE_SIZE-4, m_flHandleSize-4, g_clrBoxHi, 2);

		CBaseControl::Paint(x, y, w, h);
	}
}

void CScrollBar::SetVisible(bool bVisible)
{
	BaseClass::SetVisible(bVisible);

	CPanel* pPanel = GetParent().Downcast<CPanel>();
	if (pPanel && bVisible)
	{
		FRect rParent = pPanel->GetAbsDimensions();
		FRect rParentChildren = pPanel->GetControlBounds();

		if (m_bHorizontal)
			m_flHandleSize = (rParent.w/rParentChildren.w)*(GetWidth()-HANDLE_SIZE*2);
		else
			m_flHandleSize = (rParent.h/rParentChildren.h)*(GetHeight()-HANDLE_SIZE*2);

		if (m_flHandleSize < HANDLE_SIZE)
			m_flHandleSize = HANDLE_SIZE;
	}
}

void CScrollBar::Think()
{
	BaseClass::Think();

	if (m_bMovingHandle)
	{
		int mx, my;
		CRootPanel::GetFullscreenMousePos(mx, my);

		float x, y, w, h;
		GetAbsDimensions(x, y, w, h);

		if (m_bHorizontal)
			m_flHandlePositionGoal = RemapValClamped((float)mx, (float)x + m_flHandleGrabOffset, (float)(x + w) - m_flHandleSize + m_flHandleGrabOffset, 0.0f, 1.0f);
		else
			m_flHandlePositionGoal = RemapValClamped((float)my, (float)y + m_flHandleGrabOffset, (float)(y + h) - m_flHandleSize + m_flHandleGrabOffset, 0.0f, 1.0f);
	}

	m_flHandlePosition = Approach(m_flHandlePositionGoal, m_flHandlePosition, (float)CRootPanel::Get()->GetFrameTime()*10);
}

bool CScrollBar::MousePressed(int code, int mx, int my)
{
	float x, y, w, h;
	GetAbsDimensions(x, y, w, h);

	float hx, hy;
	hx = HandleX();
	hy = HandleY();

	if (m_bHorizontal)
	{
		if (mx >= hx && mx < hx + m_flHandleSize && my >= hy && my < hy + HANDLE_SIZE)
		{
			m_bMovingHandle = true;
			m_flHandleGrabOffset = mx - hx;
		}
		else
		{
			m_flHandlePositionGoal = RemapValClamped((float)mx, (float)x + m_flHandleSize/2, (float)(x + w - HANDLE_SIZE/2), 0.0f, 1.0f);

//			if (m_pSelectedListener)
//				m_pfnSelectedCallback(m_pSelectedListener, "");
		}
	}
	else
	{
		if (mx >= hx && mx < hx + HANDLE_SIZE && my >= hy && my < hy + m_flHandleSize)
		{
			m_bMovingHandle = true;
			m_flHandleGrabOffset = my - hy;
		}
		else
		{
			m_flHandlePositionGoal = RemapValClamped((float)my, (float)y + HANDLE_SIZE/2, (float)(y + h - m_flHandleSize/2), 0.0f, 1.0f);

//			if (m_pSelectedListener)
//				m_pfnSelectedCallback(m_pSelectedListener, "");
		}
	}

	return true;
}

bool CScrollBar::MouseReleased(int code, int mx, int my)
{
	float x, y, w, h;
	GetAbsDimensions(x, y, w, h);

	if (m_bMovingHandle)
	{
		DoneMovingHandle();
		return true;
	}

	return CBaseControl::MouseReleased(code, mx, my);
}

void CScrollBar::CursorOut()
{
	if (m_bMovingHandle)
	{
		int mx, my;
		CRootPanel::GetFullscreenMousePos(mx, my);

		float x, y, w, h;
		GetAbsDimensions(x, y, w, h);

		// If the mouse went out of the left or right side, make sure we're all the way to that side.
		if (m_bHorizontal)
		{
			if (mx < x || mx > x + w)
				m_flHandlePositionGoal = RemapValClamped((float)mx, (float)x, (float)(x + w), 0.0f, 1.0f);
		}
		else
		{
			if (my < y || my > y + h)
				m_flHandlePositionGoal = RemapValClamped((float)my, (float)y, (float)(y + h), 0.0f, 1.0f);
		}

		DoneMovingHandle();
	}
}

void CScrollBar::DoneMovingHandle()
{
	m_bMovingHandle = false;

//	if (m_pSelectedListener)
//		m_pfnSelectedCallback(m_pSelectedListener, "");
}

float CScrollBar::HandleX()
{
	if (m_bHorizontal)
	{
		float x, y, w, h;
		GetAbsDimensions(x, y, w, h);

		float flLeft = x+HANDLE_SIZE/2;
		float flWidth = w-HANDLE_SIZE;
		return flLeft + (flWidth-m_flHandleSize+HANDLE_SIZE)*m_flHandlePosition - HANDLE_SIZE/2;
	}
	else
	{
		float x, y, w, h;
		GetAbsDimensions(x, y, w, h);

		return x+w/2-HANDLE_SIZE/2;
	}
}

float CScrollBar::HandleY()
{
	if (m_bHorizontal)
	{
		float x, y, w, h;
		GetAbsDimensions(x, y, w, h);

		return y+h/2-HANDLE_SIZE/2;
	}
	else
	{
		float x, y, w, h;
		GetAbsDimensions(x, y, w, h);

		float flTop = y+HANDLE_SIZE/2;
		float flHeight = h-HANDLE_SIZE;
		return flTop + (flHeight-m_flHandleSize+HANDLE_SIZE)*m_flHandlePosition - HANDLE_SIZE/2;
	}
}
