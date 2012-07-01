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

#include "scenetree.h"

#include <tinker_platform.h>
#include <glgui/rootpanel.h>
#include <glgui/tree.h>
#include <glgui/filedialog.h>
#include <glgui/colorpicker.h>
#include <textures/texturelibrary.h>
#include <textures/materiallibrary.h>

#include "smak_renderer.h"
#include "smakwindow.h"

using namespace glgui;

CControl<CSceneTreePanel> CSceneTreePanel::s_hSceneTreePanel;

CSceneTreePanel::CSceneTreePanel(CConversionScene* pScene)
	: CMovablePanel("Scene Tree")
{
	m_pScene = pScene;

	HasCloseButton(false);
	SetClearBackground(true);

	SetPos(50, 150);

	m_hMaterialEditor = NULL;

	m_iLastSelectedMaterial = ~0;

	m_hTree = AddControl(new CTree(SMAKWindow()->GetSMAKRenderer()->GetArrowTexture(), SMAKWindow()->GetSMAKRenderer()->GetEditTexture(), SMAKWindow()->GetSMAKRenderer()->GetVisibilityTexture()));
	m_hTree->SetSelectedListener(this, Selected);
}

CSceneTreePanel::~CSceneTreePanel()
{
}

void CSceneTreePanel::Layout()
{
	m_hTree->SetPos(5, HEADER_HEIGHT);
	m_hTree->SetSize(GetWidth() - 5, GetHeight() - HEADER_HEIGHT - 20);

	CMovablePanel::Layout();
}

void CSceneTreePanel::UpdateTree()
{
	m_iLastSelectedMaterial = ~0;

	m_hTree->ClearTree();

	AddAllToTree();

	Layout();
}

void CSceneTreePanel::Paint(float x, float y, float w, float h)
{
	CMovablePanel::Paint(x, y, w, h);
}

void CSceneTreePanel::AddAllToTree()
{
	size_t iMaterialsNode = m_hTree->AddNode("Materials");
	CTreeNode* pMaterialsNode = m_hTree->GetNode(iMaterialsNode);
	pMaterialsNode->SetIcon(SMAKWindow()->GetSMAKRenderer()->GetMaterialsNodeTexture());

	size_t i;
	for (i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		size_t iMaterialNode = pMaterialsNode->AddNode<CConversionMaterial>(m_pScene->GetMaterial(i)->GetName(), m_pScene->GetMaterial(i));
		CTreeNode* pMaterialNode = pMaterialsNode->GetNode(iMaterialNode);
		pMaterialNode->AddVisibilityButton();
		dynamic_cast<CTreeNodeObject<CConversionMaterial>*>(pMaterialNode)->AddEditButton(::OpenMaterialEditor);
	}

	// Don't overload the screen.
	if (pMaterialsNode->m_ahNodes.size() > 10)
		pMaterialsNode->SetExpanded(false);

	size_t iMeshesNode = m_hTree->AddNode("Meshes");
	CTreeNode* pMeshesNode = m_hTree->GetNode(iMeshesNode);
	pMeshesNode->SetIcon(SMAKWindow()->GetSMAKRenderer()->GetMeshesNodeTexture());

	for (i = 0; i < m_pScene->GetNumMeshes(); i++)
		pMeshesNode->AddNode<CConversionMesh>(m_pScene->GetMesh(i)->GetName(), m_pScene->GetMesh(i));

	if (pMeshesNode->m_ahNodes.size() > 10)
		pMeshesNode->SetExpanded(false);

	size_t iScenesNode = m_hTree->AddNode("Scenes");
	CTreeNode* pScenesNode = m_hTree->GetNode(iScenesNode);
	pScenesNode->SetIcon(SMAKWindow()->GetSMAKRenderer()->GetScenesNodeTexture());

	for (i = 0; i < m_pScene->GetNumScenes(); i++)
		AddNodeToTree(pScenesNode, m_pScene->GetScene(i));

	if (pScenesNode->m_ahNodes.size() > 10)
		pScenesNode->SetExpanded(false);
}

