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

#ifndef TINKER_TEXTFIELD_H
#define TINKER_TEXTFIELD_H

#include "basecontrol.h"

namespace glgui
{
	class CTextField : public CBaseControl
	{
		DECLARE_CLASS(CTextField, CBaseControl);

		friend class CRootPanel;

	public:
						CTextField();

	public:
		virtual void	Paint() { float x = 0, y = 0; GetAbsPos(x, y); Paint(x, y); };
		virtual void	Paint(float x, float y) { Paint(x, y, m_flW, m_flH); };
		virtual void	Paint(float x, float y, float w, float h);
		virtual void	PostPaint();
		virtual void	DrawLine(const tchar* pszText, unsigned iLength, float x, float y, float w, float h);

		virtual bool	IsCursorListener() {return true;};

		virtual bool	TakesFocus();
		virtual bool	SetFocus(bool bFocus);

		virtual bool	MousePressed(int iButton, int mx, int my);
		virtual void	CursorMoved(int x, int y);
		virtual bool	CharPressed(int iKey);
		virtual bool	KeyPressed(int iKey, bool bCtrlDown = false);

		virtual void	SetContentsChangedListener(IEventListener* pListener, IEventListener::Callback pfnCallback, const tstring& sArgs="");
		virtual void	UpdateContentsChangedListener();

		virtual void	FindRenderOffset();

		virtual bool	IsEnabled() {return m_bEnabled;};
		virtual void	SetEnabled(bool bEnabled) {m_bEnabled = bEnabled;};

		virtual void	SetText(const tstring& pszText);
		virtual void	AppendText(const tchar* pszText);
		virtual tstring	GetText();

		void			ClearAutoCompleteCommands();
		void			SetAutoCompleteCommands(const tvector<tstring>& asCommands, bool bSlashInsensitive=false);
		void			SetAutoCompleteFiles(const tstring& sBaseDirectory=".", const tvector<tstring>& asExtensions = tvector<tstring>(), const tvector<tstring>& asExtensionsExclude = tvector<tstring>());

		virtual void	SetCursorPosition(size_t iPosition);

		virtual void	SetFontFaceSize(int iSize);
		virtual int		GetFontFaceSize() { return m_iFontFaceSize; };

		virtual float	GetTextWidth();
		virtual float	GetTextHeight();
		virtual void	EnsureTextFits();

		virtual Color	GetFGColor();
		virtual void	SetFGColor(Color FGColor);
		virtual void	SetAlpha(int a);

	protected:
		bool			m_bEnabled;
		tstring			m_sText;
		Color			m_FGColor;

		double			m_flBlinkTime;

		int				m_iFontFaceSize;

		size_t			m_iCursor;
		size_t			m_iSelection;

		float			m_flRenderOffset;

		IEventListener::Callback	m_pfnContentsChangedCallback;
		IEventListener*				m_pContentsChangedListener;
		tstring						m_sContentsChangedArgs;

		tvector<tstring>			m_asAutoCompleteCommands;
		int							m_iAutoComplete;
		bool						m_bSlashInsensitive;
	};
};

#endif
