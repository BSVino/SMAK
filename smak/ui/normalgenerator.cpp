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

#include "normalgenerator.h"

#include <glgui/rootpanel.h>
#include <glgui/tree.h>
#include <glgui/filedialog.h>
#include <textures/materiallibrary.h>

#include "smak_renderer.h"

using namespace glgui;

CControl<CNormalPanel> CNormalPanel::s_hNormalPanel;

CNormalPanel::CNormalPanel(CConversionScene* pScene)
	: CMovablePanel("Normal map generator"), m_oGenerator(pScene)
{
	m_pScene = pScene;

	m_hMaterialsLabel = AddControl(new CLabel(0, 0, 32, 32, "Choose A Material To Generate From:"));

	m_hMaterials = AddControl(new CTree(SMAKWindow()->GetSMAKRenderer()->GetArrowTexture(), SMAKWindow()->GetSMAKRenderer()->GetEditTexture(), SMAKWindow()->GetSMAKRenderer()->GetVisibilityTexture()));
	m_hMaterials->SetBackgroundColor(g_clrBox);

	m_hProgressLabel = AddControl(new CLabel(0, 0, 100, 100, ""));

	m_hDepthLabel = AddControl(new CLabel(0, 0, 32, 32, "Overall Depth"));

	m_hDepthSelector = AddControl(new CScrollSelector<float>());
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.0f, "0%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.025f, "2.5%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.05f, "5%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.075f, "7.5%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.1f, "10%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.2f, "20%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.3f, "30%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.4f, "40%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.5f, "50%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.6f, "60%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.7f, "70%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.8f, "80%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.9f, "90%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.0f, "100%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.1f, "110%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.2f, "120%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.3f, "130%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.4f, "140%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.5f, "150%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.6f, "160%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.7f, "170%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.8f, "180%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.9f, "190%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(2.0f, "200%"));
	m_hDepthSelector->SetSelection(13);
	m_hDepthSelector->SetSelectedListener(this, UpdateNormal2);

	m_hHiDepthLabel = AddControl(new CLabel(0, 0, 32, 32, "Texture Hi-Freq Depth"));

	m_hHiDepthSelector = AddControl(new CScrollSelector<float>());
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.0f, "0%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.025f, "2.5%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.05f, "5%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.075f, "7.5%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.1f, "10%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.2f, "20%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.3f, "30%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.4f, "40%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.5f, "50%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.6f, "60%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.7f, "70%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.8f, "80%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.9f, "90%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.0f, "100%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.1f, "110%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.2f, "120%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.3f, "130%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.4f, "140%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.5f, "150%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.6f, "160%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.7f, "170%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.8f, "180%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.9f, "190%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(2.0f, "200%"));
	m_hHiDepthSelector->SetSelection(13);
	m_hHiDepthSelector->SetSelectedListener(this, UpdateNormal2);

	m_hMidDepthLabel = AddControl(new CLabel(0, 0, 32, 32, "Texture Mid-Freq Depth"));

	m_hMidDepthSelector = AddControl(new CScrollSelector<float>());
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.0f, "0%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.025f, "2.5%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.05f, "5%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.075f, "7.5%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.1f, "10%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.2f, "20%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.3f, "30%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.4f, "40%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.5f, "50%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.6f, "60%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.7f, "70%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.8f, "80%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.9f, "90%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.0f, "100%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.1f, "110%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.2f, "120%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.3f, "130%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.4f, "140%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.5f, "150%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.6f, "160%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.7f, "170%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.8f, "180%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.9f, "190%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(2.0f, "200%"));
	m_hMidDepthSelector->SetSelection(13);
	m_hMidDepthSelector->SetSelectedListener(this, UpdateNormal2);

	m_hLoDepthLabel = AddControl(new CLabel(0, 0, 32, 32, "Texture Lo-Freq Depth"));

	m_hLoDepthSelector = AddControl(new CScrollSelector<float>());
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.0f, "0%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.025f, "2.5%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.05f, "5%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.075f, "7.5%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.1f, "10%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.2f, "20%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.3f, "30%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.4f, "40%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.5f, "50%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.6f, "60%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.7f, "70%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.8f, "80%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.9f, "90%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.0f, "100%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.1f, "110%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.2f, "120%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.3f, "130%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.4f, "140%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.5f, "150%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.6f, "160%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.7f, "170%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.8f, "180%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.9f, "190%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(2.0f, "200%"));
	m_hLoDepthSelector->SetSelection(13);
	m_hLoDepthSelector->SetSelectedListener(this, UpdateNormal2);

	m_hSave = AddControl(new CButton(0, 0, 100, 100, "Save Map"));

	m_hSave->SetClickedListener(this, SaveMapDialog);
	m_hSave->SetVisible(false);

	Layout();
}

