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

#include "shell.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <time.h>

#include <strutils.h>
#include <tinker_platform.h>
#include <mtrand.h>
#include <tinker/cvar.h>

CShell* CShell::s_pShell = NULL;

CShell::CShell(int argc, char** argv)
{
	s_pShell = this;

	srand((unsigned int)time(NULL));
	mtsrand((size_t)time(NULL));

	for (int i = 0; i < argc; i++)
		m_apszCommandLine.push_back(argv[i]);

	m_sBinaryName = argv[0];

	for (int i = 1; i < argc; i++)
	{
		if (m_apszCommandLine[i][0] == '+')
			CCommand::Run(&m_apszCommandLine[i][1]);
	}
}

CShell::~CShell()
{
}

double CShell::GetTime()
{
	TAssert(false);
	return 0;
}

bool CShell::HasCommandLineSwitch(const char* pszSwitch)
{
	for (size_t i = 0; i < m_apszCommandLine.size(); i++)
	{
		if (strcmp(m_apszCommandLine[i], pszSwitch) == 0)
			return true;
	}

	return false;
}

const char* CShell::GetCommandLineSwitchValue(const char* pszSwitch)
{
	// -1 to prevent buffer overrun
	for (size_t i = 0; i < m_apszCommandLine.size()-1; i++)
	{
		if (strcmp(m_apszCommandLine[i], pszSwitch) == 0)
			return m_apszCommandLine[i+1];
	}

	return NULL;
}

void CShell::PrintConsole(const tstring& sText)
{
	puts(sText.c_str());
}

void CShell::PrintError(const tstring& sText)
{
	puts((tstring("ERROR: ") + sText).c_str());
}

void CreateApplicationWithErrorHandling(CreateApplicationCallback pfnCallback, int argc, char** argv)
{
#ifdef _WIN32
#ifndef _DEBUG
	__try
	{
#endif
#endif

		// Put in a different function to avoid warnings and errors associated with object deconstructors and try/catch blocks.
		pfnCallback(argc, argv);

#if defined(_WIN32) && !defined(_DEBUG)
	}
	__except (CreateMinidump(GetExceptionInformation(), "Tinker"), EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif
}
