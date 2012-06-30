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

#ifndef TINKER_PROFILER_H
#define TINKER_PROFILER_H

#include <tmap.h>
#include <tstring.h>
#include <tvector.h>

#define TPROF(name) CProfileScope _TProf(name);

class CProfileScope
{
public:
								CProfileScope(const tstring& sName);
								~CProfileScope();

public:
	tstring						GetName() { return m_sName; };

protected:
	tstring						m_sName;
};

class CPerfBlock
{
public:
								CPerfBlock(const tstring& sName, CPerfBlock* pParent);

public:
	CPerfBlock*					GetParent() { return m_pParent; };

	CPerfBlock*					GetChild(const tstring& sName);
	CPerfBlock*					AddChild(const tstring& sName);

	void						BeginFrame();

	void						BlockStarted();
	void						BlockEnded();

	tstring						GetName() { return m_sName; };
	double						GetTime() { return m_flTime; };

public:
	CPerfBlock*					m_pParent;

	tstring						m_sName;
	double						m_flTime;

	double						m_flTimeBlockStarted;

	tmap<tstring, CPerfBlock*>	m_apPerfBlocks;
};

class CProfiler
{
public:
	static void					BeginFrame();

	static void					PushScope(CProfileScope* pScope);
	static void					PopScope(CProfileScope* pScope);

	static void					Render();

	static bool					IsProfiling() { return s_bProfiling; };

protected:
	static void					PopAllScopes();

	static void					Render(CPerfBlock* pBlock, float& flLeft, float& flTop);

protected:
	static CPerfBlock*			s_pBottomBlock;
	static CPerfBlock*			s_pTopBlock;

	static bool					s_bProfiling;
};

#endif
