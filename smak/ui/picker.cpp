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

#include "picker.h"

#include <modelconverter/convmesh.h>
#include <glgui/tree.h>

#include "smakwindow.h"
#include "smak_renderer.h"

using namespace glgui;

CPicker::CPicker(const tstring& sName, IEventListener* pCallback, IEventListener::Callback pfnCallback)
	: CMovablePanel(sName)
{
	m_pCallback = pCallback;
	m_pfnCallback = pfnCallback;

	m_bPopulated = false;

	m_pTree = new CTree(SMAKWindow()->GetSMAKRenderer()->GetArrowTexture(), SMAKWindow()->GetSMAKRenderer()->GetEditTexture(), SMAKWindow()->GetSMAKRenderer()->GetVisibilityTexture());
	m_pTree->SetBackgroundColor(g_clrBox);
	m_pTree->SetSelectedListener(this, Selected);
	AddControl(m_pTree);

	Open();
}

void CPicker::Layout()
{
	m_pTree->SetPos(10, 25);
	m_pTree->SetSize(GetWidth()-20, GetHeight()-35);

	CMovablePanel::Layout();
}

void CPicker::Think()
{
	if (!m_bPopulated)
	{
		m_pTree->ClearTree();
		PopulateTree();
		Layout();
	}

	m_bPopulated = true;

	CMovablePanel::Think();
}

void CPicker::SelectedCallback(const tstring& sArgs)
{
	CTreeNode* pSelectedNode = m_pTree->GetSelectedNode();
	if (pSelectedNode)
	{
		NodeSelected(pSelectedNode);
		m_pfnCallback(m_pCallback, "");
	}
}

void CPicker::Open()
{
	SetVisible(true);
	Layout();
}

CMeshInstancePicker::CMeshInstancePicker(IEventListener* pCallback, IEventListener::Callback pfnCallback)
	: CPicker("Pick a mesh", pCallback, pfnCallback)
{
	m_pPickedMeshInstance = NULL;
}

void CMeshInstancePicker::PopulateTree()
{
	CConversionScene* pScene = CSMAKWindow::Get()->GetScene();

	for (size_t i = 0; i < pScene->GetNumScenes(); i++)
		PopulateTreeNode(NULL, pScene->GetScene(i));
}

void CMeshInstancePicker::PopulateTreeNode(glgui::CTreeNode* pTreeNode, CConversionSceneNode* pSceneNode)
{
	if (!pSceneNode->GetNumChildren() && !pSceneNode->GetNumMeshInstances())
		return;

	size_t iNode;
	glgui::CTreeNode* pChildNode;
	if (pTreeNode)
	{
		iNode = pTreeNode->AddNode<CConversionSceneNode>(pSceneNode->GetName(), pSceneNode);
		pChildNode = pTreeNode->GetNode(iNode);
	}
	else
	{
		iNode = m_pTree->AddNode<CConversionSceneNode>(pSceneNode->GetName(), pSceneNode);
		pChildNode = m_pTree->GetNode(iNode);
	}

	for (size_t i = 0; i < pSceneNode->GetNumChildren(); i++)
		PopulateTreeNode(pChildNode, pSceneNode->GetChild(i));

	// Child never got any nodes, remove it and get on with our lives.
	if (!pChildNode->GetNumNodes() && !pSceneNode->GetNumMeshInstances())
	{
		m_pTree->RemoveNode(pChildNode);
		return;
	}

	for (size_t m = 0; m < pSceneNode->GetNumMeshInstances(); m++)
	{
		size_t iMeshInstanceNode = pChildNode->AddNode<CConversionMeshInstance>(pSceneNode->GetMeshInstance(m)->GetMesh()->GetName(), pSceneNode->GetMeshInstance(m));
		CTreeNode* pMeshInstanceNode = pChildNode->GetNode(iMeshInstanceNode);
		pMeshInstanceNode->SetIcon(SMAKWindow()->GetSMAKRenderer()->GetMeshesNodeTexture());
	}
}

void CMeshInstancePicker::NodeSelected(CTreeNode* pNode)
{
	CTreeNodeObject<CConversionMeshInstance>* pMeshNode = dynamic_cast<CTreeNodeObject<CConversionMeshInstance>*>(pNode);

	if (!pMeshNode)
		return;

	m_pPickedMeshInstance = pMeshNode->GetObject();
}
