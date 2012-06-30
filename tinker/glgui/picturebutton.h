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

#ifndef TINKER_PICTUREBUTTON_H
#define TINKER_PICTUREBUTTON_H

#include "button.h"

#include <textures/materialhandle.h>

namespace glgui
{
	class CPictureButton : public CButton
	{
	public:
						CPictureButton(const tstring& sText, const CMaterialHandle& hMaterial = CMaterialHandle(), bool bToggle = false);

	public:
		virtual void	Paint() { CButton::Paint(); };
		virtual void	Paint(float x, float y, float w, float h);

		virtual void	SetTexture(const CMaterialHandle& hMaterial);
		virtual void	SetSheetTexture(const CMaterialHandle& hMaterial, int sx, int sy, int sw, int sh, int tw, int th);
		virtual void	SetSheetTexture(const CMaterialHandle& hMaterial, const Rect& rArea, int tw, int th);
		virtual void	ShowBackground(bool bShow) { m_bShowBackground = bShow; };

	protected:
		CMaterialHandle	m_hMaterial;
		bool			m_bShowBackground;

		bool			m_bSheet;
		int				m_iSX;
		int				m_iSY;
		int				m_iSW;
		int				m_iSH;
		int				m_iTW;
		int				m_iTH;
	};
};

#endif
