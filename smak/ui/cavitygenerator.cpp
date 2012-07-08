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

#include "cavitygenerator.h"

#include <glgui/label.h>
#include <glgui/selector.h>
#include <glgui/checkbox.h>
#include <glgui/filedialog.h>
#include <glgui/tree.h>
#include <textures/materiallibrary.h>

#include "progressbar.h"
#include "smakwindow.h"

using namespace glgui;

CControl<CCavityPanel> CCavityPanel::s_hCavityPanel;

CCavityPanel::CCavityPanel(CConversionScene* pScene)
	: CMovablePanel("Cavity map generator"), m_oGenerator(pScene)
{
	m_pScene = pScene;

	m_hMaterialsLabel = AddControl(new CLabel(0, 0, 32, 32, "Choose A Material To Generate From:"));

	m_hMaterials = AddControl(new CTree());
	m_hMaterials->SetBackgroundColor(g_clrBox);

	m_hGenerate = AddControl(new CButton(0, 0, 100, 100, "Generate"));

	m_hGenerate->SetClickedListener(this, Generate);

	m_hSave = AddControl(new CButton(0, 0, 100, 100, "Save Map"));

	m_hSave->SetClickedListener(this, SaveMapDialog);
	m_hSave->SetVisible(false);

	SetWidth(300);
	SetPos(GetParent()->GetWidth() - GetWidth() - 150, GetParent()->GetHeight() - GetHeight() - 150);

	Layout();
}

void CCavityPanel::Layout()
{
	float flSpace = 20;

	m_hMaterialsLabel->EnsureTextFits();

	float flSelectorSize = m_hMaterialsLabel->GetHeight() - 4;
	float flControlY = HEADER_HEIGHT;

	m_hMaterialsLabel->SetPos(5, flControlY);
	if (m_hMaterialsLabel->GetRight() > GetWidth()-5)
		m_hMaterialsLabel->SetRight(GetWidth()-5);

	float flTreeWidth = GetWidth()-30;

	m_hMaterials->ClearTree();
	for (size_t i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		m_hMaterials->AddNode<CConversionMaterial>(m_pScene->GetMaterial(i)->GetName(), m_pScene->GetMaterial(i));

		if (SMAKWindow()->GetMaterials().size() > i && SMAKWindow()->GetMaterials()[i]->FindParameter("DiffuseTexture") == ~0)
			m_hMaterials->GetNode(i)->m_hLabel->SetAlpha(100);
	}

	m_hMaterials->SetSize(flTreeWidth, 150);
	m_hMaterials->SetPos(GetWidth()/2-flTreeWidth/2, 50);

	m_hSave->SetSize(GetWidth()/2, GetWidth()/6);
	m_hSave->SetPos(GetWidth()/4, GetHeight() - (int)(m_hSave->GetHeight()*1.5f));
	m_hSave->SetVisible(m_oGenerator.DoneGenerating());

	m_hGenerate->SetSize(GetWidth()/2, GetWidth()/6);
	m_hGenerate->SetPos(GetWidth()/4, GetHeight() - (int)(m_hSave->GetHeight()*1.5f) - (int)(m_hGenerate->GetHeight()*1.5f));

	CMovablePanel::Layout();
}

bool CCavityPanel::KeyPressed(int iKey)
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

