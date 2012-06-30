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

#include "combogenerator.h"

#include <glgui/rootpanel.h>
#include <glgui/tree.h>
#include <glgui/checkbox.h>
#include <glgui/filedialog.h>
#include <textures/materiallibrary.h>

#include "smak_renderer.h"
#include "progressbar.h"
#include "picker.h"

using namespace glgui;

COptionsButton::COptionsButton()
	: CButton(0, 0, 100, 100, "", true)
{
	m_pPanel = (new COptionsPanel(this))->shared_from_this();
	m_pPanel->SetVisible(false);

	SetClickedListener(this, Open);
	SetUnclickedListener(this, Close);
}

void COptionsButton::OpenCallback(const tstring& sArgs)
{
	RootPanel()->AddControl(m_pPanel, true);

	float flPanelWidth = m_pPanel->GetWidth();
	float flButtonWidth = GetWidth();
	float x, y, w, h;
	GetAbsDimensions(x, y, w, h);
	m_pPanel->SetPos(x + flButtonWidth/2 - flPanelWidth/2, y+h+3);

	m_pPanel->Layout();
	m_pPanel->SetVisible(true);
}

void COptionsButton::CloseCallback(const tstring& sArgs)
{
	RootPanel()->RemoveControl(m_pPanel);
	m_pPanel->SetVisible(false);

	SetState(false, false);
}

COptionsButton::COptionsPanel::COptionsPanel(COptionsButton* pButton)
	: CPanel(0, 0, 200, 350)
{
	m_pButton = pButton;

	m_hOkay = AddControl(new CButton(0, 0, 100, 100, "Okay"));
	m_hOkay->SetClickedListener(m_pButton, Close);
}

void COptionsButton::COptionsPanel::Layout()
{
	BaseClass::Layout();

	m_hOkay->SetSize(40, 20);
	m_hOkay->SetPos(GetWidth()/2 - 20, GetHeight() - 30);
}

void COptionsButton::COptionsPanel::Paint(float x, float y, float w, float h)
{
	CRootPanel::PaintRect(x, y, w, h, g_clrBox);

	BaseClass::Paint(x, y, w, h);
}

CControl<CComboGeneratorPanel> CComboGeneratorPanel::s_hComboGeneratorPanel;

