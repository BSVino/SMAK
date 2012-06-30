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

#include "aogenerator.h"

#include <glgui/label.h>
#include <glgui/selector.h>
#include <glgui/checkbox.h>
#include <glgui/filedialog.h>
#include <textures/materiallibrary.h>

#include "progressbar.h"

using namespace glgui;

CControl<CAOPanel> CAOPanel::s_hAOPanel;
CControl<CAOPanel> CAOPanel::s_hColorAOPanel;

CAOPanel::CAOPanel(bool bColor, CConversionScene* pScene)
	: CMovablePanel(bColor?"Color AO generator":"AO generator"), m_oGenerator(pScene)
{
	m_bColor = bColor;

	m_pScene = pScene;

	m_hSizeLabel = AddControl(new CLabel(0, 0, 32, 32, "Size"));

	m_hSizeSelector = AddControl(new CScrollSelector<int>());
#ifdef _DEBUG
	m_hSizeSelector->AddSelection(CScrollSelection<int>(16, "16x16"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(32, "32x32"));
#endif
	m_hSizeSelector->AddSelection(CScrollSelection<int>(64, "64x64"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(128, "128x128"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(256, "256x256"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(512, "512x512"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(1024, "1024x1024"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(2048, "2048x2048"));
	//m_hSizeSelector->AddSelection(CScrollSelection<int>(4096, "4096x4096"));
	m_hSizeSelector->SetSelection(2);

	m_hEdgeBleedLabel = AddControl(new CLabel(0, 0, 32, 32, "Edge Bleed"));

	m_hEdgeBleedSelector = AddControl(new CScrollSelector<int>());
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(0, "0"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(1, "1"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(2, "2"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(3, "3"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(4, "4"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(5, "5"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(6, "6"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(7, "7"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(8, "8"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(9, "9"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(10, "10"));
	m_hEdgeBleedSelector->SetSelection(1);

	if (!m_bColor)
	{
		m_hAOMethodLabel = AddControl(new CLabel(0, 0, 32, 32, "Method"));

		m_hAOMethodSelector = AddControl(new CScrollSelector<int>());
		m_hAOMethodSelector->AddSelection(CScrollSelection<int>(AOMETHOD_SHADOWMAP, "Shadow map (fast!)"));
		m_hAOMethodSelector->AddSelection(CScrollSelection<int>(AOMETHOD_RAYTRACE, "Raytraced (slow!)"));
		m_hAOMethodSelector->SetSelectedListener(this, AOMethod);

		m_hRayDensityLabel = AddControl(new CLabel(0, 0, 32, 32, "Ray Density"));

		m_hRayDensitySelector = AddControl(new CScrollSelector<int>());
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(5, "5"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(6, "6"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(7, "7"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(8, "8"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(9, "9"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(10, "10"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(11, "11"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(12, "12"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(13, "13"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(14, "14"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(15, "15"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(16, "16"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(17, "17"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(18, "18"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(19, "19"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(20, "20"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(21, "21"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(22, "22"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(23, "23"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(24, "24"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(25, "25"));
		m_hRayDensitySelector->SetSelection(15);

		m_hFalloffLabel = AddControl(new CLabel(0, 0, 32, 32, "Ray Falloff"));

		m_hFalloffSelector = AddControl(new CScrollSelector<float>());
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.01f, "0.01"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.05f, "0.05"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.1f, "0.1"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.25f, "0.25"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.5f, "0.5"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.75f, "0.75"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(1.0f, "1.0"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(1.25f, "1.25"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(1.5f, "1.5"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(1.75f, "1.75"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(2.0f, "2.0"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(2.5f, "2.5"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(3.0f, "3.0"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(4.0f, "4.0"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(5.0f, "5.0"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(6.0f, "6.0"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(7.5f, "7.5"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(10.0f, "10"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(25.0f, "25"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(50.0f, "50"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(100.0f, "100"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(250.0f, "250"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(500.0f, "500"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(1000.0f, "1000"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(2500.0f, "2500"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(5000.0f, "5000"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(10000.0f, "10000"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(-1.0f, "No falloff"));
		m_hFalloffSelector->SetSelection(6);

		m_hLightsLabel = AddControl(new CLabel(0, 0, 32, 32, "Lights"));

		m_hLightsSelector = AddControl(new CScrollSelector<int>());
		m_hLightsSelector->AddSelection(CScrollSelection<int>(500, "500"));
		m_hLightsSelector->AddSelection(CScrollSelection<int>(1000, "1000"));
		m_hLightsSelector->AddSelection(CScrollSelection<int>(1500, "1500"));
		m_hLightsSelector->AddSelection(CScrollSelection<int>(2000, "2000"));
		m_hLightsSelector->AddSelection(CScrollSelection<int>(2500, "2500"));
		m_hLightsSelector->AddSelection(CScrollSelection<int>(3000, "3000"));
		m_hLightsSelector->SetSelection(3);

		m_hRandomLabel = AddControl(new CLabel(0, 0, 32, 32, "Randomize rays"));

		m_hRandomCheckBox = AddControl(new CCheckBox());

		m_hCreaseLabel = AddControl(new CLabel(0, 0, 32, 32, "Crease edges"));

		m_hCreaseCheckBox = AddControl(new CCheckBox());

		m_hGroundOcclusionLabel = AddControl(new CLabel(0, 0, 32, 32, "Ground occlusion"));

		m_hGroundOcclusionCheckBox = AddControl(new CCheckBox());
	}

	m_hGenerate = AddControl(new CButton(0, 0, 100, 100, "Generate"));

	m_hGenerate->SetClickedListener(this, Generate);

	m_hSave = AddControl(new CButton(0, 0, 100, 100, "Save Map"));

	m_hSave->SetClickedListener(this, SaveMapDialog);
	m_hSave->SetVisible(false);

	Layout();
}

void CAOPanel::Layout()
{
	if (m_bColor)
		SetPos(GetParent()->GetWidth() - GetWidth() - 50, GetParent()->GetHeight() - GetHeight() - 150);
	else
		SetPos(GetParent()->GetWidth() - GetWidth() - 200, GetParent()->GetHeight() - GetHeight() - 100);

	float flSpace = 20;

	m_hSizeLabel->EnsureTextFits();

	float flSelectorSize = m_hSizeLabel->GetHeight() - 4;

	m_hSizeSelector->SetSize(GetWidth() - m_hSizeLabel->GetWidth() - flSpace, flSelectorSize);

	float flControlY = HEADER_HEIGHT;

	m_hSizeSelector->SetPos(GetWidth() - m_hSizeSelector->GetWidth() - flSpace/2, flControlY);
	m_hSizeLabel->SetPos(5, flControlY);

	flControlY += 30;

	m_hEdgeBleedLabel->EnsureTextFits();
	m_hEdgeBleedLabel->SetPos(5, flControlY);

	m_hEdgeBleedSelector->SetSize(GetWidth() - m_hEdgeBleedLabel->GetWidth() - flSpace, flSelectorSize);
	m_hEdgeBleedSelector->SetPos(GetWidth() - m_hEdgeBleedSelector->GetWidth() - flSpace/2, flControlY);

	if (!m_bColor)
	{
		flControlY += 30;

		m_hAOMethodLabel->EnsureTextFits();
		m_hAOMethodLabel->SetPos(5, flControlY);

		m_hAOMethodSelector->SetSize(GetWidth() - m_hAOMethodLabel->GetWidth() - flSpace, flSelectorSize);
		m_hAOMethodSelector->SetPos(GetWidth() - m_hAOMethodSelector->GetWidth() - flSpace/2, flControlY);

		bool bRaytracing = (m_hAOMethodSelector->GetSelectionValue() == AOMETHOD_RAYTRACE);
		m_hRayDensityLabel->SetVisible(bRaytracing);
		m_hRayDensitySelector->SetVisible(bRaytracing);

		m_hFalloffSelector->SetVisible(bRaytracing);
		m_hFalloffLabel->SetVisible(bRaytracing);

		m_hRandomCheckBox->SetVisible(bRaytracing);
		m_hRandomLabel->SetVisible(bRaytracing);

		if (bRaytracing)
		{
			flControlY += 30;

			m_hRayDensityLabel->EnsureTextFits();
			m_hRayDensityLabel->SetPos(5, flControlY);

			m_hRayDensitySelector->SetSize(GetWidth() - m_hRayDensityLabel->GetWidth() - flSpace, flSelectorSize);
			m_hRayDensitySelector->SetPos(GetWidth() - m_hRayDensitySelector->GetWidth() - flSpace/2, flControlY);

			flControlY += 30;

			m_hFalloffLabel->EnsureTextFits();
			m_hFalloffLabel->SetPos(5, flControlY);

			m_hFalloffSelector->SetSize(GetWidth() - m_hFalloffLabel->GetWidth() - flSpace, flSelectorSize);
			m_hFalloffSelector->SetPos(GetWidth() - m_hFalloffSelector->GetWidth() - flSpace/2, flControlY);

			flControlY += 30;

			m_hRandomLabel->EnsureTextFits();
			m_hRandomLabel->SetPos(25, flControlY);

			m_hRandomCheckBox->SetPos(10, flControlY + m_hRandomLabel->GetHeight()/2 - m_hRandomCheckBox->GetHeight()/2);
		}

		bool bShadowmapping = (m_hAOMethodSelector->GetSelectionValue() == AOMETHOD_SHADOWMAP);
		m_hLightsLabel->SetVisible(bShadowmapping);
		m_hLightsSelector->SetVisible(bShadowmapping);

		if (bShadowmapping)
		{
			flControlY += 30;

			m_hLightsLabel->EnsureTextFits();
			m_hLightsLabel->SetPos(5, flControlY);

			m_hLightsSelector->SetSize(GetWidth() - m_hLightsLabel->GetWidth() - flSpace, flSelectorSize);
			m_hLightsSelector->SetPos(GetWidth() - m_hLightsSelector->GetWidth() - flSpace/2, flControlY);
		}

		m_hGroundOcclusionLabel->SetVisible(bShadowmapping || bRaytracing);
		m_hGroundOcclusionCheckBox->SetVisible(bShadowmapping || bRaytracing);

		if (bShadowmapping || bRaytracing)
		{
			// If we're on the raytracing screen there was already the Randomize rays checkbox
			// so keep the spacing smaller.
			if (bRaytracing)
				flControlY += 20;
			else
				flControlY += 30;

			m_hGroundOcclusionLabel->EnsureTextFits();
			m_hGroundOcclusionLabel->SetPos(25, flControlY);

			m_hGroundOcclusionCheckBox->SetPos(10, flControlY + m_hGroundOcclusionLabel->GetHeight()/2 - m_hGroundOcclusionCheckBox->GetHeight()/2);
		}

		if (bShadowmapping || bRaytracing)
			flControlY += 20;
		else
			flControlY += 30;

		m_hCreaseLabel->EnsureTextFits();
		m_hCreaseLabel->SetPos(25, flControlY);

		m_hCreaseCheckBox->SetPos(10, flControlY + m_hCreaseLabel->GetHeight()/2 - m_hCreaseCheckBox->GetHeight()/2);
	}

	m_hSave->SetSize(GetWidth()/2, GetWidth()/6);
	m_hSave->SetPos(GetWidth()/4, GetHeight() - (int)(m_hSave->GetHeight()*1.5f));
	m_hSave->SetVisible(m_oGenerator.DoneGenerating());

	m_hGenerate->SetSize(GetWidth()/2, GetWidth()/6);
	m_hGenerate->SetPos(GetWidth()/4, GetHeight() - (int)(m_hSave->GetHeight()*1.5f) - (int)(m_hGenerate->GetHeight()*1.5f));

	CMovablePanel::Layout();
}

bool CAOPanel::KeyPressed(int iKey)
{
	if (iKey == 27 && m_oGenerator.IsGenerating())
	{
		m_hSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return true;
	}
	else
		return CMovablePanel::KeyPressed(iKey);
}

void CAOPanel::GenerateCallback(const tstring& sArgs)
{
	if (m_oGenerator.IsGenerating())
	{
		m_hSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return;
	}

	m_hSave->SetVisible(false);

	// Switch over to UV mode so we can see our progress.
	CSMAKWindow::Get()->SetRenderMode(true);

	// If the 3d model was there get rid of it.
	CSMAKWindow::Get()->Render();
	Application()->SwapBuffers();

	CSMAKWindow::Get()->Render();
	Application()->SwapBuffers();

	if (m_bColor)
		CSMAKWindow::Get()->SetDisplayColorAO(true);
	else
		CSMAKWindow::Get()->SetDisplayAO(true);

	m_hGenerate->SetText("Cancel");

	int iSize = m_hSizeSelector->GetSelectionValue();
	m_oGenerator.SetMethod(m_bColor?AOMETHOD_RENDER:(aomethod_t)m_hAOMethodSelector->GetSelectionValue());
	m_oGenerator.SetSize(iSize, iSize);
	m_oGenerator.SetBleed(m_hEdgeBleedSelector->GetSelectionValue());
	m_oGenerator.SetUseTexture(true);
	m_oGenerator.SetWorkListener(this);
	if (!m_bColor)
	{
		if (m_hAOMethodSelector->GetSelectionValue() == AOMETHOD_SHADOWMAP)
			m_oGenerator.SetSamples(m_hLightsSelector->GetSelectionValue());
		else
			m_oGenerator.SetSamples(m_hRayDensitySelector->GetSelectionValue());
		m_oGenerator.SetRandomize(m_hRandomCheckBox->GetToggleState());
		m_oGenerator.SetCreaseEdges(m_hCreaseCheckBox->GetToggleState());
		m_oGenerator.SetGroundOcclusion(m_hGroundOcclusionCheckBox->GetToggleState());
		m_oGenerator.SetRayFalloff(m_hFalloffSelector->GetSelectionValue());
	}
	m_oGenerator.Generate();

	CTextureHandle hAO;
	if (m_oGenerator.DoneGenerating())
		hAO = m_oGenerator.GenerateTexture();

	for (size_t i = 0; i < SMAKWindow()->GetMaterials().size(); i++)
	{
		CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[i];
		if (!hMaterial.IsValid())
			continue;

		if (!m_pScene->GetMaterial(i)->IsVisible())
			continue;

		hMaterial->SetParameter(m_bColor?"ColorAmbientOcclusion":"AmbientOcclusion", hAO);
	}

	m_hSave->SetVisible(m_oGenerator.DoneGenerating());

	m_hGenerate->SetText("Generate");
}

void CAOPanel::SaveMapDialogCallback(const tstring& sArgs)
{
	if (!m_oGenerator.DoneGenerating())
		return;

	CFileDialog::ShowSaveDialog("", ".png;.bmp;.tga", this, SaveMapFile);
}

void CAOPanel::SaveMapFileCallback(const tstring& sArgs)
{
	m_oGenerator.SaveToFile(sArgs.c_str());
}

void CAOPanel::BeginProgress()
{
	CProgressBar::Get()->SetVisible(true);
}

void CAOPanel::SetAction(const tstring& sAction, size_t iTotalProgress)
{
	CProgressBar::Get()->SetTotalProgress(iTotalProgress);
	CProgressBar::Get()->SetAction(sAction);
	WorkProgress(0, true);
}

void CAOPanel::WorkProgress(size_t iProgress, bool bForceDraw)
{
	static double flLastTime = 0;
	static double flLastGenerate = 0;

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && CSMAKWindow::Get()->GetTime() - flLastTime < 0.01f)
		return;

	CProgressBar::Get()->SetProgress(iProgress);

	if (m_oGenerator.IsGenerating() && flLastTime - flLastGenerate > 0.5f)
	{
		CTextureHandle hAO = m_oGenerator.GenerateTexture(true);

		for (size_t i = 0; i < SMAKWindow()->GetMaterials().size(); i++)
		{
			CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[i];
			if (!hMaterial.IsValid())
				continue;

			if (!m_pScene->GetMaterial(i)->IsVisible())
				continue;

			hMaterial->SetParameter(m_bColor?"ColorAmbientOcclusion":"AmbientOcclusion", hAO);
		}

		flLastGenerate = CSMAKWindow::Get()->GetTime();
	}

	CSMAKWindow::Get()->Render();
	Application()->SwapBuffers();

	flLastTime = CSMAKWindow::Get()->GetTime();
}

void CAOPanel::EndProgress()
{
	CProgressBar::Get()->SetVisible(false);
}

void CAOPanel::FindBestRayFalloff()
{
	if (!m_pScene->GetNumMeshes())
		return;

	if (!m_bColor)
		m_hFalloffSelector->SetSelection(m_hFalloffSelector->FindClosestSelectionValue(m_pScene->m_oExtends.Size().Length()/2));
}

void CAOPanel::Open(CConversionScene* pScene)
{
	CControl<CAOPanel> hPanel = Get();

	// Get rid of the last one, in case we've changed the scene.
	if (hPanel.Get())
		hPanel->Close();

#if 0
	if (bColor)
		s_pColorAOPanel = new CAOPanel(true, pScene);
	else
#endif
		s_hAOPanel = new CAOPanel(false, pScene);

	hPanel = Get();

	if (!hPanel)
		return;

	hPanel->SetVisible(true);
	hPanel->Layout();

	hPanel->FindBestRayFalloff();
}

CControl<CAOPanel> CAOPanel::Get()
{
	CControl<CAOPanel> hControl(s_hAOPanel);
	if (hControl)
		return hControl;

	return CControl<CAOPanel>();
}

void CAOPanel::SetVisible(bool bVisible)
{
	m_oGenerator.StopGenerating();

	CMovablePanel::SetVisible(bVisible);
}

void CAOPanel::AOMethodCallback(const tstring& sArgs)
{
	// So we can appear/disappear the ray density bar if the AO method has changed.
	Layout();
}
