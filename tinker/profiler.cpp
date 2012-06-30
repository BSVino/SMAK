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

#include "profiler.h"

#include <strutils.h>

#include <tinker/application.h>
#include <tinker/cvar.h>
#include <renderer/renderingcontext.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>

CVar prof_enable("prof_enable", "no");

CProfileScope::CProfileScope(const tstring& sName)
{
	m_sName = sName;

	CProfiler::PushScope(this);
}

CProfileScope::~CProfileScope()
{
	CProfiler::PopScope(this);
}

CPerfBlock::CPerfBlock(const tstring& sName, CPerfBlock* pParent)
{
	m_pParent = pParent;
	m_sName = sName;
	m_flTime = 0;
}

CPerfBlock* CPerfBlock::GetChild(const tstring& sName)
{
	tmap<tstring, CPerfBlock*>::iterator it = m_apPerfBlocks.find(sName);

	if (it == m_apPerfBlocks.end())
		return NULL;

	return it->second;
}

CPerfBlock* CPerfBlock::AddChild(const tstring& sName)
{
	CPerfBlock* pChild = new CPerfBlock(sName, this);
	m_apPerfBlocks[sName] = pChild;
	return pChild;
}

void CPerfBlock::BeginFrame()
{
	m_flTime = 0;

	for (tmap<tstring, CPerfBlock*>::iterator it = m_apPerfBlocks.begin(); it != m_apPerfBlocks.end(); it++)
		it->second->BeginFrame();
}

void CPerfBlock::BlockStarted()
{
	m_flTimeBlockStarted = CApplication::Get()->GetTime();
}

void CPerfBlock::BlockEnded()
{
	double flTimeBlockEnded = CApplication::Get()->GetTime();

	m_flTime += flTimeBlockEnded - m_flTimeBlockStarted;
}

CPerfBlock* CProfiler::s_pTopBlock = NULL;
CPerfBlock* CProfiler::s_pBottomBlock = NULL;
bool CProfiler::s_bProfiling = false;

void CProfiler::BeginFrame()
{
	s_bProfiling = prof_enable.GetBool();

	if (s_pBottomBlock)
		s_pBottomBlock->BeginFrame();

	// Just in case.
	s_pTopBlock = NULL;
}

void CProfiler::PushScope(CProfileScope* pScope)
{
	if (!IsProfiling())
		return;

	CPerfBlock* pBlock = NULL;

	if (!s_pTopBlock)
	{
		if (!s_pBottomBlock)
			s_pBottomBlock = new CPerfBlock(pScope->GetName(), NULL);

		pBlock = s_pBottomBlock;
	}
	else
	{
		pBlock = s_pTopBlock->GetChild(pScope->GetName());

		if (!pBlock)
			pBlock = s_pTopBlock->AddChild(pScope->GetName());
	}

	pBlock->BlockStarted();
	s_pTopBlock = pBlock;
}

void CProfiler::PopScope(CProfileScope* pScope)
{
	if (!IsProfiling())
		return;

	TAssert(s_pTopBlock);
	if (!s_pTopBlock)
		return;

	TAssert(pScope->GetName() == s_pTopBlock->GetName());

	s_pTopBlock->BlockEnded();

	s_pTopBlock = s_pTopBlock->GetParent();
}

void CProfiler::PopAllScopes()
{
	if (!IsProfiling())
		return;

	CPerfBlock* pBlock = s_pTopBlock;

	while (pBlock)
	{
		pBlock->BlockEnded();
		pBlock = pBlock->GetParent();
	}

	s_pTopBlock = NULL;
}

void CProfiler::Render()
{
	if (!IsProfiling())
		return;

	if (!s_pBottomBlock)
		return;

	PopAllScopes();

	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	float flCurrLeft = flWidth - 400;
	float flCurrTop = 200;

	Matrix4x4 mProjection = Matrix4x4::ProjectOrthographic(0, flWidth, flHeight, 0, -1000, 1000);

	CRenderingContext c;

	c.SetProjection(mProjection);
	c.UseProgram("gui");
	c.SetDepthTest(false);
	c.UseFrameBuffer(NULL);

	glgui::CBaseControl::PaintRect(flCurrLeft, flCurrTop, 400, 800, Color(0, 0, 0, 150), 5, true);

	Render(s_pBottomBlock, flCurrLeft, flCurrTop);
}

void CProfiler::Render(CPerfBlock* pBlock, float& flLeft, float& flTop)
{
	flLeft += 15;
	flTop += 15;

	Color clrBlock(255, 255, 255);
	if (pBlock->GetTime() < 0.005)
		clrBlock = Color(255, 255, 255, 150);

	glgui::CBaseControl::PaintRect(flLeft, flTop+1, (float)pBlock->GetTime()*5000, 1, clrBlock);

	tstring sName = pBlock->GetName();
	sName += sprintf(tstring(": %d ms"), (int)(pBlock->GetTime()*1000));
	glgui::CLabel::PaintText(sName, sName.length(), "sans-serif", 10, (float)flLeft, (float)flTop, clrBlock);

	for (tmap<tstring, CPerfBlock*>::iterator it = pBlock->m_apPerfBlocks.begin(); it != pBlock->m_apPerfBlocks.end(); it++)
		Render(it->second, flLeft, flTop);

	flLeft -= 15;
}
