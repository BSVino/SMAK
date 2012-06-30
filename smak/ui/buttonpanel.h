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

#pragma once

#include <glgui/panel.h>

typedef enum
{
	BA_TOP,
	BA_BOTTOM,
} buttonalignment_t;

class CButtonPanel : public glgui::CPanel
{
public:
							CButtonPanel(buttonalignment_t eAlign);

public:
	virtual void			Layout();

	virtual glgui::CControl<glgui::CButton>	AddButton(glgui::CButton* pButton, const tstring& sHints, bool bNewSection, glgui::IEventListener* pListener = NULL, glgui::IEventListener::Callback pfnCallback = NULL);

	virtual void			Think();
	virtual void			Paint(float x, float y, float w, float h);

protected:
	buttonalignment_t		m_eAlign;

	tvector<float>			m_aflSpaces;
	tvector<glgui::CControl<glgui::CButton>>	m_ahButtons;
	tvector<glgui::CControl<glgui::CLabel>>		m_ahHints;
};