void CSceneTreePanel::AddNodeToTree(glgui::CTreeNode* pTreeNode, CConversionSceneNode* pSceneNode)
{
	size_t iNode = pTreeNode->AddNode<CConversionSceneNode>(pSceneNode->GetName(), pSceneNode);
	for (size_t i = 0; i < pSceneNode->GetNumChildren(); i++)
		AddNodeToTree(pTreeNode->GetNode(iNode), pSceneNode->GetChild(i));

	for (size_t m = 0; m < pSceneNode->GetNumMeshInstances(); m++)
	{
		size_t iMeshInstanceNode = pTreeNode->GetNode(iNode)->AddNode<CConversionMeshInstance>(pSceneNode->GetMeshInstance(m)->GetMesh()->GetName(), pSceneNode->GetMeshInstance(m));
		CTreeNode* pMeshInstanceNode = pTreeNode->GetNode(iNode)->GetNode(iMeshInstanceNode);
		pMeshInstanceNode->SetIcon(SMAKWindow()->GetSMAKRenderer()->GetMeshesNodeTexture());
		pMeshInstanceNode->AddVisibilityButton();
		pMeshInstanceNode->SetDraggable(true);

		for (size_t s = 0; s < pSceneNode->GetMeshInstance(m)->GetMesh()->GetNumMaterialStubs(); s++)
		{
			CConversionMaterialMap* pMaterialMap = pSceneNode->GetMeshInstance(m)->GetMappedMaterial(s);

			if (!pMaterialMap)
				continue;

			if (!m_pScene->GetMaterial(pMaterialMap->m_iMaterial))
				continue;

			size_t iMapNode = pMeshInstanceNode->AddNode<CConversionMaterialMap>(m_pScene->GetMaterial(pMaterialMap->m_iMaterial)->GetName(), pMaterialMap);
			CTreeNode* pMeshMapNode = pMeshInstanceNode->GetNode(iMapNode);
			pMeshMapNode->AddVisibilityButton();
		}
	}
}

void OpenMaterialEditor(CConversionMaterial* pMaterial, const tstring& sArgs)
{
	CSceneTreePanel::Get()->OpenMaterialEditor(pMaterial);
}

void CSceneTreePanel::OpenMaterialEditor(CConversionMaterial* pMaterial)
{
	CloseMaterialEditor();

	m_hMaterialEditor = new CMaterialEditor(pMaterial, m_hThis);

	if (!m_hMaterialEditor)
		return;

	m_hMaterialEditor->SetVisible(true);
	m_hMaterialEditor->Layout();
}

void CSceneTreePanel::CloseMaterialEditor()
{
	if (m_hMaterialEditor.Get())
		m_hMaterialEditor.DowncastStatic<CMaterialEditor>()->Close();
}

void CSceneTreePanel::SelectedCallback(const tstring& sArgs)
{
	if (!m_hTree->GetSelectedNode())
		return;

	CTreeNodeObject<CConversionMaterial>* pMaterialNode = m_hTree->GetSelectedNode().Downcast<CTreeNodeObject<CConversionMaterial>>();
	if (pMaterialNode)
	{
		CConversionMaterial* pMaterial = pMaterialNode->GetObject();
		for (size_t i = 0; i < SMAKWindow()->GetScene()->GetNumMaterials(); i++)
		{
			if (pMaterial == SMAKWindow()->GetScene()->GetMaterial(i))
			{
				m_iLastSelectedMaterial = i;
				return;
			}
		}

		return;
	}
}

void CSceneTreePanel::Open(CConversionScene* pScene)
{
	CSceneTreePanel* pPanel = Get();

	if (!pPanel)
		pPanel = s_hSceneTreePanel = new CSceneTreePanel(pScene);

	if (!pPanel)
		return;

	pPanel->SetVisible(true);
	pPanel->Layout();
}

CSceneTreePanel* CSceneTreePanel::Get()
{
	return s_hSceneTreePanel;
}

