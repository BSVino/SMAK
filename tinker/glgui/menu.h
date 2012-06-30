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

#ifndef TINKER_MENU_H
#define TINKER_MENU_H

#include "panel.h"
#include "button.h"

namespace glgui
{
#define MENU_SPACING 10
#define MENU_HEIGHT 22

	class CMenuBar : public CPanel
	{
	public:
									CMenuBar();

		void						Layout();

		void						SetActive(class CMenu* pMenu);
	};

	class CMenu : public CButton, public IEventListener
	{
		DECLARE_CLASS(CMenu, CButton);

	public:
									CMenu(const tstring& sTitle, bool bSubmenu = false);
		virtual						~CMenu();

	public:
		virtual void				Think();
		virtual void				Layout();
		virtual void				Paint(float x, float y, float w, float h);
		virtual void				PostPaint();

		virtual bool				IsCursorListener() { return true; };
		virtual void				CursorIn();
		virtual void				CursorOut();

		virtual void				SetMenuListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		EVENT_CALLBACK(CMenu, Open);
		EVENT_CALLBACK(CMenu, Close);
		EVENT_CALLBACK(CMenu, Clicked);

		void						OpenMenu();
		void						CloseMenu();

		virtual void				AddSubmenu(const tstring& sTitle, IEventListener* pListener = NULL, IEventListener::Callback pfnCallback = NULL);
		virtual void				ClearSubmenus();

		virtual size_t				GetSelectedMenu();

	protected:
		class CSubmenuPanel : public CPanel
		{
			friend class CMenu;

			DECLARE_CLASS(CSubmenuPanel, CPanel);
		public:
									CSubmenuPanel(CControl<CMenu> hMenu);

		public:
			void					Think();

			void					Paint(float x, float y, float w, float h);
			void					PostPaint();

			virtual bool			IsVisible();

			void					SetFakeHeight(float flFakeHeight) { m_flFakeHeight = flFakeHeight; };

		protected:
			float					m_flFakeHeight;

			tvector<float>			m_aflControlHighlightGoal;
			tvector<float>			m_aflControlHighlight;

			CControl<CMenu>			m_hMenu;
		};

		bool						m_bSubmenu;

		float						m_flHighlightGoal;
		float						m_flHighlight;

		float						m_flMenuHighlightGoal;
		float						m_flMenuHighlight;
		float						m_flMenuHeightGoal;
		float						m_flMenuHeight;
		float						m_flMenuSelectionHighlightGoal;
		float						m_flMenuSelectionHighlight;
		FRect						m_MenuSelectionGoal;
		FRect						m_MenuSelection;

		IEventListener::Callback	m_pfnMenuCallback;
		IEventListener*				m_pMenuListener;

		CControl<CSubmenuPanel>		m_hMenu;

		tvector<CControl<CMenu>>	m_ahEntries;
	};
};

#endif
