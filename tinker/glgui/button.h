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

#ifndef TINKER_BUTTON_H
#define TINKER_BUTTON_H

#include "label.h"

namespace glgui
{
	class CButton : public CLabel
	{
		friend class CRootPanel;
		friend class CSlidingPanel;

	public:
						CButton(const tstring& sText, bool bToggle = false, const tstring& sFont="sans-serif", size_t iSize=13);
						CButton(float x, float y, float w, float h, const tstring& sText, bool bToggle = false, const tstring& sFont="sans-serif", size_t iSize=13);

	public:
		virtual void	Think();

		virtual void	Paint() { CLabel::Paint(); };
		virtual void	Paint(float x, float y, float w, float h);
		virtual void	PaintButton(float x, float y, float w, float h);

		virtual bool	MousePressed(int code, int mx, int my);
		virtual bool	MouseReleased(int code, int mx, int my);
		virtual bool	IsCursorListener() {return true;};
		virtual void	CursorIn();
		virtual void	CursorOut();

		virtual bool	IsToggleButton() {return m_bToggle;};
		virtual void	SetToggleButton(bool bToggle);
		virtual void	SetToggleState(bool bState);
		virtual bool	GetToggleState() {return m_bToggleOn;};

		virtual bool	Push();
		virtual bool	Pop(bool bRegister = true, bool bRevert = false);
		virtual void	SetState(bool bDown, bool bRegister = true);
		virtual bool	GetState() {return m_bDown;};

		virtual void	SetClickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback, const tstring& sArgs="");
		// Toggle buttons only
		virtual void	SetUnclickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback, const tstring& sArgs="");
		virtual IEventListener::Callback	GetClickedListenerCallback() { return m_pfnClickCallback; };
		virtual IEventListener*				GetClickedListener() { return m_pClickListener; };

		virtual bool	IsHighlighted() {return m_flHighlight > 0;};

		virtual void	SetButtonColor(Color clrButton) { m_clrButton = clrButton; };
		virtual void	SetDownColor(Color clrDown) { m_clrDown = clrDown; };

	protected:
		bool			m_bToggle;
		bool			m_bToggleOn;
		bool			m_bDown;
		float			m_flHighlightGoal;
		float			m_flHighlight;

		// Need multiple event listeners? Too bad! Make a list.
		IEventListener::Callback m_pfnClickCallback;
		IEventListener*	m_pClickListener;
		tstring			m_sClickArgs;

		IEventListener::Callback m_pfnUnclickCallback;
		IEventListener*	m_pUnclickListener;
		tstring			m_sUnclickArgs;

		Color			m_clrButton;
		Color			m_clrDown;
	};
};

#endif
