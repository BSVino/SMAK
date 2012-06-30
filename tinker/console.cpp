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

#include "console.h"

#include <tinker_platform.h>
#include <strutils.h>
#include <glgui/textfield.h>
#include <glgui/label.h>
#include <glgui/rootpanel.h>

#include "application.h"
#include "keys.h"
#include "cvar.h"

#ifdef _DEBUG
#define DEV_VALUE "1"
#else
#define DEV_VALUE "0"
#endif

CVar developer("developer", DEV_VALUE);

#undef DEV_VALUE

CConsole::CConsole()
	: glgui::CPanel(0, 0, 100, 100)
{
	m_bBackground = true;

	m_iHistory = -1;

	glgui::RootPanel()->AddControl(this, true);

	m_hOutput = AddControl(new glgui::CLabel(0, 0, 100, 100, ""));
	m_hOutput->SetAlign(glgui::CLabel::TA_BOTTOMLEFT);

	m_hInput = AddControl(new glgui::CTextField());
	m_hInput->SetContentsChangedListener(this, CommandChanged);
}

CConsole::~CConsole()
{
}

bool CConsole::IsVisible()
{
	if (developer.GetBool() && m_bBackground)
		return true;

	return BaseClass::IsVisible();
}

bool CConsole::IsChildVisible(CBaseControl* pChild)
{
	if (!BaseClass::IsVisible() && pChild == m_hInput)
		return false;

	return true;
}

void CConsole::SetVisible(bool bVisible)
{
	BaseClass::SetVisible(bVisible);

	m_hInput->SetFocus(bVisible);
}

bool CConsole::IsOpen()
{
	return BaseClass::IsVisible();
}

bool CConsole::IsCursorListener()
{
	// Don't interfere with mouse events, we're just showing an overlay.
	if (developer.GetBool() && !BaseClass::IsVisible())
		return false;

	return BaseClass::IsCursorListener();
}

void CConsole::Layout()
{
	SetSize(glgui::CRootPanel::Get()->GetWidth()/3, glgui::CRootPanel::Get()->GetHeight()-150);
	SetPos(glgui::CRootPanel::Get()->GetWidth()/6, 0);

	m_hInput->SetSize(GetWidth(), 20);
	m_hInput->SetPos(0, GetHeight()-20);

	m_hOutput->SetSize(GetWidth(), GetHeight()-24);
	m_hOutput->SetPos(0, 0);

	BaseClass::Layout();
}

void CConsole::Paint(float x, float y, float w, float h)
{
	if (!CApplication::Get()->IsOpen())
		return;

	if (!BaseClass::IsVisible() && developer.GetBool())
	{
		int iAlpha = m_hOutput->GetAlpha();
		m_hOutput->SetAlpha(100);
		m_hOutput->Paint();
		m_hOutput->SetAlpha(iAlpha);
		return;
	}

	if (!BaseClass::IsVisible())
		return;

	glgui::CRootPanel::PaintRect(x, y, w, h, Color(0, 0, 0, 200), 1, true);

	BaseClass::Paint(x, y, w, h);
}

void CConsole::PrintConsole(const tstring& sText)
{
	DebugPrint(sText.c_str());
	m_hOutput->AppendText(sText);

	if (m_hOutput->GetText().length() > 2500)
		m_hOutput->SetText(m_hOutput->GetText().substr(500));

	if (!CApplication::Get()->IsOpen())
		return;

	if (IsVisible())
		Layout();
}

bool CConsole::KeyPressed(int code, bool bCtrlDown)
{
	if (!IsOpen())
		return false;

	if (code == TINKER_KEY_ESCAPE)
	{
		m_hInput->SetText("");
		CApplication::Get()->CloseConsole();
		return true;
	}

	if (m_asHistory.size())
	{
		if (code == TINKER_KEY_DOWN)
		{
			if (m_iHistory >= 0 && m_iHistory < (int)m_asHistory.size()-1)
			{
				m_iHistory++;

				m_hInput->SetText(m_asHistory[m_iHistory]);
				m_hInput->SetCursorPosition(-1);
			}
			else if (m_iHistory == (int)m_asHistory.size()-1)
			{
				m_iHistory = -1;
				m_hInput->SetText("");
			}
		}
		else if (code == TINKER_KEY_UP)
		{
			if (m_iHistory == -1)
				m_iHistory = m_asHistory.size()-1;
			else if (m_iHistory > 1)
				m_iHistory--;

			m_hInput->SetText(m_asHistory[m_iHistory]);
			m_hInput->SetCursorPosition(-1);
		}
		else
			m_iHistory = -1;
	}

	bool bReturn = BaseClass::KeyPressed(code, bCtrlDown);

	if (bReturn && !(code == TINKER_KEY_ENTER || code == TINKER_KEY_KP_ENTER))
		return true;

	if (code == TINKER_KEY_ENTER || code == TINKER_KEY_KP_ENTER)
	{
		tstring sText = m_hInput->GetText();
		m_hInput->SetText("");

		PrintConsole(tstring("] ") + sText + "\n");

		CCommand::Run(sText);

		if (trim(sText).length())
			m_asHistory.push_back(trim(sText));
		m_iHistory = -1;

		return true;
	}

	return false;
}

bool CConsole::CharPressed(int iKey)
{
	if (!IsOpen())
		return false;

	if (iKey == '`')
	{
		CApplication::Get()->CloseConsole();
		return true;
	}

	return BaseClass::CharPressed(iKey);
}

void CConsole::CommandChangedCallback(const tstring& sArgs)
{
	tstring sInput = m_hInput->GetText();
	if (sInput.find(' ') != ~0)
	{
		m_hInput->ClearAutoCompleteCommands();
		return;
	}

	m_hInput->SetAutoCompleteCommands(CCommand::GetCommandsBeginningWith(sInput));
}

void CApplication::OpenConsole()
{
	if (!Get())
		return;

	CConsole* pConsole = Get()->GetConsole();
	pConsole->Layout();
	pConsole->SetVisible(true);

	glgui::CRootPanel::Get()->MoveToTop(pConsole);
}

void CApplication::CloseConsole()
{
	if (!Get())
		return;

	CConsole* pConsole = Get()->GetConsole();
	pConsole->SetVisible(false);
}

void CApplication::ToggleConsole()
{
	if (!Get())
		return;

	CConsole* pConsole = Get()->GetConsole();
	if (IsConsoleOpen())
		CloseConsole();
	else
		OpenConsole();
}

bool CApplication::IsConsoleOpen()
{
	if (!Get())
		return false;

	CConsole* pConsole = Get()->GetConsole();
	return pConsole->IsOpen();
}

CConsole* CApplication::GetConsole()
{
	if (m_pConsole == NULL)
	{
		m_pConsole = new CConsole();
		m_pConsole->SetVisible(false);

		if (developer.GetBool())
			TMsg("Developer mode ON.\n");
	}

	return m_pConsole;
}
