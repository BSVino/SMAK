/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "tree.h"

#include <tinker/shell.h>
#include <tinker/cvar.h>
#include <renderer/renderingcontext.h>

#include "rootpanel.h"
#include "scrollbar.h"

using namespace glgui;

CTree::CTree(const CMaterialHandle& hArrowMaterial, const CMaterialHandle& hEditMaterial, const CMaterialHandle& hVisibilityMaterial)
	: CPanel(0, 0, 10, 10)
{
	m_iHilighted = ~0;
	m_iSelected = ~0;

	m_hArrowMaterial = hArrowMaterial;
	m_hVisibilityMaterial = hVisibilityMaterial;
	m_hEditMaterial = hEditMaterial;

	m_pfnSelectedCallback = NULL;
	m_pSelectedListener = NULL;

	m_pfnDroppedCallback = NULL;
	m_pDroppedListener = NULL;

	m_bMouseDown = false;

	CRootPanel::Get()->AddDroppable(this);

	SetVerticalScrollBarEnabled(true);
	SetScissoring(true);
}

CTree::~CTree()
{
	if (RootPanel())
		RootPanel()->RemoveDroppable(this);
}

void CTree::Layout()
{
	m_flCurrentHeight = 0;
	m_flCurrentDepth = 0;

	for (size_t i = 0; i < m_ahNodes.size(); i++)
		m_ahNodes[i]->LayoutNode();

	CPanel::Layout();
}

void CTree::Think()
{
	int mx, my;
	CRootPanel::GetFullscreenMousePos(mx, my);

	m_iHilighted = ~0;
	for (size_t i = 0; i < m_ahAllNodes.size(); i++)
	{
		CBaseControl* pNode = m_ahAllNodes[i];

		if (!pNode->IsVisible())
			continue;

		float x, y, w, h;
		pNode->GetAbsDimensions(x, y, w, h);

		if (mx >= x && my >= y && mx < x+w && my < y+h)
		{
			m_iHilighted = i;
			break;
		}
	}

	if (m_bMouseDown && abs(mx - m_iMouseDownX) > 10 && abs(my - m_iMouseDownY) && GetSelectedNode() && !CRootPanel::Get()->GetCurrentDraggable())
	{
		m_hDragging = GetSelectedNode();
		CRootPanel::Get()->DragonDrop(this);
		m_bMouseDown = false;
	}

	CPanel::Think();
}

void CTree::Paint()
{
	float x = 0, y = 0;
	GetAbsPos(x, y);
	Paint(x, y);
}

void CTree::Paint(float x, float y)
{
	Paint(x, y, m_flW, m_flH);
}

void CTree::Paint(float x, float y, float w, float h)
{
	Color clrHilight = g_clrBoxHi;
	clrHilight.SetAlpha(100);
	Color clrSelected = g_clrBoxHi;

	bool bScissor = m_bScissoring;
	float sx, sy;
	if (bScissor)
	{
		GetAbsPos(sx, sy);

		CRootPanel::GetContext()->SetUniform("bScissor", true);
		CRootPanel::GetContext()->SetUniform("vecScissor", Vector4D(sx, sy, GetWidth(), GetHeight()));
	}

	if (m_iHilighted != ~0)
	{
		CBaseControl* pNode = m_ahAllNodes[m_iHilighted];
		float cx, cy, cw, ch;
		pNode->GetAbsDimensions(cx, cy, cw, ch);
		CRootPanel::PaintRect(cx, cy, cw, ch, clrHilight, 2);
	}

	if (m_iSelected < m_ahAllNodes.size() && m_ahAllNodes[m_iSelected]->IsVisible())
	{
		CBaseControl* pNode = m_ahAllNodes[m_iSelected];
		float cx, cy, cw, ch;
		pNode->GetAbsDimensions(cx, cy, cw, ch);
		CRootPanel::PaintRect(cx, cy, cw, ch, clrSelected, 2);
	}

	CPanel::Paint(x, y, w, h);
}

bool CTree::MousePressed(int code, int mx, int my)
{
	m_bMouseDown = true;
	m_iMouseDownX = mx;
	m_iMouseDownY = my;

	if (CPanel::MousePressed(code, mx, my))
		return true;

	m_iSelected = ~0;
	for (size_t i = 0; i < m_ahAllNodes.size(); i++)
	{
		CBaseControl* pNode = m_ahAllNodes[i];

		if (!pNode->IsVisible())
			continue;

		float x, y, w, h;
		pNode->GetAbsDimensions(x, y, w, h);

		if (mx >= x && my >= y && mx < x+w && my < y+h)
		{
			m_iSelected = i;
			CTreeNode* pTreeNode = dynamic_cast<CTreeNode*>(pNode);
			pTreeNode->Selected();
			return true;
		}
	}

	if (m_pSelectedListener)
		m_pfnSelectedCallback(m_pSelectedListener, "-1");

	return false;
}

