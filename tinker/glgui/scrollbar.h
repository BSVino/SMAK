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

#include "basecontrol.h"

namespace glgui
{
	class CScrollBar : public CBaseControl
	{
		DECLARE_CLASS(CScrollBar, CBaseControl);

	public:
						CScrollBar(bool bHorizontal);	// If it's not horizontal it's vertical

	public:
		virtual void	Layout();
		virtual void	Paint(float x, float y, float w, float h);

		virtual void	SetVisible(bool bVisible);

		virtual void	Think();

		virtual bool	MousePressed(int code, int mx, int my);
		virtual bool	MouseReleased(int code, int mx, int my);
		virtual bool	IsCursorListener() {return true;};

		virtual void	CursorOut();

		virtual void	DoneMovingHandle();

		virtual float	HandleX();
		virtual float	HandleY();

		virtual float	GetHandlePosition() const { return m_flHandlePosition; }

	protected:
		bool			m_bHorizontal;

		float			m_flHandlePosition;
		float			m_flHandlePositionGoal;
		bool			m_bMovingHandle;

		float			m_flHandleSize;
		float			m_flHandleGrabOffset;
	};
}