CMaterialEditor::CMaterialEditor(CConversionMaterial* pMaterial, CControl<CSceneTreePanel> hSceneTree)
	: CMovablePanel("Material Properties")
{
	m_pMaterial = pMaterial;
	m_hSceneTree = hSceneTree;

	m_pScene = CSMAKWindow::Get()->GetScene();

	for (size_t i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		if (m_pScene->GetMaterial(i) == m_pMaterial)
		{
			m_iMaterial = i;
			break;
		}
	}

	float x, y;
	m_hSceneTree->GetAbsPos(x, y);

	SetPos(x + m_hSceneTree->GetWidth(), y);
	SetSize(500, 300);

	m_hDiffuseLabel = AddControl(new CLabel(0, 0, 1, 1, "Diffuse map: "));
	m_hDiffuseFile = AddControl(new CButton(0, 0, 1, 1, ""));
	m_hDiffuseFile->SetAlign(CLabel::TA_LEFTCENTER);
	m_hDiffuseFile->SetWrap(false);
	m_hDiffuseFile->SetClickedListener(this, ChooseDiffuse);
	m_hDiffuseRemove = AddControl(new CButton(0, 0, 70, 20, "Remove"));
	m_hDiffuseRemove->SetClickedListener(this, RemoveDiffuse);

	m_hNormalLabel = AddControl(new CLabel(0, 0, 1, 1, "Normal map: "));
	m_hNormalFile = AddControl(new CButton(0, 0, 1, 1, ""));
	m_hNormalFile->SetAlign(CLabel::TA_LEFTCENTER);
	m_hNormalFile->SetWrap(false);
	m_hNormalFile->SetClickedListener(this, ChooseNormal);
	m_hNormalRemove = AddControl(new CButton(0, 0, 70, 20, "Remove"));
	m_hNormalRemove->SetClickedListener(this, RemoveNormal);

	m_hAmbientLabel = AddControl(new CLabel(0, 0, 1, 1, "Ambient: "));
	m_hAmbientColorPicker = AddControl(new CColorPickerButton());
	m_hAmbientColorPicker->SetChangedListener(this, SetAmbient);

	m_hDiffuseSelectorLabel = AddControl(new CLabel(0, 0, 1, 1, "Diffuse: "));
	m_hDiffuseColorPicker = AddControl(new CColorPickerButton());
	m_hDiffuseColorPicker->SetChangedListener(this, SetDiffuse);

	m_hSpecularLabel = AddControl(new CLabel(0, 0, 1, 1, "Specular: "));
	m_hSpecularColorPicker = AddControl(new CColorPickerButton());
	m_hSpecularColorPicker->SetChangedListener(this, SetSpecular);

	m_hEmissiveLabel = AddControl(new CLabel(0, 0, 1, 1, "Emissive: "));
	m_hEmissiveColorPicker = AddControl(new CColorPickerButton());
	m_hEmissiveColorPicker->SetChangedListener(this, SetEmissive);

	m_hShininessLabel = AddControl(new CLabel(0, 0, 1, 1, "Shininess: "));
	m_hShininessSelector = AddControl(new CScrollSelector<float>());
	SetupSelector(m_hShininessSelector, 128);
	m_hShininessSelector->SetSelectedListener(this, SetShininess);


	m_hName->AppendText(" - ");
	m_hName->AppendText(m_pMaterial->GetName().c_str());

	Layout();
}

void CMaterialEditor::SetupSelector(CScrollSelector<float>* pSelector, float flMaxValue)
{
	pSelector->AddSelection(CScrollSelection<float>(0*flMaxValue/20, "0%"));
	pSelector->AddSelection(CScrollSelection<float>(1*flMaxValue/20, "5%"));
	pSelector->AddSelection(CScrollSelection<float>(2*flMaxValue/20, "10%"));
	pSelector->AddSelection(CScrollSelection<float>(3*flMaxValue/20, "15%"));
	pSelector->AddSelection(CScrollSelection<float>(4*flMaxValue/20, "20%"));
	pSelector->AddSelection(CScrollSelection<float>(5*flMaxValue/20, "25%"));
	pSelector->AddSelection(CScrollSelection<float>(6*flMaxValue/20, "30%"));
	pSelector->AddSelection(CScrollSelection<float>(7*flMaxValue/20, "35%"));
	pSelector->AddSelection(CScrollSelection<float>(8*flMaxValue/20, "40%"));
	pSelector->AddSelection(CScrollSelection<float>(9*flMaxValue/20, "45%"));
	pSelector->AddSelection(CScrollSelection<float>(10*flMaxValue/20, "50%"));
	pSelector->AddSelection(CScrollSelection<float>(11*flMaxValue/20, "55%"));
	pSelector->AddSelection(CScrollSelection<float>(12*flMaxValue/20, "60%"));
	pSelector->AddSelection(CScrollSelection<float>(13*flMaxValue/20, "65%"));
	pSelector->AddSelection(CScrollSelection<float>(14*flMaxValue/20, "70%"));
	pSelector->AddSelection(CScrollSelection<float>(15*flMaxValue/20, "75%"));
	pSelector->AddSelection(CScrollSelection<float>(16*flMaxValue/20, "80%"));
	pSelector->AddSelection(CScrollSelection<float>(17*flMaxValue/20, "85%"));
	pSelector->AddSelection(CScrollSelection<float>(18*flMaxValue/20, "90%"));
	pSelector->AddSelection(CScrollSelection<float>(19*flMaxValue/20, "95%"));
	pSelector->AddSelection(CScrollSelection<float>(20*flMaxValue/20, "100%"));
}