bool CTree::MouseReleased(int code, int mx, int my)
{
	m_bMouseDown = false;

	if (CPanel::MouseReleased(code, mx, my))
		return true;

	return false;
}

bool CTree::MouseDoubleClicked(int code, int mx, int my)
{
	if (CPanel::MouseDoubleClicked(code, mx, my))
		return true;

	m_iSelected = ~0;
	for (size_t i = 0; i < m_ahAllNodes.size(); i++)
	{
		CBaseControl* pNode = m_ahAllNodes[i];

		if (!pNode->IsVisible())
			continue;

		float x, y, w, h;
		pNode->GetAbsDimensions(x, y, w, h);

		if (mx >= x && my >= y && mx < x+w && my < y+h)
		{
			m_iSelected = i;
			CTreeNode* pTreeNode = dynamic_cast<CTreeNode*>(pNode);
			pTreeNode->Selected();

			if (m_pfnConfirmedCallback)
				m_pfnConfirmedCallback(m_pConfirmedListener, sprintf("%d", GetSelectedNodeId()));

			return true;
		}
	}

	return false;
}

CControlHandle CTree::AddControl(CBaseControl* pControl, bool bToTail)
{
	CControlHandle hControl = BaseClass::AddControl(pControl, bToTail);

	if (pControl != m_hVerticalScrollBar && pControl != m_hHorizontalScrollBar)
	{
		CTreeNode* pTreeNode = dynamic_cast<CTreeNode*>(pControl);
		if (pTreeNode)
			m_ahAllNodes.push_back(CControl<CTreeNode>(pTreeNode->GetHandle()));
	}

	return hControl;
}

void CTree::RemoveControl(CBaseControl* pControl)
{
	BaseClass::RemoveControl(pControl);

	if (pControl != m_hVerticalScrollBar && pControl != m_hHorizontalScrollBar)
	{
		for (size_t i = m_ahAllNodes.size()-1; i < m_ahAllNodes.size(); i--)
		{
			CBaseControl* pNode = m_ahAllNodes[i];

			if (pControl == pNode)
			{
				m_ahAllNodes.erase(m_ahAllNodes.begin()+i);
				break;
			}
		}
	}
}

void CTree::ClearTree()
{
	m_iHilighted = ~0;
	m_iSelected = ~0;

	if (m_pSelectedListener)
		m_pfnSelectedCallback(m_pSelectedListener, "-1");

	for (size_t i = m_ahAllNodes.size()-1; i < m_ahAllNodes.size(); i--)
	{
		CBaseControl* pNode = m_ahAllNodes[i];

		RemoveControl(pNode);
	}

	m_ahNodes.clear();
	m_ahAllNodes.clear();
}

size_t CTree::AddNode(const tstring& sName)
{
	return AddNode(new CTreeNode(CControl<CTreeNode>(), CControl<CTree>(m_hThis), sName, "sans-serif"));
}

size_t CTree::AddNode(CBaseControl* pNode, size_t iPosition)
{
	if (iPosition == ~0)
		m_ahNodes.push_back(pNode->GetHandle());
	else
		m_ahNodes.insert(m_ahNodes.begin()+iPosition, pNode->GetHandle());

	AddControl(pNode, true);
	return m_ahNodes.size()-1;
}

void CTree::RemoveNode(CTreeNode* pNode)
{
	CBaseControl* pHilighted = NULL;
	CBaseControl* pSelected = NULL;

	// Tuck these away so we can find them again after the controls list has changed.
	if (m_iHilighted != ~0)
		pHilighted = m_ahAllNodes[m_iHilighted];
	if (m_iSelected != ~0)
		pSelected = m_ahAllNodes[m_iSelected];

	m_iHilighted = ~0;
	m_iSelected = ~0;

	for (size_t i = 0; i < m_ahNodes.size(); i++)
	{
		if (m_ahNodes[i] == pNode)
		{
			m_ahNodes.erase(m_ahNodes.begin()+i);
			break;
		}
		else
			m_ahNodes[i]->RemoveNode(pNode);
	}

	RemoveControl(pNode);

	// Figure out if our hilighted or selected controls were deleted.
	for (size_t c = 0; c < m_ahAllNodes.size(); c++)
	{
		if (m_ahAllNodes[c] == pHilighted)
			m_iHilighted = c;
		if (m_ahAllNodes[c] == pSelected)
			m_iSelected = c;
	}
}

CControl<CTreeNode> CTree::GetNode(size_t i)
{
	return m_ahNodes[i];
}

