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

#include <glgui/panel.h>
#include <glgui/button.h>
#include <glgui/selector.h>

#define HEADER_HEIGHT 16

namespace glgui
{
	class CCloseButton : public glgui::CButton
	{
	public:
								CCloseButton() : glgui::CButton(0, 0, 10, 10, "") {};

	public:
		virtual void			Paint() { glgui::CButton::Paint(); };
		virtual void			Paint(float x, float y, float w, float h);
	};

	class CMinimizeButton : public glgui::CButton
	{
	public:
								CMinimizeButton() : glgui::CButton(0, 0, 10, 10, "") {};

	public:
		virtual void			Paint() { glgui::CButton::Paint(); };
		virtual void			Paint(float x, float y, float w, float h);
	};

	class CMovablePanel : public glgui::CPanel, public glgui::IEventListener
	{
		DECLARE_CLASS(CMovablePanel, glgui::CPanel);

	public:
								CMovablePanel(const tstring& sName);
								~CMovablePanel();

	public:
		virtual void			Layout();

		virtual void			Think();

		virtual void			PaintBackground(float x, float y, float w, float h);
		virtual void			Paint(float x, float y, float w, float h);

		virtual bool			MousePressed(int iButton, int mx, int my);
		virtual bool			MouseReleased(int iButton, int mx, int my);

		virtual void			HasCloseButton(bool bHasClose) { m_bHasCloseButton = bHasClose; };
		virtual void			Minimize();

		virtual bool			IsChildVisible(CBaseControl* pChild);

		virtual void			SetClearBackground(bool bClearBackground);
		virtual void			SetHeaderColor(const Color& clrHeader) { m_clrHeader = clrHeader; }

		EVENT_CALLBACK(CMovablePanel, MinimizeWindow);
		EVENT_CALLBACK(CMovablePanel, CloseWindow);

		void					Close();

		bool					ShouldControlOffset(const CBaseControl* pControl) const;

	protected:
		int						m_iMouseStartX;
		int						m_iMouseStartY;
		float					m_flStartX;
		float					m_flStartY;
		bool					m_bMoving;

		bool					m_bHasCloseButton;
		bool					m_bMinimized;
		float					m_flNonMinimizedHeight;
		float					m_flMinimizeTransitionLerp;

		bool					m_bClearBackground;

		CControl<CLabel>		m_hName;

		CControl<CCloseButton>	m_hCloseButton;
		CControl<CMinimizeButton>	m_hMinimizeButton;

		Color					m_clrHeader;
	};
}