void CMaterialEditor::Layout()
{
	float flHeight = HEADER_HEIGHT+10;

	m_hDiffuseLabel->SetPos(10, flHeight);
	m_hDiffuseLabel->EnsureTextFits();

	float x, y;
	m_hDiffuseLabel->GetPos(x, y);

	float flControlHeight = y;

	float flDiffuseRight = x + m_hDiffuseLabel->GetWidth();

	m_hDiffuseFile->SetPos(flDiffuseRight, flHeight);
	if (m_pMaterial->GetDiffuseTexture().length())
		m_hDiffuseFile->SetText(m_pMaterial->GetDiffuseTexture().c_str());
	else
		m_hDiffuseFile->SetText("Choose...");
	m_hDiffuseFile->SetSize(0, 0);
	m_hDiffuseFile->EnsureTextFits();
	if (m_hDiffuseFile->GetWidth() + m_hDiffuseLabel->GetWidth() + 10 > GetWidth())
		m_hDiffuseFile->SetSize(GetWidth() - m_hDiffuseLabel->GetWidth() - 10, m_hDiffuseFile->GetHeight());

	flHeight += flControlHeight;

	m_hDiffuseRemove->SetPos(GetWidth() - m_hDiffuseRemove->GetWidth() - 10, flHeight);

	flHeight += flControlHeight;

	m_hNormalLabel->SetPos(10, flHeight);
	m_hNormalLabel->EnsureTextFits();

	m_hNormalLabel->GetPos(x, y);
	float flNormalRight = x + m_hNormalLabel->GetWidth();

	m_hNormalFile->SetPos(flNormalRight, flHeight);
	if (m_pMaterial->GetNormalTexture().length())
		m_hNormalFile->SetText(m_pMaterial->GetNormalTexture().c_str());
	else
		m_hNormalFile->SetText("Choose...");
	m_hNormalFile->SetSize(0, 0);
	m_hNormalFile->EnsureTextFits();
	if (m_hNormalFile->GetWidth() + m_hNormalLabel->GetWidth() + 10 > GetWidth())
		m_hNormalFile->SetSize(GetWidth() - m_hNormalLabel->GetWidth() - 10, m_hNormalFile->GetHeight());

	flHeight += flControlHeight;

	m_hNormalRemove->SetPos(GetWidth() - m_hNormalRemove->GetWidth() - 10, flHeight);

	flHeight += flControlHeight;

	float flLabelHeight = flHeight + 10;

	m_hAmbientLabel->SetPos(10, flLabelHeight);
	m_hAmbientLabel->EnsureTextFits();

	m_hAmbientLabel->GetPos(x, y);
	float flAmbientRight = x + m_hAmbientLabel->GetWidth();

	float flColorPickerHeight = flLabelHeight + m_hAmbientLabel->GetHeight()/2 - m_hAmbientColorPicker->GetHeight()/2;

	m_hAmbientColorPicker->SetPos(flAmbientRight, flColorPickerHeight);
	m_hAmbientColorPicker->SetColor(m_pMaterial->m_vecAmbient);

	m_hDiffuseSelectorLabel->SetPos(GetWidth()/4+5, flLabelHeight);
	m_hDiffuseSelectorLabel->EnsureTextFits();

	m_hDiffuseSelectorLabel->GetPos(x, y);
	flDiffuseRight = x + m_hDiffuseSelectorLabel->GetWidth();

	m_hDiffuseColorPicker->SetPos(flDiffuseRight, flColorPickerHeight);
	m_hDiffuseColorPicker->SetColor(m_pMaterial->m_vecDiffuse);

	m_hSpecularLabel->SetPos(GetWidth()/2+5, flLabelHeight);
	m_hSpecularLabel->EnsureTextFits();

	m_hSpecularLabel->GetPos(x, y);
	float flSpecularRight = x + m_hSpecularLabel->GetWidth();

	m_hSpecularColorPicker->SetPos(flSpecularRight, flColorPickerHeight);
	m_hSpecularColorPicker->SetColor(m_pMaterial->m_vecSpecular);

	m_hEmissiveLabel->SetPos(GetWidth()*3/4+5, flLabelHeight);
	m_hEmissiveLabel->EnsureTextFits();

	m_hEmissiveLabel->GetPos(x, y);
	float flEmissiveRight = x + m_hEmissiveLabel->GetWidth();

	m_hEmissiveColorPicker->SetPos(flEmissiveRight, flColorPickerHeight);
	m_hEmissiveColorPicker->SetColor(m_pMaterial->m_vecEmissive);

	flHeight += flControlHeight + 15;

	m_hShininessLabel->SetPos(10, flHeight);
	m_hShininessLabel->EnsureTextFits();

	m_hShininessLabel->GetPos(x, y);
	float flShininessRight = x + m_hShininessLabel->GetWidth();

	m_hShininessSelector->SetPos(flShininessRight, flHeight);
	m_hShininessSelector->SetSelection((int)(m_pMaterial->m_flShininess/127*20));
	m_hShininessSelector->SetRight(GetWidth()/2-5);

	CMovablePanel::Layout();
}

