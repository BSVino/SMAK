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

#ifndef TINKER_LABEL_H
#define TINKER_LABEL_H

#include "basecontrol.h"

#include <tmap.h>

class FTFont;

namespace glgui
{
	class CLabel : public CBaseControl
	{
		friend class CRootPanel;

	public:
		class CLineSection
		{
		public:
			CLineSection()
				: m_bColor(false)
			{
			}

		public:
			tstring		m_sText;
			tstring		m_sLink;
			FRect		m_rArea;
			tstring		m_sFont;
			size_t		m_iFontSize;
			class ::FTFont* m_pFont;
			bool		m_bColor;
			Color		m_clrText;
		};

		class CLine
		{
		public:
			tvector<CLineSection>	m_aSections;
			float		m_flLineHeight;
			float		m_flLineWidth;
		};

	public:
						CLabel();
						CLabel(const tstring& sText, const tstring& sFont="sans-serif", size_t iSize=13);
						CLabel(float x, float y, float w, float h, const tstring& sText, const tstring& sFont="sans-serif", size_t iSize=13);
						~CLabel();

	public:
		typedef enum
		{
			TA_TOPLEFT		= 0,
			TA_TOPCENTER	= 1,
			TA_LEFTCENTER	= 2,
			TA_MIDDLECENTER	= 3,
			TA_RIGHTCENTER	= 4,
			TA_BOTTOMCENTER	= 5,
			TA_BOTTOMLEFT	= 6,
		} TextAlign;

		virtual void	Paint() { float x = 0, y = 0; GetAbsPos(x, y); Paint(x, y, m_flW, m_flH); };
		virtual void	Paint(float x, float y) { Paint(x, y, m_flW, m_flH); };
		virtual void	Paint(float x, float y, float w, float h);
		virtual void	DrawSection(const CLine& l, const CLineSection& s, float x, float y, float w, float h);
		virtual void	Layout() {};
		virtual void	Think() {};

		virtual void	GetAlignmentOffset(float flLineWidth, float flLineHeight, const tstring& sFont, size_t iFontSize, float flAreaWidth, float flAreaHeight, float& x, float& y) const;

		virtual void	SetWidth(float w);
		virtual void	SetHeight(float h);
		virtual void	SetSize(float w, float h);

		void			GetRealMousePosition(float& x, float& y);
		bool			MouseIsInside(const CLine& oLine, const CLineSection& oSection);

		virtual bool	IsCursorListener() { return true; };
		virtual bool	MousePressed(int code, int mx, int my);
		virtual bool	MouseReleased(int code, int mx, int my);

		virtual bool	IsEnabled() {return m_bEnabled;};
		virtual void	SetEnabled(bool bEnabled) {m_bEnabled = bEnabled;};

		virtual TextAlign	GetAlign() const { return m_eAlign; };
		virtual void	SetAlign(TextAlign eAlign) { m_eAlign = eAlign; };

		virtual bool	GetWrap() { return m_bWrap; };
		virtual void	SetWrap(bool bWrap) { m_bWrap = bWrap; m_bNeedsCompute = true; };

		virtual void	SetPrintChars(int iPrintChars) { m_iPrintChars = iPrintChars; }

		virtual void	SetText(const tstring& sText);
		virtual void	AppendText(const tstring& sText);
		virtual tstring	GetText() const;

		virtual void	SetFont(const tstring& sFontName, int iSize=13);
		virtual int		GetFontFaceSize() const { return m_iFontFaceSize; };

		virtual float	GetTextWidth() const;
		virtual float	GetTextHeight();
		virtual void	EnsureTextFits();

		virtual void	ComputeLines(float w = -1, float h = -1);
		virtual void	PushSection(const CLineSection& oSection, const tstring& sText);

		virtual size_t				GetNumLines() const { return m_aLines.size(); }
		virtual const CLine&		GetLine(size_t iLine) const { return m_aLines[iLine]; }
		virtual size_t				GetNumSections(size_t iLine) const { return m_aLines[iLine].m_aSections.size(); }
		virtual const CLineSection&	GetSection(size_t iLine, size_t iSection) const { return m_aLines[iLine].m_aSections[iSection]; }

		virtual Color	GetTextColor() const;
		virtual void	SetTextColor(const Color& clrText);
		virtual void	SetAlpha(int a);
		virtual void	SetAlpha(float a);

		virtual void	SetScissor(bool bScissor); // 61

		virtual void	Set3D(bool b3D) { m_b3D = b3D; }
		virtual bool	Is3D() const { return m_b3D; }
		virtual void	Set3DMousePosition(const Vector& vecMouse) { m_vec3DMouse = vecMouse; }

		virtual void	SetLinkClickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);
		virtual IEventListener::Callback	GetLinkClickedListenerCallback() { return m_pfnLinkClickCallback; };
		virtual IEventListener*				GetLinkClickedListener() { return m_pLinkClickListener; };

		virtual void	SetSectionHoverListener(IEventListener* pListener, IEventListener::Callback pfnCallback);
		virtual IEventListener::Callback	GetSectionHoverListenerCallback() { return m_pfnSectionHoverCallback; };
		virtual IEventListener*				GetSectionHoverListener() { return m_pSectionHoverListener; };

		static class ::FTFont*	GetFont(const tstring& sName, size_t iSize);
		static void		AddFont(const tstring& sName, const tstring& sFile);
		static void		AddFontSize(const tstring& sName, size_t iSize);

		static float	GetTextWidth(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize);
		static float	GetFontHeight(const tstring& sFontName, int iFontFaceSize);
		static void		PaintText(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize, float x, float y, const Color& clrText = Color(255, 255, 255), const FRect& rStencil = FRect(-1, -1, -1, -1));
		static void		PaintText(const tstring& sText, unsigned iLength, class ::FTFont* pFont, float x, float y, const Color& clrText = Color(255, 255, 255), const FRect& rStencil = FRect(-1, -1, -1, -1));
		static void		PaintText3D(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize, Vector vecPosition, const Color& clrText = Color(255, 255, 255));

	protected:
		bool			m_bEnabled;
		bool			m_bWrap;
		bool			m_b3D;
		Vector			m_vec3DMouse;
		tstring			m_sText;
		Color			m_clrText;
		bool			m_bScissor;

		TextAlign		m_eAlign;

		tvector<CLine>	m_aLines;
		float			m_flTotalHeight;
		bool			m_bNeedsCompute;

		int				m_iPrintChars;
		int				m_iCharsDrawn;

		tstring			m_sFontName;
		int				m_iFontFaceSize;
		class ::FTFont*	m_pFont;

		static tmap<tstring, tmap<size_t, class ::FTFont*> >	s_apFonts;
		static tmap<tstring, tstring>							s_apFontNames;

		IEventListener::Callback m_pfnLinkClickCallback;
		IEventListener*	m_pLinkClickListener;

		IEventListener::Callback m_pfnSectionHoverCallback;
		IEventListener*	m_pSectionHoverListener;
	};
};

#endif
