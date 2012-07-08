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
#include "crunch/cavity.h"

class CCavityPanel : public glgui::CMovablePanel, public IWorkListener
{
	DECLARE_CLASS(CCavityPanel, glgui::CMovablePanel);

public:
							CCavityPanel(CConversionScene* pScene);

public:
	virtual void			SetVisible(bool bVisible);

	virtual void			Layout();

	virtual bool			KeyPressed(int iKey);

	virtual void			BeginProgress();
	virtual void			SetAction(const tstring& sAction, size_t iTotalProgress);
	virtual void			WorkProgress(size_t iProgress, bool bForceDraw = false);
	virtual void			EndProgress();

	virtual bool			IsGenerating() { return m_oGenerator.IsGenerating(); }
	virtual bool			DoneGenerating() { return m_oGenerator.DoneGenerating(); }

	EVENT_CALLBACK(CCavityPanel, Generate);
	EVENT_CALLBACK(CCavityPanel, SaveMapDialog);
	EVENT_CALLBACK(CCavityPanel, SaveMapFile);

	static void				Open(CConversionScene* pScene);
	static glgui::CControl<CCavityPanel>	Get();

protected:
	CConversionScene*		m_pScene;

	CCavityGenerator		m_oGenerator;

	glgui::CControl<glgui::CLabel>			m_hMaterialsLabel;
	glgui::CControl<glgui::CTree>			m_hMaterials;

	glgui::CControl<glgui::CButton>			m_hGenerate;
	glgui::CControl<glgui::CButton>			m_hSave;

	static glgui::CControl<CCavityPanel>	s_hCavityPanel;
};