void CCavityPanel::GenerateCallback(const tstring& sArgs)
{
	if (m_oGenerator.IsGenerating())
	{
		m_hSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return;
	}

	CTreeNode* pNode = m_hMaterials->GetSelectedNode();
	if (!pNode)
		return;

	CTreeNodeObject<CConversionMaterial>* pNodeObject = static_cast<CTreeNodeObject<CConversionMaterial>*>(pNode);

	if (!pNodeObject->GetObject())
		return;

	CConversionMaterial* pMaterial = pNodeObject->GetObject();

	CMaterialHandle hMaterial;

	for (size_t i = 0; i < SMAKWindow()->GetScene()->GetNumMaterials(); i++)
	{
		if (SMAKWindow()->GetScene()->GetMaterial(i) == pMaterial)
		{
			hMaterial = SMAKWindow()->GetMaterials()[i];
			break;
		}
	}

	if (!hMaterial.IsValid())
		return;

	m_hSave->SetVisible(false);

	// Switch over to UV mode so we can see our progress.
	CSMAKWindow::Get()->SetRenderMode(true);

	// If the 3d model was there get rid of it.
	CSMAKWindow::Get()->Render();
	Application()->SwapBuffers();

	CSMAKWindow::Get()->Render();
	Application()->SwapBuffers();

	CSMAKWindow::Get()->SetDisplayCavity(true);

	m_hGenerate->SetText("Cancel");

	m_oGenerator.SetWorkListener(this);
	m_oGenerator.SetNormalTexture(hMaterial);
	m_oGenerator.Generate();

	CTextureHandle hCavity;
	if (m_oGenerator.DoneGenerating())
		hCavity = m_oGenerator.GenerateTexture();

	hMaterial->SetParameter("Cavity", hCavity);

	m_hSave->SetVisible(m_oGenerator.DoneGenerating());

	m_hGenerate->SetText("Generate");
}

void CCavityPanel::SaveMapDialogCallback(const tstring& sArgs)
{
	if (!m_oGenerator.DoneGenerating())
		return;

	CFileDialog::ShowSaveDialog("", ".png;.bmp;.tga", this, SaveMapFile);
}

void CCavityPanel::SaveMapFileCallback(const tstring& sArgs)
{
	m_oGenerator.SaveToFile(sArgs.c_str());
}

void CCavityPanel::BeginProgress()
{
	CProgressBar::Get()->SetVisible(true);
}

void CCavityPanel::SetAction(const tstring& sAction, size_t iTotalProgress)
{
	CProgressBar::Get()->SetTotalProgress(iTotalProgress);
	CProgressBar::Get()->SetAction(sAction);
	WorkProgress(0, true);
}

void CCavityPanel::WorkProgress(size_t iProgress, bool bForceDraw)
{
	static double flLastTime = 0;
	static double flLastGenerate = 0;

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && CSMAKWindow::Get()->GetTime() - flLastTime < 0.01f)
		return;

	CProgressBar::Get()->SetProgress(iProgress);

	if (m_oGenerator.IsGenerating() && flLastTime - flLastGenerate > 0.5f)
	{
		CTextureHandle hCavity = m_oGenerator.GenerateTexture(true);

		for (size_t i = 0; i < SMAKWindow()->GetMaterials().size(); i++)
		{
			CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[i];
			if (!hMaterial.IsValid())
				continue;

			if (!m_pScene->GetMaterial(i)->IsVisible())
				continue;

			hMaterial->SetParameter("Cavity", hCavity);
		}

		flLastGenerate = CSMAKWindow::Get()->GetTime();
	}

	CSMAKWindow::Get()->Render();
	Application()->SwapBuffers();

	flLastTime = CSMAKWindow::Get()->GetTime();
}

void CCavityPanel::EndProgress()
{
	CProgressBar::Get()->SetVisible(false);
}

void CCavityPanel::Open(CConversionScene* pScene)
{
	CControl<CCavityPanel> hPanel = Get();

	// Get rid of the last one, in case we've changed the scene.
	if (hPanel.Get())
		hPanel->Close();

	s_hCavityPanel = new CCavityPanel(pScene);

	hPanel = Get();

	if (!hPanel)
		return;

	hPanel->SetVisible(true);
	hPanel->Layout();
}

CControl<CCavityPanel> CCavityPanel::Get()
{
	CControl<CCavityPanel> hControl(s_hCavityPanel);
	if (hControl)
		return hControl;

	return CControl<CCavityPanel>();
}

void CCavityPanel::SetVisible(bool bVisible)
{
	m_oGenerator.StopGenerating();

	CMovablePanel::SetVisible(bVisible);
}
