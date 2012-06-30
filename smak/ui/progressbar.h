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

class CProgressBar : public glgui::CPanel
{
	DECLARE_CLASS(CProgressBar, glgui::CPanel);

public:
							CProgressBar();

public:
	void					Layout();
	void					Paint(float x, float y, float w, float h);

	void					SetTotalProgress(size_t iProgress);
	void					SetProgress(size_t iProgress, const tstring& sAction = "");
	void					SetAction(const tstring& sAction);

	static CProgressBar*	Get();

protected:
	size_t					m_iTotalProgress;
	size_t					m_iCurrentProgress;

	glgui::CControl<glgui::CLabel>	m_hAction;
	tstring					m_sAction;

	static glgui::CControl<CProgressBar>	s_hProgressBar;
};
