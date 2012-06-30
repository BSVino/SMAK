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

#include "droppablepanel.h"

#include "rootpanel.h"

using namespace glgui;

CDroppablePanel::CDroppablePanel(float x, float y, float w, float h)
	: CPanel(x, y, w, h)
{
	m_bGrabbable = true;

	CRootPanel::Get()->AddDroppable(this);
};

CDroppablePanel::~CDroppablePanel()
{
	if (CRootPanel::Get())
		CRootPanel::Get()->RemoveDroppable(this);
}

void CDroppablePanel::Paint(float x, float y, float w, float h)
{
	if (!IsVisible())
		return;

	for (size_t i = 0; i < m_apDraggables.size(); i++)
	{
		// Translate this location to the child's local space.
		float ax, ay;
		FRect c = m_apDraggables[i]->GetHoldingRect();
		GetAbsPos(ax, ay);
		m_apDraggables[i]->Paint(c.x+x-ax, c.y+y-ay);
	}

	CPanel::Paint(x, y, w, h);
}

void CDroppablePanel::SetSize(float w, float h)
{
	CPanel::SetSize(w, h);
	for (size_t i = 0; i < m_apDraggables.size(); i++)
		m_apDraggables[i]->SetHoldingRect(GetHoldingRect());
}

void CDroppablePanel::SetPos(float x, float y)
{
	CPanel::SetPos(x, y);
	for (size_t i = 0; i < m_apDraggables.size(); i++)
		m_apDraggables[i]->SetHoldingRect(GetHoldingRect());
}

bool CDroppablePanel::MousePressed(int code, int mx, int my)
{
	if (!IsVisible())
		return false;

	if (m_bGrabbable && m_apDraggables.size() > 0)
	{
		FRect r = GetHoldingRect();
		if (code == 0 &&
			mx >= r.x &&
			my >= r.y &&
			mx < r.x + r.w &&
			my < r.y + r.h)
		{
			CRootPanel::Get()->DragonDrop(this);
			return true;
		}
	}

	return CPanel::MousePressed(code, mx, my);
}

void CDroppablePanel::SetDraggable(IDraggable* pDragged, bool bDelete)
{
	ClearDraggables(bDelete);

	AddDraggable(pDragged);
}

void CDroppablePanel::AddDraggable(IDraggable* pDragged)
{
	if (pDragged)
	{
		m_apDraggables.push_back(pDragged);
		pDragged->SetHoldingRect(GetHoldingRect());
		pDragged->SetDroppable(this);
	}
}

void CDroppablePanel::ClearDraggables(bool bDelete)
{
	m_apDraggables.clear();
}

IDraggable* CDroppablePanel::GetDraggable(int i)
{
	return m_apDraggables[i];
}
