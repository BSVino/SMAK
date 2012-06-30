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

#include <glgui/movablepanel.h>

class CHelpPanel : public glgui::CMovablePanel
{
	DECLARE_CLASS(CHelpPanel, glgui::CMovablePanel);

public:
							CHelpPanel();

public:
	virtual void			Layout();
	virtual void			Paint(float x, float y, float w, float h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();

protected:
	glgui::CControl<glgui::CLabel>			m_hInfo;

	static glgui::CControl<CHelpPanel>		s_hHelpPanel;
};

class CAboutPanel : public glgui::CMovablePanel
{
	DECLARE_CLASS(CAboutPanel, glgui::CMovablePanel);

public:
							CAboutPanel();

public:
	virtual void			Layout();
	virtual void			Paint(float x, float y, float w, float h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();

protected:
	glgui::CControl<glgui::CLabel>			m_hInfo;

	static glgui::CControl<CAboutPanel>		s_hAboutPanel;
};