void CMaterialEditor::ChooseDiffuseCallback(const tstring& sArgs)
{
	CFileDialog::ShowOpenDialog("", ".bmp;.jpg;.png;.tga;.psd;.gif;.tif", this, OpenDiffuse);
}

void CMaterialEditor::OpenDiffuseCallback(const tstring& sArgs)
{
	if (!sArgs.length())
		return;

	CTextureHandle hTexture = CTextureLibrary::AddTexture(sArgs);

	if (!hTexture.IsValid())
		return;

	CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[m_iMaterial];

	hMaterial->SetParameter("DiffuseTexture", hTexture);

	m_pMaterial->m_sDiffuseTexture = sArgs;

	Layout();
}

void CMaterialEditor::ChooseNormalCallback(const tstring& sArgs)
{
	CFileDialog::ShowOpenDialog("", ".bmp;.jpg;.png;.tga;.psd;.gif;.tif", this, OpenNormal);
}

void CMaterialEditor::OpenNormalCallback(const tstring& sArgs)
{
	if (!sArgs.length())
		return;

	CTextureHandle hTexture = CTextureLibrary::AddTexture(sArgs);

	if (!hTexture.IsValid())
		return;

	CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[m_iMaterial];

	hMaterial->SetParameter("Normal", hTexture);

	m_pMaterial->m_sNormalTexture = sArgs;

	Layout();
}

void CMaterialEditor::RemoveDiffuseCallback(const tstring& sArgs)
{
	CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[m_iMaterial];

	hMaterial->SetParameter("DiffuseTexture", CTextureHandle());

	m_pMaterial->m_sDiffuseTexture = "";

	Layout();
}

void CMaterialEditor::RemoveNormalCallback(const tstring& sArgs)
{
	CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[m_iMaterial];

	hMaterial->SetParameter("Normal", CTextureHandle());

	m_pMaterial->m_sNormalTexture = "";

	Layout();
}

void CMaterialEditor::SetAmbientCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecAmbient = m_hAmbientColorPicker->GetColorVector();

	CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[m_iMaterial];
	hMaterial->SetParameter("Ambient", m_pMaterial->m_vecAmbient);
}

void CMaterialEditor::SetDiffuseCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecDiffuse = m_hDiffuseColorPicker->GetColorVector();

	CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[m_iMaterial];
	hMaterial->SetParameter("Diffuse", m_pMaterial->m_vecDiffuse);
}

void CMaterialEditor::SetSpecularCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecSpecular = m_hSpecularColorPicker->GetColorVector();

	CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[m_iMaterial];
	hMaterial->SetParameter("Specular", m_pMaterial->m_vecSpecular);
}

void CMaterialEditor::SetEmissiveCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecEmissive = m_hEmissiveColorPicker->GetColorVector();

	CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[m_iMaterial];
	hMaterial->SetParameter("Emissive", m_pMaterial->m_vecEmissive);
}

void CMaterialEditor::SetShininessCallback(const tstring& sArgs)
{
	m_pMaterial->m_flShininess = m_hShininessSelector->GetSelectionValue();

	CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[m_iMaterial];
	hMaterial->SetParameter("Shininess", m_pMaterial->m_flShininess);
}

void CMaterialEditor::SetVisible(bool bVisible)
{
	BaseClass::SetVisible(bVisible);

	if (!bVisible)
	{
		m_hDiffuseColorPicker->Pop(true, true);
		m_hAmbientColorPicker->Pop(true, true);
		m_hEmissiveColorPicker->Pop(true, true);
		m_hSpecularColorPicker->Pop(true, true);
	}
}

