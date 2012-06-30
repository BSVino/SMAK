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

#include "buttonpanel.h"

#include <glgui/button.h>
#include <glgui/rootpanel.h>

using namespace glgui;

#define BTN_HEIGHT 32
#define BTN_SPACE 8
#define BTN_SECTION 18

CButtonPanel::CButtonPanel(buttonalignment_t eAlign)
: CPanel(0, 0, BTN_HEIGHT, BTN_HEIGHT)
{
	m_eAlign = eAlign;
}

void CButtonPanel::Layout()
{
	float flX = 0;

	for (size_t i = 0; i < m_ahButtons.size(); i++)
	{
		CBaseControl* pButton = m_ahButtons[i];

		if (!pButton->IsVisible())
			continue;

		pButton->SetSize(BTN_HEIGHT, BTN_HEIGHT);
		pButton->SetPos(flX, 0);

		CBaseControl* pHint = m_ahHints[i];
		pHint->SetPos(flX + BTN_HEIGHT/2 - pHint->GetWidth()/2, -18);

		flX += BTN_HEIGHT + m_aflSpaces[i];
	}

	SetSize(flX - m_aflSpaces[m_aflSpaces.size()-1], BTN_HEIGHT);
	if (m_eAlign == BA_TOP)
		SetPos(CRootPanel::Get()->GetWidth()/2 - GetWidth()/2, BTN_HEIGHT + BTN_SPACE);
	else
		SetPos(CRootPanel::Get()->GetWidth()/2 - GetWidth()/2, CRootPanel::Get()->GetHeight() - BTN_HEIGHT*2 - BTN_SPACE);

	CPanel::Layout();
}

glgui::CControl<glgui::CButton> CButtonPanel::AddButton(CButton* pButton, const tstring& sHints, bool bNewSection, IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	CControl<glgui::CButton> hButton = AddControl(pButton);
	m_ahButtons.push_back(hButton);

	m_aflSpaces.push_back((float)(bNewSection?BTN_SECTION:BTN_SPACE));

	CControl<CLabel> hHint = AddControl(new CLabel(0, 0, 0, 0, sHints));
	hHint->SetAlpha(0);
	hHint->EnsureTextFits();
	m_ahHints.push_back(hHint);

	if (pListener)
	{
		hButton->SetClickedListener(pListener, pfnCallback);
		hButton->SetUnclickedListener(pListener, pfnCallback);
	}

	return hButton;
}

void CButtonPanel::Think()
{
	int mx, my;
	CRootPanel::GetFullscreenMousePos(mx, my);

	for (size_t i = 0; i < m_ahButtons.size(); i++)
	{
		if (!m_ahButtons[i]->IsVisible())
		{
			m_ahHints[i]->SetAlpha((int)Approach(0.0f, (float)m_ahHints[i]->GetAlpha(), 30.0f));
			continue;
		}

		float x = 0, y = 0, w = 0, h = 0;
		m_ahButtons[i]->GetAbsDimensions(x, y, w, h);

		float flAlpha = (float)m_ahHints[i]->GetAlpha();

		if (mx >= x && my >= y && mx < x + w && my < y + h)
			m_ahHints[i]->SetAlpha((int)Approach(255.0f, flAlpha, 30.0f));
		else
			m_ahHints[i]->SetAlpha((int)Approach(0.0f, flAlpha, 30.0f));
	}

	CPanel::Think();
}

void CButtonPanel::Paint(float x, float y, float w, float h)
{
	CPanel::Paint(x, y, w, h);
}

