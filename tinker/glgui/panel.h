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

#ifndef TINKER_PANEL_H
#define TINKER_PANEL_H

#include "basecontrol.h"

#include <tinker_memory.h>

namespace glgui
{
	class CScrollBar;

	// A panel is a container for other controls. It is for organization
	// purposes only; it does not currently keep its children from drawing
	// outside of it.
	class CPanel : public CBaseControl
	{
		DECLARE_CLASS(CPanel, CBaseControl);

#ifdef _DEBUG
		// Just so CBaseControl can get at CPanel's textures for the purpose of debug paint methods.
		friend class CBaseControl;
#endif

	public:
								CPanel();
								CPanel(float x, float y, float w, float h);
		virtual					~CPanel();

	public:
		virtual void			Paint();
		virtual void			Paint(float x, float y);
		virtual void			Paint(float x, float y, float w, float h);
		virtual void			PostPaint();
		virtual void			Layout();
		virtual void			Think();
		virtual void			UpdateScene();

		virtual void			SetDefaultMargin(float flMargin) { m_flMargin = flMargin; }
		virtual float			GetDefaultMargin() { return m_flMargin; }

		virtual bool			KeyPressed(int code, bool bCtrlDown = false);
		virtual bool			KeyReleased(int code);
		virtual bool			CharPressed(int iKey);
		virtual bool			MousePressed(int code, int mx, int my);
		virtual bool			MouseReleased(int code, int mx, int my);
		virtual bool			MouseDoubleClicked(int code, int mx, int my);
		virtual bool			IsCursorListener() {return true;};
		virtual void			CursorMoved(int mx, int my);
		virtual void			CursorOut();

		virtual CControlHandle	GetHasCursor();

		void					NextTabStop();

		virtual CControlHandle	AddControl(CBaseControl* pControl, bool bToTail = false);
		virtual CControlHandle	AddControl(CControlResource pControl, bool bToTail = false);
		virtual void			RemoveControl(CBaseControl* pControl);
		virtual tvector<CControlResource>&	GetControls() { return m_apControls; };
		virtual void			MoveToTop(CBaseControl* pControl);

		virtual void			SetHighlighted(bool bHighlight) { m_bHighlight = bHighlight; };
		virtual bool			IsHighlighted() { return m_bHighlight; };

		void					SetScissoring(bool b) { m_bScissoring = b; };
		bool					IsScissoring() const;

		FRect					GetControlBounds() const { return m_rControlBounds; };
		FRect					GetControlOffset() const { return m_rControlOffset; };

		void					SetVerticalScrollBarEnabled(bool b);
		void					SetHorizontalScrollBarEnabled(bool b);

		CControl<CScrollBar>	GetVerticalScrollBar() const;
		CControl<CScrollBar>	GetHorizontalScrollBar() const;

		virtual bool			ShouldControlOffset(const CBaseControl* pControl) const;

	protected:
		tvector<CControlResource>	m_apControls;

		float					m_flMargin;

		// If two controls in the same panel are never layered, a single
		// pointer should suffice. Otherwise a list must be created.
		CControlHandle			m_hHasCursor;

		bool					m_bHighlight;
		bool					m_bScissoring;

		FRect					m_rControlBounds;	// Bounding box for all child controls but not children of children. Only valid after Layout()
		FRect					m_rControlOffset;	// w/h offset for children as determined by scrollbar

		CControl<CScrollBar>	m_hVerticalScrollBar;
		CControl<CScrollBar>	m_hHorizontalScrollBar;
	};
};

#endif
