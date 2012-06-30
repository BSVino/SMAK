/*
SMAK - The Super Model Army Knife
Copyright (C) 2012, Lunar Workshop, Inc

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "progressbar.h"

#include <glgui/label.h>
#include <glgui/rootpanel.h>

#define BTN_HEIGHT 32

using namespace glgui;

CControl<CProgressBar> CProgressBar::s_hProgressBar;

CProgressBar::CProgressBar()
	: CPanel(0, 0, 100, 100)
{
	SetVisible(false);

	m_hAction = AddControl(new CLabel(0, 0, 200, BTN_HEIGHT, ""));
	m_hAction->SetWrap(false);

	Layout();
}

void CProgressBar::Layout()
{
	if (!GetParent())
		return;

	SetSize(GetParent()->GetWidth()/2, BTN_HEIGHT);
	SetPos(GetParent()->GetWidth()/4, GetParent()->GetHeight()/5);

	m_hAction->SetSize(GetWidth(), BTN_HEIGHT);
}

void CProgressBar::Paint(float x, float y, float w, float h)
{
	float flTotalProgress = (float)m_iCurrentProgress/(float)m_iTotalProgress;

	if (flTotalProgress > 1)
		flTotalProgress = 1;

	CPanel::PaintRect(x, y, w, h);
	CPanel::PaintRect(x+10, y+10, (w-20)*flTotalProgress, h-20, g_clrBoxHi);

	m_hAction->Paint();
}

void CProgressBar::SetTotalProgress(size_t iProgress)
{
	m_iTotalProgress = iProgress;
	SetProgress(0);
}

void CProgressBar::SetProgress(size_t iProgress, const tstring& sAction)
{
	m_iCurrentProgress = iProgress;

	SetAction(sAction);
}

void CProgressBar::SetAction(const tstring& sAction)
{
	if (sAction.length())
		m_sAction = sAction;

	m_hAction->SetText(m_sAction);

	if (m_iTotalProgress)
	{
		tstring sProgress;
		sProgress = sprintf(" %d%%", m_iCurrentProgress*100/m_iTotalProgress);
		m_hAction->AppendText(sProgress);
	}
}

CProgressBar* CProgressBar::Get()
{
	if (!s_hProgressBar)
		s_hProgressBar = RootPanel()->AddControl(new CProgressBar());

	return s_hProgressBar;
}
