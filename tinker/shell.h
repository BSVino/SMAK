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

#pragma once

#include <tvector.h>
#include <common.h>
#include <vector.h>
#include <color.h>
#include <configfile.h>

class CShell
{
public:
								CShell(int argc, char** argv);
	virtual 					~CShell();

public:
	virtual double				GetTime();

	bool						HasCommandLineSwitch(const char* pszSwitch);
	const char*					GetCommandLineSwitchValue(const char* pszSwitch);

	const tstring&				GetBinaryName() { return m_sBinaryName; }

	virtual void				PrintConsole(const tstring& sText);
	virtual void				PrintError(const tstring& sText);

	static inline CShell*		Get() { return s_pShell; };

protected:
	tstring						m_sBinaryName;
	tvector<const char*>		m_apszCommandLine;

	static CShell*				s_pShell;
};

inline CShell* Shell()
{
	return CShell::Get();
}

// Tinker messages and errors
#define TMsg Shell()->PrintConsole
#define TError Shell()->PrintError

typedef void (*CreateApplicationCallback)(int argc, char** argv);
void CreateApplicationWithErrorHandling(CreateApplicationCallback pfnCallback, int argc, char** argv);