void CTree::SetSelectedNode(size_t iNode)
{
	TAssert(iNode < m_apControls.size() || iNode == ~0);

	if (iNode >= m_apControls.size())
		return;

	m_iSelected = iNode;

	if (m_pSelectedListener)
		m_pfnSelectedCallback(m_pSelectedListener, sprintf("%d", GetSelectedNodeId()));
}

void CTree::SetSelectedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	TAssert(pListener && pfnCallback || !pListener && !pfnCallback);
	m_pSelectedListener = pListener;
	m_pfnSelectedCallback = pfnCallback;
}

void CTree::SetConfirmedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	TAssert(pListener && pfnCallback || !pListener && !pfnCallback);
	m_pConfirmedListener = pListener;
	m_pfnConfirmedCallback = pfnCallback;
}

void CTree::SetDroppedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	TAssert(pListener && pfnCallback || !pListener && !pfnCallback);
	m_pDroppedListener = pListener;
	m_pfnDroppedCallback = pfnCallback;
}

void CTree::SetDraggable(IDraggable* pDraggable, bool bDelete)
{
	if (m_pDroppedListener)
		m_pfnDroppedCallback(m_pDroppedListener, "");

	AddNode(dynamic_cast<CTreeNode*>(pDraggable->MakeCopy()));
}

CTreeNode::CTreeNode(CControl<CTreeNode> hParent, CControl<CTree> hTree, const tstring& sText, const tstring& sFont)
	: CPanel(0, 0, 10, 10)
{
	m_hParent = hParent;
	m_hTree = hTree;

	m_bDraggable = false;

	m_hLabel = AddControl(new CLabel(0, 0, GetWidth(), GetHeight(), ""));
	m_hLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hLabel->SetText(sText.c_str());
	m_hLabel->SetFont(sFont, 11);
	m_hLabel->SetWrap(false);

	m_hExpandButton = AddControl(new CExpandButton(m_hTree->m_hArrowMaterial));
	m_hExpandButton->SetExpanded(false);
	m_hExpandButton->SetClickedListener(this, Expand);
}

CTreeNode::CTreeNode(const CTreeNode& c)
	: CPanel(GetLeft(), GetTop(), GetWidth(), GetHeight())
{
	m_hParent = c.m_hParent;
	m_hTree = c.m_hTree;

	m_hIconMaterial = c.m_hIconMaterial;
	m_bDraggable = false;

	m_hLabel = AddControl(new CLabel(c.m_hLabel->GetLeft(), c.m_hLabel->GetTop(), c.m_hLabel->GetWidth(), c.m_hLabel->GetHeight(), c.m_hLabel->GetText()));
	m_hLabel->SetAlign(c.m_hLabel->GetAlign());
	m_hLabel->SetFont("sans-serif", c.m_hLabel->GetFontFaceSize());

	m_hExpandButton = AddControl(new CExpandButton(m_hTree->m_hArrowMaterial));
	m_hExpandButton->SetExpanded(false);
	m_hExpandButton->SetClickedListener(this, Expand);
}

CTreeNode::~CTreeNode()
{
}

float CTreeNode::GetNodeHeight()
{
	return m_hLabel->GetTextHeight();
}

void CTreeNode::LayoutNode()
{
	float& flCurrentDepth = m_hTree->m_flCurrentDepth;
	float& flCurrentHeight = m_hTree->m_flCurrentHeight;

	float flHeight = GetNodeHeight();

	float x = flCurrentDepth*flHeight;
	float y = flCurrentHeight;
	float w = m_hTree->GetWidth() - flCurrentDepth*flHeight;
	float h = flHeight;

	SetPos(x, y);
	SetSize(w, h);

	m_hLabel->SetHeight(h);
	m_hLabel->SetWidth(w);
	if (m_hIconMaterial.IsValid())
		m_hLabel->SetPos(h+12, 0);
	else
		m_hLabel->SetPos(h, 0);

	m_hExpandButton->SetPos(0, 0);
	m_hExpandButton->SetSize(flHeight, flHeight);

	flCurrentHeight += flHeight;
	flCurrentHeight += GetNodeSpacing();

	if (IsExpanded())
	{
		flCurrentDepth += 1;
		for (size_t i = 0; i < m_ahNodes.size(); i++)
			m_ahNodes[i]->LayoutNode();
		flCurrentDepth -= 1;
	}
}

void CTreeNode::Paint(float x, float y, float w, float h)
{
	Paint(x, y, w, h, false);
}

