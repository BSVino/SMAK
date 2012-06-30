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

#ifndef LW_TINKER_CVAR
#define LW_TINKER_CVAR

#include <tmap.h>

#include <common.h>
#include <tstring.h>
#include <tvector.h>

typedef void (*CommandCallback)(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand);

class CCommand
{
public:
						CCommand(tstring sName, CommandCallback pfnCallback);

public:
	static void			Run(tstring sCommand);

	tstring				GetName() { return m_sName; };

	virtual void		MakeMePolymorphic() {};	// Can delete if another virtual function is added

	static tvector<tstring> GetCommandsBeginningWith(tstring sFragment);

protected:
	tstring				m_sName;
	CommandCallback		m_pfnCallback;

	static void			RegisterCommand(CCommand* pCommand);

protected:
	static tmap<tstring, CCommand*>& GetCommands()
	{
		static tmap<tstring, CCommand*> aCommands;
		return aCommands;
	}
};

class CVar : public CCommand
{
	DECLARE_CLASS(CVar, CCommand);

public:
						CVar(tstring sName, tstring sValue);

public:
	void				SetValue(tstring sValue);
	void				SetValue(int iValue);
	void				SetValue(float flValue);

	tstring				GetValue() { return m_sValue; };
	bool				GetBool();
	int					GetInt();
	float				GetFloat();

	void				CalculateValues();

	static CVar*		FindCVar(tstring sName);

	static void			SetCVar(tstring sName, tstring sValue);
	static void			SetCVar(tstring sName, int iValue);
	static void			SetCVar(tstring sName, float flValue);

	static tstring		GetCVarValue(tstring sName);
	static bool			GetCVarBool(tstring sName);
	static int			GetCVarInt(tstring sName);
	static float		GetCVarFloat(tstring sName);

protected:
	tstring				m_sValue;

	bool				m_bDirtyValues;
	bool				m_bValue;
	int					m_iValue;
	float				m_flValue;
};

#endif