void CNormalPanel::Layout()
{
	SetSize(400, 450);
	SetPos(GetParent()->GetWidth() - GetWidth() - 50, GetParent()->GetHeight() - GetHeight() - 100);

	float flSpace = 20;

	m_hMaterialsLabel->EnsureTextFits();

	float flSelectorSize = m_hMaterialsLabel->GetHeight() - 4;
	float flControlY = HEADER_HEIGHT;

	m_hMaterialsLabel->SetPos(5, flControlY);

	float flTreeWidth = GetWidth()/2-15;

	m_hMaterials->ClearTree();
	for (size_t i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		m_hMaterials->AddNode<CConversionMaterial>(m_pScene->GetMaterial(i)->GetName(), m_pScene->GetMaterial(i));

		if (SMAKWindow()->GetMaterials().size() > i && SMAKWindow()->GetMaterials()[i]->FindParameter("DiffuseTexture") == ~0)
			m_hMaterials->GetNode(i)->m_hLabel->SetAlpha(100);

		m_hMaterials->SetSelectedListener(this, SetupNormal2);
	}

	m_hMaterials->SetSize(flTreeWidth, 150);
	m_hMaterials->SetPos(GetWidth()/2-flTreeWidth/2, 50);

	m_hProgressLabel->SetSize(1, 1);
	m_hProgressLabel->SetPos(35, 220);
	m_hProgressLabel->EnsureTextFits();
	m_hProgressLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hProgressLabel->SetWrap(false);

	m_hDepthLabel->SetPos(10, 290);
	m_hDepthLabel->EnsureTextFits();
	m_hDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hDepthLabel->SetWrap(false);
	m_hDepthSelector->SetPos(m_hDepthLabel->GetRight() + 10, 290 + m_hDepthLabel->GetHeight()/2 - m_hDepthSelector->GetHeight()/2);
	m_hDepthSelector->SetRight(GetWidth() - 10);

	m_hHiDepthLabel->SetPos(10, 310);
	m_hHiDepthLabel->EnsureTextFits();
	m_hHiDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hHiDepthLabel->SetWrap(false);
	m_hHiDepthSelector->SetPos(m_hHiDepthLabel->GetRight() + 10, 310 + m_hHiDepthLabel->GetHeight()/2 - m_hHiDepthSelector->GetHeight()/2);
	m_hHiDepthSelector->SetRight(GetWidth() - 10);

	m_hMidDepthLabel->SetPos(10, 330);
	m_hMidDepthLabel->EnsureTextFits();
	m_hMidDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hMidDepthLabel->SetWrap(false);
	m_hMidDepthSelector->SetPos(m_hMidDepthLabel->GetRight() + 10, 330 + m_hMidDepthLabel->GetHeight()/2 - m_hMidDepthSelector->GetHeight()/2);
	m_hMidDepthSelector->SetRight(GetWidth() - 10);

	m_hLoDepthLabel->SetPos(10, 350);
	m_hLoDepthLabel->EnsureTextFits();
	m_hLoDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hLoDepthLabel->SetWrap(false);
	m_hLoDepthSelector->SetPos(m_hLoDepthLabel->GetRight() + 10, 350 + m_hLoDepthLabel->GetHeight()/2 - m_hLoDepthSelector->GetHeight()/2);
	m_hLoDepthSelector->SetRight(GetWidth() - 10);

	m_hSave->SetSize(100, 33);
	m_hSave->SetPos(GetWidth()/2 - m_hSave->GetWidth()/2, GetHeight() - (int)(m_hSave->GetHeight()*1.5f));
	m_hSave->SetVisible(m_oGenerator.DoneGenerating());

	CMovablePanel::Layout();
}