void CTreeNode::Paint(float x, float y, float w, float h, bool bFloating)
{
	if (!IsVisible())
		return;

	if (m_hTree->m_hArrowMaterial.IsValid() && m_ahNodes.size())
		m_hExpandButton->Paint();

//	CBaseControl::PaintRect(x+15, y, w-25, h);

	float flIconSize = 0;
	if (m_hIconMaterial.IsValid())
	{
		flIconSize = 12;

		PaintTexture(m_hIconMaterial, x+12, y, flIconSize, flIconSize);
	}

	m_hLabel->Paint();

	if (m_hVisibilityButton)
		m_hVisibilityButton->Paint();

	if (m_hEditButton)
		m_hEditButton->Paint();

	// Skip CPanel, controls are painted in other ways.
	CBaseControl::Paint(x, y, w, h);
}

size_t CTreeNode::AddNode(const tstring& sName)
{
	return AddNode(new CTreeNode(CControl<CTreeNode>(m_hThis), CControl<CTree>(m_hTree), sName, "sans-serif"));
}

size_t CTreeNode::AddNode(CBaseControl* pNode)
{
	if (!m_ahNodes.size())
		SetExpanded(true);
	m_ahNodes.push_back(pNode->GetHandle());
	m_hTree->AddControl(pNode);
	return m_ahNodes.size()-1;
}

void CTreeNode::RemoveNode(CTreeNode* pNode)
{
	for (size_t i = 0; i < m_ahNodes.size(); i++)
	{
		if (m_ahNodes[i] == pNode)
		{
			m_ahNodes.erase(m_ahNodes.begin()+i);
			return;
		}
		m_ahNodes[i]->RemoveNode(pNode);
	}
}

CControl<CTreeNode> CTreeNode::GetNode(size_t i)
{
	return m_ahNodes[i];
}

void CTreeNode::Selected()
{
	if (m_hTree->m_pSelectedListener)
		m_hTree->m_pfnSelectedCallback(m_hTree->m_pSelectedListener, sprintf("%d", m_hTree->GetSelectedNodeId()));
}

bool CTreeNode::IsVisible()
{
	if (!CPanel::IsVisible())
		return false;

	if (!m_hParent)
		return true;

	CTreeNode* pNode = m_hParent;
	do
	{
		if (!pNode->IsExpanded())
			return false;
	}
	while (pNode = pNode->m_hParent);

	return true;
}

void CTreeNode::SetHoldingRect(const FRect&)
{
}

FRect CTreeNode::GetHoldingRect()
{
	return FRect(0, 0, 0, 0);
}

IDroppable* CTreeNode::GetDroppable()
{
	return NULL;
}

void CTreeNode::SetDroppable(IDroppable* pDroppable)
{
}

void CTreeNode::ExpandCallback(const tstring& sArgs)
{
	SetExpanded(!IsExpanded());
	m_hTree->Layout();
}

CTreeNode::CExpandButton::CExpandButton(const CMaterialHandle& hMaterial)
	: CPictureButton("*", hMaterial, false)
{
	m_bExpanded = false;
	m_flExpandedGoal = m_flExpandedCurrent = 0;
}

void CTreeNode::CExpandButton::Think()
{
	m_flExpandedCurrent = Approach(m_flExpandedGoal, m_flExpandedCurrent, (float)CRootPanel::Get()->GetFrameTime()*10);
}

CVar glgui_spinnyarrows("glgui_spinnyarrows", "off");

void CTreeNode::CExpandButton::Paint(float x, float y, float w, float h)
{
	MakeQuad();

	::CRenderingContext r(nullptr, true);

	if ((w < 0) ^ (h < 0))
		r.SetBackCulling(false);

	r.UseMaterial(m_hMaterial);

	r.SetBlend(BLEND_ALPHA);
	r.SetUniform("iBorder", 0);
	r.SetUniform("bHighlight", false);
	r.SetUniform("vecColor", Color(255, 255, 255));
	r.SetUniform("bDiffuse", true);
	r.SetUniform("bTexCoords", false);

	r.SetUniform("vecDimensions", Vector4D(-w/2, -h/2, w, h));

	r.Translate(Vector((float)x+w/2, (float)y+h/2, 0));
	r.Rotate(m_flExpandedCurrent*90-90, Vector(0, 0, 1));

	// Hehe.
	if (glgui_spinnyarrows.GetBool())
		r.Rotate((float)RootPanel()->GetTime()*200, Vector(0, 0, 1));

	r.BeginRenderVertexArray(s_iQuad);
	r.SetPositionBuffer((size_t)0u, 24);
	r.SetTexCoordBuffer(12, 24);
	r.SetCustomIntBuffer("iVertex", 1, 20, 24);
	r.EndRenderVertexArray(6);
}

void CTreeNode::CExpandButton::SetExpanded(bool bExpanded)
{
	m_bExpanded = bExpanded;
	m_flExpandedGoal = m_bExpanded?1.0f:0.0f;
}