CComboGeneratorPanel::CComboGeneratorPanel(CConversionScene* pScene)
	: CMovablePanel("Combo map generator"), m_oGenerator(pScene)
{
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
	m_hSizeSelector->AddSelection(CScrollSelection<int>(4096, "4096x4096"));
	m_hSizeSelector->SetSelection(4);

	m_hLoResLabel = AddControl(new CLabel(0, 0, 32, 32, "Low Resolution Meshes"));

	m_hLoRes = AddControl(new CTree(SMAKWindow()->GetSMAKRenderer()->GetArrowTexture(), SMAKWindow()->GetSMAKRenderer()->GetEditTexture(), SMAKWindow()->GetSMAKRenderer()->GetVisibilityTexture()));
	m_hLoRes->SetBackgroundColor(g_clrBox);
	m_hLoRes->SetDroppedListener(this, DroppedLoResMesh);

	m_hHiResLabel = AddControl(new CLabel(0, 0, 32, 32, "High Resolution Meshes"));

	m_hHiRes = AddControl(new CTree(SMAKWindow()->GetSMAKRenderer()->GetArrowTexture(), SMAKWindow()->GetSMAKRenderer()->GetEditTexture(), SMAKWindow()->GetSMAKRenderer()->GetVisibilityTexture()));
	m_hHiRes->SetBackgroundColor(g_clrBox);
	m_hHiRes->SetDroppedListener(this, DroppedHiResMesh);

	m_hAddLoRes = AddControl(new CButton(0, 0, 100, 100, "Add"));
	m_hAddLoRes->SetClickedListener(this, AddLoRes);

	m_hAddHiRes = AddControl(new CButton(0, 0, 100, 100, "Add"));
	m_hAddHiRes->SetClickedListener(this, AddHiRes);

	m_hRemoveLoRes = AddControl(new CButton(0, 0, 100, 100, "Remove"));
	m_hRemoveLoRes->SetClickedListener(this, RemoveLoRes);

	m_hRemoveHiRes = AddControl(new CButton(0, 0, 100, 100, "Remove"));
	m_hRemoveHiRes->SetClickedListener(this, RemoveHiRes);

	m_hDiffuseCheckBox = AddControl(new CCheckBox());
	m_hDiffuseCheckBox->SetState(true, false);

	m_hDiffuseLabel = AddControl(new CLabel(0, 0, 100, 100, "Diffuse"));

	m_hAOCheckBox = AddControl(new CCheckBox());
	m_hAOCheckBox->SetState(true, false);

	m_hAOLabel = AddControl(new CLabel(0, 0, 100, 100, "Ambient Occlusion"));

	m_hAOOptions = AddControl(new COptionsButton());

	m_hBleedLabel = m_hAOOptions->GetOptionsPanel()->AddControl(new CLabel(0, 0, 100, 100, "Edge Bleed"));

	m_hBleedSelector = m_hAOOptions->GetOptionsPanel()->AddControl(new CScrollSelector<int>());
	m_hBleedSelector->AddSelection(CScrollSelection<int>(0, "0"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(1, "1"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(2, "2"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(3, "3"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(4, "4"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(5, "5"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(6, "6"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(7, "7"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(8, "8"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(9, "9"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(10, "10"));
	m_hBleedSelector->SetSelection(1);

	m_hSamplesLabel = m_hAOOptions->GetOptionsPanel()->AddControl(new CLabel(0, 0, 100, 100, "Samples"));

	m_hSamplesSelector = m_hAOOptions->GetOptionsPanel()->AddControl(new CScrollSelector<int>());
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(5, "5"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(6, "6"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(7, "7"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(8, "8"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(9, "9"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(10, "10"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(11, "11"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(12, "12"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(13, "13"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(14, "14"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(15, "15"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(16, "16"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(17, "17"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(18, "18"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(19, "19"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(20, "20"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(21, "21"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(22, "22"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(23, "23"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(24, "24"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(25, "25"));
	m_hSamplesSelector->SetSelection(15);

	m_hFalloffLabel = m_hAOOptions->GetOptionsPanel()->AddControl(new CLabel(0, 0, 100, 100, "Ray Falloff"));

	m_hFalloffSelector = m_hAOOptions->GetOptionsPanel()->AddControl(new CScrollSelector<float>());
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

	m_hRandomCheckBox = m_hAOOptions->GetOptionsPanel()->AddControl(new CCheckBox());

	m_hRandomLabel = m_hAOOptions->GetOptionsPanel()->AddControl(new CLabel(0, 0, 100, 100, "Randomize rays"));

	m_hGroundOcclusionCheckBox = m_hAOOptions->GetOptionsPanel()->AddControl(new CCheckBox());

	m_hGroundOcclusionLabel = m_hAOOptions->GetOptionsPanel()->AddControl(new CLabel(0, 0, 100, 100, "Ground occlusion"));

	m_hNormalCheckBox = AddControl(new CCheckBox());
	m_hNormalCheckBox->SetState(true, false);

	m_hNormalLabel = AddControl(new CLabel(0, 0, 100, 100, "Normal Map"));

	m_hGenerate = AddControl(new CButton(0, 0, 100, 100, "Generate"));
	m_hGenerate->SetClickedListener(this, Generate);

	m_hSave = AddControl(new CButton(0, 0, 100, 100, "Save Map"));

	m_hSave->SetClickedListener(this, SaveMapDialog);
	m_hSave->SetVisible(false);

	Layout();
}

void CComboGeneratorPanel::Layout()
{
	SetSize(400, 450);
	SetPos(GetParent()->GetWidth() - GetWidth() - 50, GetParent()->GetHeight() - GetHeight() - 100);

	float flSpace = 20;

	m_hSizeLabel->EnsureTextFits();

	float flSelectorSize = m_hSizeLabel->GetHeight() - 4;

	m_hSizeSelector->SetSize(GetWidth() - m_hSizeLabel->GetWidth() - flSpace, flSelectorSize);

	float flControlY = HEADER_HEIGHT;

	m_hSizeSelector->SetPos(GetWidth() - m_hSizeSelector->GetWidth() - flSpace/2, flControlY);
	m_hSizeLabel->SetPos(5, flControlY);

	float flTreeWidth = GetWidth()/2-15;

	m_hLoResLabel->EnsureTextFits();
	m_hLoResLabel->SetPos(10, 40);

	m_hLoRes->SetSize(flTreeWidth, 150);
	m_hLoRes->SetPos(10, 70);

	m_hAddLoRes->SetSize(40, 20);
	m_hAddLoRes->SetPos(10, 225);

	m_hRemoveLoRes->SetSize(60, 20);
	m_hRemoveLoRes->SetPos(60, 225);

	m_hHiResLabel->EnsureTextFits();
	m_hHiResLabel->SetPos(flTreeWidth+20, 40);

	m_hHiRes->SetSize(flTreeWidth, 150);
	m_hHiRes->SetPos(flTreeWidth+20, 70);

	m_hAddHiRes->SetSize(40, 20);
	m_hAddHiRes->SetPos(flTreeWidth+20, 225);

	m_hRemoveHiRes->SetSize(60, 20);
	m_hRemoveHiRes->SetPos(flTreeWidth+70, 225);

	m_hDiffuseLabel->SetPos(35, 220);
	m_hDiffuseLabel->EnsureTextFits();
	m_hDiffuseLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hDiffuseLabel->SetWrap(false);
	m_hDiffuseCheckBox->SetPos(20, 220 + m_hDiffuseLabel->GetHeight()/2 - m_hDiffuseCheckBox->GetHeight()/2);

	m_hAOLabel->SetPos(35, 250);
	m_hAOLabel->EnsureTextFits();
	m_hAOLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hAOLabel->SetWrap(false);
	m_hAOCheckBox->SetPos(20, 250 + m_hAOLabel->GetHeight()/2 - m_hAOCheckBox->GetHeight()/2);

	m_hAOOptions->SetPos(250, 250 + m_hAOLabel->GetHeight()/2 - m_hAOOptions->GetHeight()/2);
	m_hAOOptions->SetSize(60, 20);
	m_hAOOptions->SetText("Options...");

	float flControl = 3;

	m_hAOOptions->GetOptionsPanel()->SetSize(200, 200);

	flControlY = 10;

	m_hBleedLabel->SetSize(10, 10);
	m_hBleedLabel->EnsureTextFits();
	m_hBleedLabel->SetPos(5, flControlY);

	m_hBleedSelector->SetSize(m_hAOOptions->GetOptionsPanel()->GetWidth() - m_hBleedLabel->GetWidth() - flSpace, flSelectorSize);
	m_hBleedSelector->SetPos(m_hAOOptions->GetOptionsPanel()->GetWidth() - m_hBleedSelector->GetWidth() - flSpace/2, flControlY);

	flControlY += 30;

	m_hSamplesLabel->SetSize(10, 10);
	m_hSamplesLabel->EnsureTextFits();
	m_hSamplesLabel->SetPos(5, flControlY);

	m_hSamplesSelector->SetSize(m_hAOOptions->GetOptionsPanel()->GetWidth() - m_hSamplesLabel->GetWidth() - flSpace, flSelectorSize);
	m_hSamplesSelector->SetPos(m_hAOOptions->GetOptionsPanel()->GetWidth() - m_hSamplesSelector->GetWidth() - flSpace/2, flControlY);

	flControlY += 30;

	m_hFalloffLabel->SetSize(10, 10);
	m_hFalloffLabel->EnsureTextFits();
	m_hFalloffLabel->SetPos(5, flControlY);

	m_hFalloffSelector->SetSize(m_hAOOptions->GetOptionsPanel()->GetWidth() - m_hFalloffLabel->GetWidth() - flSpace, flSelectorSize);
	m_hFalloffSelector->SetPos(m_hAOOptions->GetOptionsPanel()->GetWidth() - m_hFalloffSelector->GetWidth() - flSpace/2, flControlY);

	flControlY += 30;

	m_hRandomLabel->SetSize(10, 10);
	m_hRandomLabel->EnsureTextFits();
	m_hRandomLabel->SetPos(25, flControlY);

	m_hRandomCheckBox->SetPos(10, flControlY + m_hRandomLabel->GetHeight()/2 - m_hRandomCheckBox->GetHeight()/2);

	flControlY += 30;

	m_hGroundOcclusionLabel->SetSize(10, 10);
	m_hGroundOcclusionLabel->EnsureTextFits();
	m_hGroundOcclusionLabel->SetPos(25, flControlY);

	m_hGroundOcclusionCheckBox->SetPos(10, flControlY + m_hGroundOcclusionLabel->GetHeight()/2 - m_hGroundOcclusionCheckBox->GetHeight()/2);

	m_hNormalLabel->SetPos(35, 280);
	m_hNormalLabel->EnsureTextFits();
	m_hNormalLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hNormalLabel->SetWrap(false);
	m_hNormalCheckBox->SetPos(20, 280 + m_hNormalLabel->GetHeight()/2 - m_hNormalCheckBox->GetHeight()/2);

	m_hGenerate->SetSize(100, 33);
	m_hGenerate->SetPos(GetWidth()/2 - m_hGenerate->GetWidth()/2, GetHeight() - (int)(m_hGenerate->GetHeight()*3));

	m_hSave->SetSize(100, 33);
	m_hSave->SetPos(GetWidth()/2 - m_hSave->GetWidth()/2, GetHeight() - (int)(m_hSave->GetHeight()*1.5f));
	m_hSave->SetVisible(m_oGenerator.DoneGenerating());

	size_t i;
	m_hLoRes->ClearTree();
	if (!m_apLoResMeshes.size())
		m_hLoRes->AddNode("No meshes. Click 'Add'");
	else
	{
		for (i = 0; i < m_apLoResMeshes.size(); i++)
		{
			m_hLoRes->AddNode<CConversionMeshInstance>(m_apLoResMeshes[i]->GetMesh()->GetName(), m_apLoResMeshes[i]);
			m_hLoRes->GetNode(i)->SetIcon(SMAKWindow()->GetSMAKRenderer()->GetMeshesNodeTexture());
		}
	}

	m_hHiRes->ClearTree();
	if (!m_apHiResMeshes.size())
		m_hHiRes->AddNode("No meshes. Click 'Add'");
	else
	{
		for (i = 0; i < m_apHiResMeshes.size(); i++)
		{
			m_hHiRes->AddNode<CConversionMeshInstance>(m_apHiResMeshes[i]->GetMesh()->GetName(), m_apHiResMeshes[i]);
			m_hHiRes->GetNode(i)->SetIcon(SMAKWindow()->GetSMAKRenderer()->GetMeshesNodeTexture());
		}
	}

	CMovablePanel::Layout();
}

void CComboGeneratorPanel::UpdateScene()
{
	m_apLoResMeshes.clear();
	m_apHiResMeshes.clear();

	if (m_pScene->GetNumMeshes())
		m_hFalloffSelector->SetSelection(m_hFalloffSelector->FindClosestSelectionValue(m_pScene->m_oExtends.Size().Length()/2));
}

void CComboGeneratorPanel::Think()
{
	bool bFoundMaterial = false;
	for (size_t iMesh = 0; iMesh < m_apLoResMeshes.size(); iMesh++)
	{
		CConversionMeshInstance* pMeshInstance = m_apLoResMeshes[iMesh];

		for (size_t iMaterialStub = 0; iMaterialStub < pMeshInstance->GetMesh()->GetNumMaterialStubs(); iMaterialStub++)
		{
			size_t iMaterial = pMeshInstance->GetMappedMaterial(iMaterialStub)->m_iMaterial;

			if (iMaterial >= SMAKWindow()->GetMaterials().size())
				continue;

			bFoundMaterial = true;
			break;
		}
	}

	CMovablePanel::Think();
}

void CComboGeneratorPanel::Paint(float x, float y, float w, float h)
{
	CMovablePanel::Paint(x, y, w, h);

	if (!m_bMinimized)
	{
		CRootPanel::PaintRect(x+10, y+250, w-20, 1, g_clrBoxHi);
		CRootPanel::PaintRect(x+10, y+h-110, w-20, 1, g_clrBoxHi);
	}
}

bool CComboGeneratorPanel::KeyPressed(int iKey)
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

void CComboGeneratorPanel::GenerateCallback(const tstring& sArgs)
{
	if (m_oGenerator.IsGenerating())
	{
		m_hSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return;
	}

	m_hSave->SetVisible(false);

	m_hGenerate->SetText("Cancel");

	CSMAKWindow::Get()->SetDisplayNormal(true);

	// Disappear all of the hi-res meshes so we can see the lo res better.
	for (size_t m = 0; m < m_apHiResMeshes.size(); m++)
		m_apHiResMeshes[m]->SetVisible(false);

	for (size_t m = 0; m < m_apLoResMeshes.size(); m++)
		m_apLoResMeshes[m]->SetVisible(true);

	int iSize = m_hSizeSelector->GetSelectionValue();
	m_oGenerator.SetSize(iSize, iSize);
	m_oGenerator.ClearMethods();

	if (m_hDiffuseCheckBox->GetState())
	{
		m_oGenerator.AddDiffuse();
		CSMAKWindow::Get()->SetDisplayTexture(true);
	}

	if (m_hAOCheckBox->GetState())
	{
		m_oGenerator.AddAO(
			m_hSamplesSelector->GetSelectionValue(),
			m_hRandomCheckBox->GetState(),
			m_hFalloffSelector->GetSelectionValue(),
			m_hGroundOcclusionCheckBox->GetState(),
			m_hBleedSelector->GetSelectionValue());
		CSMAKWindow::Get()->SetDisplayAO(true);
	}

	if (m_hNormalCheckBox->GetState())
	{
		m_oGenerator.AddNormal();
		CSMAKWindow::Get()->SetDisplayNormal(true);
	}

	m_oGenerator.SetModels(m_apHiResMeshes, m_apLoResMeshes);
	m_oGenerator.SetWorkListener(this);
	m_oGenerator.Generate();

	CTextureHandle hDiffuse;
	CTextureHandle hAO;
	CTextureHandle hNormal;
	if (m_oGenerator.DoneGenerating())
	{
		hDiffuse = m_oGenerator.GenerateDiffuse();
		hAO = m_oGenerator.GenerateAO();
		hNormal = m_oGenerator.GenerateNormal();
	}

	for (size_t i = 0; i < SMAKWindow()->GetMaterials().size(); i++)
	{
		CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[i];

		if (!m_pScene->GetMaterial(i)->IsVisible())
			continue;

		hMaterial->SetParameter("DiffuseTexture", hDiffuse);
		hMaterial->SetParameter("AmbientOcclusion", hAO);
		hMaterial->SetParameter("Normal", hNormal);
	}

	m_hSave->SetVisible(m_oGenerator.DoneGenerating());

	m_hGenerate->SetText("Generate");
}

void CComboGeneratorPanel::SaveMapDialogCallback(const tstring& sArgs)
{
	if (!m_oGenerator.DoneGenerating())
		return;

	CFileDialog::ShowSaveDialog("", ".png;.bmp;.tga", this, SaveMapFile);
}

void CComboGeneratorPanel::SaveMapFileCallback(const tstring& sArgs)
{
	tstring sFilename = sArgs;

	if (!sFilename.length())
		return;

	m_oGenerator.SaveAll(sFilename);

	for (size_t i = 0; i < SMAKWindow()->GetMaterials().size(); i++)
	{
		if (!m_pScene->GetMaterial(i)->IsVisible())
			continue;

		m_pScene->GetMaterial(i)->m_sNormalTexture = sFilename;
	}

	CRootPanel::Get()->Layout();
}

void CComboGeneratorPanel::BeginProgress()
{
	CProgressBar::Get()->SetVisible(true);
}

void CComboGeneratorPanel::SetAction(const tstring& sAction, size_t iTotalProgress)
{
	CProgressBar::Get()->SetTotalProgress(iTotalProgress);
	CProgressBar::Get()->SetAction(sAction);
	WorkProgress(0, true);
}

void CComboGeneratorPanel::WorkProgress(size_t iProgress, bool bForceDraw)
{
	static double flLastTime = 0;
	static double flLastGenerate = 0;

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && CSMAKWindow::Get()->GetTime() - flLastTime < 0.01f)
		return;

	CProgressBar::Get()->SetProgress(iProgress);

	if (m_oGenerator.IsGenerating() && CSMAKWindow::Get()->GetTime() - flLastGenerate > 0.5f)
	{
		CTextureHandle hDiffuse = m_oGenerator.GenerateDiffuse(true);
		CTextureHandle hAO = m_oGenerator.GenerateAO(true);
		CTextureHandle hNormal = m_oGenerator.GenerateNormal(true);

		for (size_t i = 0; i < SMAKWindow()->GetMaterials().size(); i++)
		{
			CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[i];
			if (!hMaterial.IsValid())
				continue;

			if (!m_pScene->GetMaterial(i)->IsVisible())
				continue;

			hMaterial->SetParameter("DiffuseTexture", hDiffuse);
			hMaterial->SetParameter("AmbientOcclusion", hAO);
			hMaterial->SetParameter("Normal", hNormal);
		}

		flLastGenerate = CSMAKWindow::Get()->GetTime();
	}

	CSMAKWindow::Get()->Render();
	SMAKWindow()->SwapBuffers();

	flLastTime = CSMAKWindow::Get()->GetTime();
}

void CComboGeneratorPanel::EndProgress()
{
	CProgressBar::Get()->SetVisible(false);
}

void CComboGeneratorPanel::AddLoResCallback(const tstring& sArgs)
{
	if (m_hMeshInstancePicker)
		m_hMeshInstancePicker->Close();

	m_hMeshInstancePicker = (new CMeshInstancePicker(this, AddLoResMesh))->GetHandle();

	float x, y, w, h, pw, ph;
	GetAbsDimensions(x, y, w, h);
	m_hMeshInstancePicker->GetSize(pw, ph);
	m_hMeshInstancePicker->SetPos(x + w/2 - pw/2, y + h/2 - ph/2);
}

void CComboGeneratorPanel::AddHiResCallback(const tstring& sArgs)
{
	if (m_hMeshInstancePicker)
		m_hMeshInstancePicker->Close();

	m_hMeshInstancePicker = (new CMeshInstancePicker(this, AddHiResMesh))->GetHandle();

	float x, y, w, h, pw, ph;
	GetAbsDimensions(x, y, w, h);
	m_hMeshInstancePicker->GetSize(pw, ph);
	m_hMeshInstancePicker->SetPos(x + w/2 - pw/2, y + h/2 - ph/2);
}

void CComboGeneratorPanel::AddLoResMeshCallback(const tstring& sArgs)
{
	CConversionMeshInstance* pMeshInstance = m_hMeshInstancePicker->GetPickedMeshInstance();
	if (!pMeshInstance)
		return;

	size_t i;
	for (i = 0; i < m_apLoResMeshes.size(); i++)
		if (m_apLoResMeshes[i] == pMeshInstance)
			m_apLoResMeshes.erase(m_apLoResMeshes.begin()+i);

	for (i = 0; i < m_apHiResMeshes.size(); i++)
		if (m_apHiResMeshes[i] == pMeshInstance)
			m_apHiResMeshes.erase(m_apHiResMeshes.begin()+i);

	m_apLoResMeshes.push_back(pMeshInstance);

	m_hMeshInstancePicker->Close();

	Layout();
}

void CComboGeneratorPanel::AddHiResMeshCallback(const tstring& sArgs)
{
	CConversionMeshInstance* pMeshInstance = m_hMeshInstancePicker->GetPickedMeshInstance();
	if (!pMeshInstance)
		return;

	size_t i;
	for (i = 0; i < m_apLoResMeshes.size(); i++)
		if (m_apLoResMeshes[i] == pMeshInstance)
			m_apLoResMeshes.erase(m_apLoResMeshes.begin()+i);

	for (i = 0; i < m_apHiResMeshes.size(); i++)
		if (m_apHiResMeshes[i] == pMeshInstance)
			m_apHiResMeshes.erase(m_apHiResMeshes.begin()+i);

	m_apHiResMeshes.push_back(pMeshInstance);

	m_hMeshInstancePicker->Close();

	Layout();
}

void CComboGeneratorPanel::RemoveLoResCallback(const tstring& sArgs)
{
	CTreeNode* pNode = m_hLoRes->GetSelectedNode();
	if (!pNode)
		return;

	CTreeNodeObject<CConversionMeshInstance>* pMeshNode = dynamic_cast<CTreeNodeObject<CConversionMeshInstance>*>(pNode);
	if (!pMeshNode)
		return;

	for (size_t i = 0; i < m_apLoResMeshes.size(); i++)
		if (m_apLoResMeshes[i] == pMeshNode->GetObject())
			m_apLoResMeshes.erase(m_apLoResMeshes.begin()+i);

	Layout();
}

void CComboGeneratorPanel::RemoveHiResCallback(const tstring& sArgs)
{
	CTreeNode* pNode = m_hHiRes->GetSelectedNode();
	if (!pNode)
		return;

	CTreeNodeObject<CConversionMeshInstance>* pMeshNode = dynamic_cast<CTreeNodeObject<CConversionMeshInstance>*>(pNode);
	if (!pMeshNode)
		return;

	for (size_t i = 0; i < m_apHiResMeshes.size(); i++)
		if (m_apHiResMeshes[i] == pMeshNode->GetObject())
			m_apHiResMeshes.erase(m_apHiResMeshes.begin()+i);

	Layout();
}

void CComboGeneratorPanel::DroppedLoResMeshCallback(const tstring& sArgs)
{
	IDraggable* pDraggable = CRootPanel::Get()->GetCurrentDraggable();
	CConversionMeshInstance* pMeshInstance = dynamic_cast<CTreeNodeObject<CConversionMeshInstance>*>(pDraggable)->GetObject();

	if (!pMeshInstance)
		return;

	size_t i;
	for (i = 0; i < m_apLoResMeshes.size(); i++)
		if (m_apLoResMeshes[i] == pMeshInstance)
			m_apLoResMeshes.erase(m_apLoResMeshes.begin()+i);

	for (i = 0; i < m_apHiResMeshes.size(); i++)
		if (m_apHiResMeshes[i] == pMeshInstance)
			m_apHiResMeshes.erase(m_apHiResMeshes.begin()+i);

	m_apLoResMeshes.push_back(pMeshInstance);

	Layout();
}

void CComboGeneratorPanel::DroppedHiResMeshCallback(const tstring& sArgs)
{
	IDraggable* pDraggable = CRootPanel::Get()->GetCurrentDraggable();
	CConversionMeshInstance* pMeshInstance = dynamic_cast<CTreeNodeObject<CConversionMeshInstance>*>(pDraggable)->GetObject();

	if (!pMeshInstance)
		return;

	size_t i;
	for (i = 0; i < m_apLoResMeshes.size(); i++)
		if (m_apLoResMeshes[i] == pMeshInstance)
			m_apLoResMeshes.erase(m_apLoResMeshes.begin()+i);

	for (i = 0; i < m_apHiResMeshes.size(); i++)
		if (m_apHiResMeshes[i] == pMeshInstance)
			m_apHiResMeshes.erase(m_apHiResMeshes.begin()+i);

	m_apHiResMeshes.push_back(pMeshInstance);

	m_hMeshInstancePicker->Close();

	Layout();
}

void CComboGeneratorPanel::Open(CConversionScene* pScene)
{
	CComboGeneratorPanel* pPanel = s_hComboGeneratorPanel;

	if (pPanel)
		pPanel->Close();

	pPanel = s_hComboGeneratorPanel = new CComboGeneratorPanel(pScene);

	if (!pPanel)
		return;

	pPanel->SetVisible(true);
	pPanel->Layout();

	if (pScene->GetNumMeshes())
		pPanel->m_hFalloffSelector->SetSelection(pPanel->m_hFalloffSelector->FindClosestSelectionValue(pScene->m_oExtends.Size().Length()/2));
}

void CComboGeneratorPanel::SetVisible(bool bVisible)
{
	m_oGenerator.StopGenerating();

	CMovablePanel::SetVisible(bVisible);
}