void CNormalPanel::UpdateScene()
{
	Layout();
}

void CNormalPanel::Think()
{
	if (m_oGenerator.IsNewNormal2Available())
	{
		CTextureHandle hNewNormal2;
		m_oGenerator.GetNormalMap2(hNewNormal2);

		for (size_t i = 0; i < SMAKWindow()->GetMaterials().size(); i++)
		{
			if (!m_pScene->GetMaterial(i)->IsVisible())
				continue;

			SMAKWindow()->GetMaterials()[i]->SetParameter("Normal2", hNewNormal2);
			break;
		}

		m_hSave->SetVisible(hNewNormal2.IsValid());

		if (hNewNormal2.IsValid())
			CSMAKWindow::Get()->SetDisplayNormal(true);
	}

	if (m_oGenerator.IsSettingUp())
	{
		tstring s;
		s = sprintf("Setting up... %d%%", (int)(m_oGenerator.GetSetupProgress()*100));
		m_hProgressLabel->SetText(s);
		m_hSave->SetVisible(false);
	}
	else if (m_oGenerator.IsGeneratingNewNormal2())
	{
		tstring s;
		s = sprintf("Generating... %d%%", (int)(m_oGenerator.GetNormal2GenerationProgress()*100));
		m_hProgressLabel->SetText(s);
		m_hSave->SetVisible(false);
	}
	else
		m_hProgressLabel->SetText("");

	m_oGenerator.Think();

	CMovablePanel::Think();
}

void CNormalPanel::Paint(float x, float y, float w, float h)
{
	CMovablePanel::Paint(x, y, w, h);

	if (!m_bMinimized)
	{
		CRootPanel::PaintRect(x+10, y+295, w-20, 1, g_clrBoxHi);
		CRootPanel::PaintRect(x+10, y+385, w-20, 1, g_clrBoxHi);
	}
}

bool CNormalPanel::KeyPressed(int iKey)
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

void CNormalPanel::SaveMapDialogCallback(const tstring& sArgs)
{
	if (!m_oGenerator.DoneGenerating())
		return;

	CFileDialog::ShowSaveDialog("", ".png;.bmp;.tga", this, SaveMapFile);
}

void CNormalPanel::SaveMapFileCallback(const tstring& sArgs)
{
	tstring sFilename = sArgs;

	if (!sFilename.length())
		return;

	SMAKWindow()->SaveNormal(m_oGenerator.GetGenerationMaterial(), sFilename);

	CRootPanel::Get()->Layout();
}

void CNormalPanel::SetupNormal2Callback(const tstring& sArgs)
{
	m_oGenerator.SetNormalTextureDepth(m_hDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureHiDepth(m_hHiDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureMidDepth(m_hMidDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureLoDepth(m_hLoDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTexture(m_hMaterials->GetSelectedNodeId());

	CSMAKWindow::Get()->SetDisplayNormal(true);
}

void CNormalPanel::UpdateNormal2Callback(const tstring& sArgs)
{
	m_oGenerator.SetNormalTextureDepth(m_hDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureHiDepth(m_hHiDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureMidDepth(m_hMidDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureLoDepth(m_hLoDepthSelector->GetSelectionValue());
	m_oGenerator.UpdateNormal2();
}

void CNormalPanel::Open(CConversionScene* pScene)
{
	CNormalPanel* pPanel = s_hNormalPanel;

	if (pPanel)
		pPanel->Close();

	pPanel = s_hNormalPanel = new CNormalPanel(pScene);

	if (!pPanel)
		return;

	pPanel->SetVisible(true);
	pPanel->Layout();
}

void CNormalPanel::SetVisible(bool bVisible)
{
	m_oGenerator.StopGenerating();

	CMovablePanel::SetVisible(bVisible);
}
