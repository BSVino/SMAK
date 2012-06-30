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

#include "picturebutton.h"

#include <GL3/gl3w.h>

using namespace glgui;

CPictureButton::CPictureButton(const tstring& sText, const CMaterialHandle& hMaterial, bool bToggle)
	: CButton(0, 0, 32, 32, sText, bToggle)
{
	m_hMaterial = hMaterial;
	m_bShowBackground = true;
	m_bSheet = false;
}

void CPictureButton::Paint(float x, float y, float w, float h)
{
	if (!IsVisible())
		return;

	if (m_bShowBackground)
		PaintButton(x, y, w, h);

	float flHighlight = 1;
	if (m_bEnabled)
		flHighlight = RemapVal(m_flHighlight, 0, 1, 0.8f, 1);

	if (m_bSheet)
	{
		PaintSheet(m_hMaterial, x, y, w, h, m_iSX, m_iSY, m_iSW, m_iSH, m_iTW, m_iTH, Color(255,255,255,(unsigned char)(GetAlpha()*flHighlight)));
	}
	else if (m_hMaterial.IsValid())
	{
		glEnablei(GL_BLEND, 0);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		PaintTexture(m_hMaterial, x, y, w, h, Color(255,255,255,(unsigned char)(GetAlpha()*flHighlight)));
	}
	else
	{
		// Now paint the text which appears on the button.
		CLabel::Paint(x, y, w, h);
		return;
	}

	CBaseControl::Paint(x, y, w, h);
}

void CPictureButton::SetTexture(const CMaterialHandle& hMaterial)
{
	m_bSheet = false;
	m_hMaterial = hMaterial;
}

void CPictureButton::SetSheetTexture(const CMaterialHandle& hMaterial, int sx, int sy, int sw, int sh, int tw, int th)
{
	m_bSheet = true;
	m_hMaterial = hMaterial;
	m_iSX = sx;
	m_iSY = sy;
	m_iSW = sw;
	m_iSH = sh;
	m_iTW = tw;
	m_iTH = th;
}

void CPictureButton::SetSheetTexture(const CMaterialHandle& hMaterial, const Rect& rArea, int tw, int th)
{
	m_bSheet = true;
	m_hMaterial = hMaterial;
	m_iSX = rArea.x;
	m_iSY = rArea.y;
	m_iSW = rArea.w;
	m_iSH = rArea.h;
	m_iTW = tw;
	m_iTH = th;
}
