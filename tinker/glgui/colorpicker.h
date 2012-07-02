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

#include "panel.h"
#include "button.h"

namespace glgui
{
	class CColorPicker : public CPanel, public IEventListener
	{
		DECLARE_CLASS(CColorPicker, CPanel);

	public:
							CColorPicker(class CColorPickerButton* pButton);

	public:
		virtual void		Layout();
		virtual void		Paint(float x, float y, float w, float h);

		virtual void		SetVisible(bool bVisible);

		virtual bool		MousePressed(int code, int mx, int my);
		virtual void		CursorMoved(int mx, int my);
		virtual bool		Update(int x, int y);

		EVENT_CALLBACK(CColorPicker, ValueChanged);

		void				Close();

		void				SetChangedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		void				SetColor(const Vector& vecColor);
		Vector				GetColorVector();

		float				CircleX() const { return GetLeft() + 10; }
		float				CircleY() const { return GetTop() + 10; }
		float				CircleW() const { return GetWidth() - 50; }
		float				CircleH() const { return GetWidth() - 50; }

		float				BarX() const { return GetLeft() + GetWidth() - 35; }
		float				BarY() const { return GetTop() + 10; }
		float				BarW() const { return 30; }
		float				BarH() const { return GetWidth() - 50; }

	protected:
		class CColorPickerButton*	m_pButton;

		CControl<CLabel>		m_hRGB;
		CControl<CTextField>	m_hValue;
		CControl<CButton>		m_hOkay;

		IEventListener::Callback	m_pfnChangedCallback;
		IEventListener*				m_pChangedListener;

		Color				m_clrRGB;
		float				m_flHue;
		float				m_flSaturation;
		float				m_flLightness;

		static CControl<CColorPicker>	s_hOpenPicker;
	};

	class CColorPickerButton : public CButton, public IEventListener
	{
		DECLARE_CLASS(CColorPickerButton, CButton);

	public:
							CColorPickerButton();

	public:
		void				Paint(float x, float y, float w, float h);

		EVENT_CALLBACK(CColorPickerButton, Open);
		EVENT_CALLBACK(CColorPickerButton, Close);

		CColorPicker*		GetColorPicker() { return m_pColorPicker.DowncastStatic<CColorPicker>(); }

		void				SetChangedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		void				SetColor(const Vector& vecColor);
		Vector				GetColorVector();

	protected:
		glgui::CControlResource	m_pColorPicker;
	};
}
