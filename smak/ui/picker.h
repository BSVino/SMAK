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

#ifndef SMAK_PICKER_H
#define SMAK_PICKER_H

#include <glgui/movablepanel.h>

#include "smakwindow_ui.h"

class CPicker : public glgui::CMovablePanel
{
public:
								CPicker(const tstring& sName, IEventListener* pCallback, IEventListener::Callback pfnCallback);

public:
	virtual void				Layout();

	virtual void				Think();

	virtual void				PopulateTree() {};

	EVENT_CALLBACK(CPicker,		Selected);

	virtual void				NodeSelected(glgui::CTreeNode* pNode) {};

protected:
	virtual void				Open();

	glgui::IEventListener::Callback	m_pfnCallback;
	glgui::IEventListener*		m_pCallback;

	glgui::CTree*				m_pTree;

	bool						m_bPopulated;
};

class CMeshInstancePicker : public CPicker
{
public:
								CMeshInstancePicker(IEventListener* pCallback, IEventListener::Callback pfnCallback);

public:
	virtual void				PopulateTree();
	virtual void				PopulateTreeNode(glgui::CTreeNode* pTreeNode, class CConversionSceneNode* pSceneNode);

	virtual void				NodeSelected(glgui::CTreeNode* pNode);

	virtual class CConversionMeshInstance*	GetPickedMeshInstance() { return m_pPickedMeshInstance; }

protected:
	class CConversionMeshInstance*	m_pPickedMeshInstance;
};

#endif
