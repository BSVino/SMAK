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

#include "button.h"

#include <tinker/shell.h>
#include <tinker/keys.h>

#include "rootpanel.h"

using namespace glgui;

CButton::CButton(const tstring& sText, bool bToggle, const tstring& sFont, size_t iSize)
	: CLabel(10, 10, 100, 30, sText, sFont, iSize)
{
	m_bToggle = bToggle;
	m_bToggleOn = false;
	m_bDown = false;
	m_flHighlightGoal = m_flHighlight = 0;
	m_pClickListener = NULL;
	m_pfnClickCallback = NULL;
	m_pUnclickListener = NULL;
	m_pfnUnclickCallback = NULL;
	m_clrButton = g_clrBox;
	m_clrDown = g_clrBoxHi;
}

CButton::CButton(float x, float y, float w, float h, const tstring& sText, bool bToggle, const tstring& sFont, size_t iSize)
	: CLabel(x, y, w, h, sText, sFont, iSize)
{
	m_bToggle = bToggle;
	m_bToggleOn = false;
	m_bDown = false;
	m_flHighlightGoal = m_flHighlight = 0;
	m_pClickListener = NULL;
	m_pfnClickCallback = NULL;
	m_pUnclickListener = NULL;
	m_pfnUnclickCallback = NULL;
	m_clrButton = g_clrBox;
	m_clrDown = g_clrBoxHi;
}

void CButton::SetToggleState(bool bState)
{
	if (m_bDown == bState)
		return;

	m_bToggleOn = m_bDown = bState;
}

bool CButton::Push()
{
	if (!m_bEnabled)
		return false;

	if (m_bDown && !m_bToggle)
		return false;

	m_bDown = true;

	if (m_bToggle)
		m_bToggleOn = !m_bToggleOn;

	return true;
}

bool CButton::Pop(bool bRegister, bool bReverting)
{
	if (!m_bDown)
		return false;

	if (m_bToggle)
	{
		if (bReverting)
			m_bToggleOn = !m_bToggleOn;

		if (m_bToggleOn)
			SetState(true, bRegister);
		else
			SetState(false, bRegister);
	}
	else
		SetState(false, bRegister);

	return true;
}

void CButton::SetState(bool bDown, bool bRegister)
{
	m_bDown = bDown;

	if (m_bToggle)
		m_bToggleOn = bDown;

	if (m_bToggle && !m_bToggleOn)
	{
		if (bRegister && m_pUnclickListener && m_pfnUnclickCallback)
			m_pfnUnclickCallback(m_pUnclickListener, m_sUnclickArgs);
	}
	else
	{
		if (bRegister && m_pClickListener && m_pfnClickCallback)
			m_pfnClickCallback(m_pClickListener, m_sClickArgs);
	}
}

void CButton::SetClickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback, const tstring& sArgs)
{
	TAssert(pListener && pfnCallback || !pListener && !pfnCallback);
	m_pClickListener = pListener;
	m_pfnClickCallback = pfnCallback;
	m_sClickArgs = sArgs;
}

void CButton::SetUnclickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback, const tstring& sArgs)
{
	TAssert(pListener && pfnCallback || !pListener && !pfnCallback);
	m_pUnclickListener = pListener;
	m_pfnUnclickCallback = pfnCallback;
	m_sUnclickArgs = sArgs;
}

bool CButton::MousePressed(int code, int mx, int my)
{
	if (!IsVisible())
		return CLabel::MousePressed(code, mx, my);

	bool bUsed = false;
	if (code == TINKER_KEY_MOUSE_LEFT)
	{
		bUsed = Push();
		CRootPanel::Get()->SetButtonDown(m_hThis);
	}
	return bUsed;
}

bool CButton::MouseReleased(int code, int mx, int my)
{
	if (!IsVisible())
		return CLabel::MouseReleased(code, mx, my);

	if (CRootPanel::Get()->GetButtonDown() != this)
		return false;

	bool bUsed = false;
	if (code == TINKER_KEY_MOUSE_LEFT)
	{
		bUsed = Pop();
		CRootPanel::Get()->SetButtonDown(CControlHandle());
	}
	return bUsed;
}

void CButton::CursorIn()
{
	CLabel::CursorIn();

	m_flHighlightGoal = 1;
}

void CButton::CursorOut()
{
	CLabel::CursorOut();

	m_flHighlightGoal = 0;
}

void CButton::SetToggleButton(bool bToggle)
{
	if (m_bToggle == bToggle)
		return;

	m_bToggle = bToggle;

	SetState(false, false);
}

void CButton::Think()
{
	m_flHighlight = Approach(m_flHighlightGoal, m_flHighlight, (float)(CRootPanel::Get()->GetFrameTime()*3));

	CLabel::Think();
}

void CButton::Paint(float x, float y, float w, float h)
{
	if (!IsVisible())
		return;

	PaintButton(x, y, w, h);

	// Now paint the text which appears on the button.
	CLabel::Paint(x, y, w, h);
}

void CButton::PaintButton(float x, float y, float w, float h)
{
	if (m_bDown)
	{
		CRootPanel::PaintRect(x, y, w, h, m_clrDown, 3);
	}
	else
	{
		Color clrBox = m_clrButton;
		if (m_bEnabled)
			clrBox.SetAlpha((int)RemapVal(m_flHighlight, 0, 1, 125, 255));
		CRootPanel::PaintRect(x, y, w, h, clrBox, 2, m_bEnabled && m_flHighlightGoal > 1);
	}
}
