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

#include "crunch/normal.h"

class CNormalPanel : public glgui::CMovablePanel
{
	DECLARE_CLASS(CNormalPanel, glgui::CMovablePanel);

public:
								CNormalPanel(CConversionScene* pScene);

public:
	virtual void				SetVisible(bool bVisible);

	virtual void				Layout();
	virtual void				UpdateScene();

	virtual void				Think();

	virtual void				Paint(float x, float y, float w, float h);

	virtual bool				KeyPressed(int iKey);

	virtual bool				IsGenerating() { return m_oGenerator.IsGenerating(); }
	virtual bool				DoneGenerating() { return m_oGenerator.DoneGenerating(); }

	EVENT_CALLBACK(CNormalPanel,	Generate);
	EVENT_CALLBACK(CNormalPanel,	SaveMapDialog);
	EVENT_CALLBACK(CNormalPanel,	SaveMapFile);
	EVENT_CALLBACK(CNormalPanel,	SetupNormal2);
	EVENT_CALLBACK(CNormalPanel,	UpdateNormal2);

	static void					Open(CConversionScene* pScene);
	static glgui::CControl<CNormalPanel>		Get() { return s_hNormalPanel; }

protected:
	CConversionScene*			m_pScene;

	CNormalGenerator			m_oGenerator;

	glgui::CControl<glgui::CLabel>				m_hMaterialsLabel;
	glgui::CControl<glgui::CTree>				m_hMaterials;

	glgui::CControl<glgui::CLabel>				m_hProgressLabel;

	glgui::CControl<glgui::CScrollSelector<float>>	m_hDepthSelector;
	glgui::CControl<glgui::CLabel>				m_hDepthLabel;

	glgui::CControl<glgui::CScrollSelector<float>>	m_hHiDepthSelector;
	glgui::CControl<glgui::CLabel>				m_hHiDepthLabel;

	glgui::CControl<glgui::CScrollSelector<float>>	m_hMidDepthSelector;
	glgui::CControl<glgui::CLabel>				m_hMidDepthLabel;

	glgui::CControl<glgui::CScrollSelector<float>>	m_hLoDepthSelector;
	glgui::CControl<glgui::CLabel>				m_hLoDepthLabel;

	glgui::CControl<glgui::CButton>				m_hSave;

	glgui::CControl<class CMaterialPicker>		m_hMaterialPicker;

	static glgui::CControl<CNormalPanel>		s_hNormalPanel;
};
