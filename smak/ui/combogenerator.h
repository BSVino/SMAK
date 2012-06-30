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
#include <glgui/button.h>
#include "crunch/crunch.h"

class CMeshInstancePicker;

class COptionsButton : public glgui::CButton, public glgui::IEventListener
{
public:
	class COptionsPanel : public glgui::CPanel
	{
		DECLARE_CLASS(COptionsPanel, glgui::CPanel);

	public:
							COptionsPanel(COptionsButton* pButton);

	public:
		virtual void		Layout();
		virtual void		Paint(float x, float y, float w, float h);

	protected:
		COptionsButton*		m_pButton;

		glgui::CControl<glgui::CButton>	m_hOkay;
	};

public:
						COptionsButton();

public:
	EVENT_CALLBACK(COptionsButton, Open);
	EVENT_CALLBACK(COptionsButton, Close);

	COptionsPanel*		GetOptionsPanel() { return m_pPanel.DowncastStatic<COptionsPanel>(); }

protected:
	glgui::CControlResource	m_pPanel;
};

class CComboGeneratorPanel : public glgui::CMovablePanel, public IWorkListener
{
	DECLARE_CLASS(CComboGeneratorPanel, glgui::CMovablePanel);

public:
								CComboGeneratorPanel(CConversionScene* pScene);

public:
	virtual void				SetVisible(bool bVisible);

	virtual void				Layout();
	virtual void				UpdateScene();

	virtual void				Think();

	virtual void				Paint(float x, float y, float w, float h);

	virtual bool				KeyPressed(int iKey);

	virtual void				BeginProgress();
	virtual void				SetAction(const tstring& sAction, size_t iTotalProgress);
	virtual void				WorkProgress(size_t iProgress, bool bForceDraw = false);
	virtual void				EndProgress();

	virtual bool				IsGenerating() { return m_oGenerator.IsGenerating(); }
	virtual bool				DoneGenerating() { return m_oGenerator.DoneGenerating(); }

	EVENT_CALLBACK(CComboGeneratorPanel,	Generate);
	EVENT_CALLBACK(CComboGeneratorPanel,	SaveMapDialog);
	EVENT_CALLBACK(CComboGeneratorPanel,	SaveMapFile);
	EVENT_CALLBACK(CComboGeneratorPanel,	AddLoRes);
	EVENT_CALLBACK(CComboGeneratorPanel,	AddHiRes);
	EVENT_CALLBACK(CComboGeneratorPanel,	RemoveLoRes);
	EVENT_CALLBACK(CComboGeneratorPanel,	RemoveHiRes);
	EVENT_CALLBACK(CComboGeneratorPanel,	AddLoResMesh);
	EVENT_CALLBACK(CComboGeneratorPanel,	AddHiResMesh);
	EVENT_CALLBACK(CComboGeneratorPanel,	DroppedLoResMesh);
	EVENT_CALLBACK(CComboGeneratorPanel,	DroppedHiResMesh);

	static void					Open(CConversionScene* pScene);
	static glgui::CControl<CComboGeneratorPanel> Get() { return s_hComboGeneratorPanel; }

protected:
	CConversionScene*			m_pScene;

	CTexelGenerator				m_oGenerator;

	glgui::CControl<glgui::CLabel>				m_hSizeLabel;
	glgui::CControl<glgui::CScrollSelector<int>>	m_hSizeSelector;

	glgui::CControl<glgui::CLabel>				m_hLoResLabel;
	glgui::CControl<glgui::CTree>				m_hLoRes;

	glgui::CControl<glgui::CLabel>				m_hHiResLabel;
	glgui::CControl<glgui::CTree>				m_hHiRes;

	tvector<CConversionMeshInstance*>	m_apLoResMeshes;
	tvector<CConversionMeshInstance*>	m_apHiResMeshes;

	glgui::CControl<glgui::CButton>				m_hAddLoRes;
	glgui::CControl<glgui::CButton>				m_hAddHiRes;

	glgui::CControl<glgui::CButton>				m_hRemoveLoRes;
	glgui::CControl<glgui::CButton>				m_hRemoveHiRes;

	glgui::CControl<glgui::CLabel>				m_hDiffuseLabel;
	glgui::CControl<glgui::CCheckBox>			m_hDiffuseCheckBox;

	glgui::CControl<glgui::CLabel>				m_hAOLabel;
	glgui::CControl<glgui::CCheckBox>			m_hAOCheckBox;

	glgui::CControl<glgui::CLabel>				m_hNormalLabel;
	glgui::CControl<glgui::CCheckBox>			m_hNormalCheckBox;

	glgui::CControl<COptionsButton>				m_hAOOptions;

	glgui::CControl<glgui::CLabel>				m_hBleedLabel;
	glgui::CControl<glgui::CScrollSelector<int>>	m_hBleedSelector;

	glgui::CControl<glgui::CLabel>				m_hSamplesLabel;
	glgui::CControl<glgui::CScrollSelector<int>>	m_hSamplesSelector;

	glgui::CControl<glgui::CLabel>				m_hFalloffLabel;
	glgui::CControl<glgui::CScrollSelector<float>>	m_hFalloffSelector;

	glgui::CControl<glgui::CLabel>				m_hRandomLabel;
	glgui::CControl<glgui::CCheckBox>			m_hRandomCheckBox;

	glgui::CControl<glgui::CLabel>				m_hGroundOcclusionLabel;
	glgui::CControl<glgui::CCheckBox>			m_hGroundOcclusionCheckBox;

	glgui::CControl<glgui::CButton>				m_hGenerate;
	glgui::CControl<glgui::CButton>				m_hSave;

	glgui::CControl<CMeshInstancePicker>		m_hMeshInstancePicker;

	static glgui::CControl<CComboGeneratorPanel>		s_hComboGeneratorPanel;
};
