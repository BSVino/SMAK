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

#include "colorpicker.h"

#include <maths.h>

#include <renderer/renderingcontext.h>
#include <tinker/application.h>

#include "rootpanel.h"
#include "label.h"
#include "textfield.h"

using namespace glgui;

CColorPickerButton::CColorPickerButton()
	: CButton(0, 0, 30, 30, "", true)
{
	m_pColorPicker = (new CColorPicker(this))->shared_from_this();
	m_pColorPicker->SetVisible(false);

	SetClickedListener(this, Open);
	SetUnclickedListener(this, Close);
}

void CColorPickerButton::Paint(float x, float y, float w, float h)
{
	BaseClass::Paint(x, y, w, h);

	CBaseControl::PaintRect(x + 3, y + 3, w - 6, h - 6, GetColorPicker()->GetColorVector());
}

void CColorPickerButton::OpenCallback(const tstring& sArgs)
{
	RootPanel()->AddControl(m_pColorPicker, true);

	float flPanelWidth = m_pColorPicker->GetWidth();
	float flButtonWidth = GetWidth();
	float x, y, w, h;
	GetAbsDimensions(x, y, w, h);
	m_pColorPicker->SetPos(x + flButtonWidth/2 - flPanelWidth/2, y+h+3);

	m_pColorPicker->GetAbsDimensions(x, y, w, h);

	if (y + h > RootPanel()->GetHeight() - 5)
		m_pColorPicker->SetTop(RootPanel()->GetHeight() - m_pColorPicker->GetHeight() - 5);

	m_pColorPicker->Layout();
	m_pColorPicker->SetVisible(true);
}

void CColorPickerButton::CloseCallback(const tstring& sArgs)
{
	m_pColorPicker.DowncastStatic<CColorPicker>()->Close();

	SetState(false, false);
}

void CColorPickerButton::SetChangedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	m_pColorPicker.DowncastStatic<CColorPicker>()->SetChangedListener(pListener, pfnCallback);
}

void CColorPickerButton::SetColor(const Vector& vecColor)
{
	m_pColorPicker.DowncastStatic<CColorPicker>()->SetColor(vecColor);
}

Vector CColorPickerButton::GetColorVector()
{
	return m_pColorPicker.DowncastStatic<CColorPicker>()->GetColorVector();
}

CControl<CColorPicker> CColorPicker::s_hOpenPicker;

CColorPicker::CColorPicker(CColorPickerButton* pButton)
	: CPanel(0, 0, 200, 250)
{
	m_pButton = pButton;

	m_hRGB = AddControl(new CLabel("RGB:"));

	m_hValue = AddControl(new CTextField());
	m_hValue->SetContentsChangedListener(this, ValueChanged);

	m_hOkay = AddControl(new CButton(0, 0, 100, 100, "Okay"));
	m_hOkay->SetClickedListener(m_pButton, CColorPickerButton::Close);

	m_clrRGB = Color(0.5f, 0.5f, 0.5f);
	m_clrRGB.GetHSL(m_flHue, m_flSaturation, m_flLightness);
}

void CColorPicker::Layout()
{
	BaseClass::Layout();

	m_hRGB->SetPos(5, 180);
	m_hRGB->SetWidth(1);
	m_hRGB->EnsureTextFits();

	m_hValue->SetPos(m_hRGB->GetRight(), 180);
	m_hValue->SetRight(GetWidth() - 5);

	m_hOkay->SetSize(40, 20);
	m_hOkay->SetPos(GetWidth()/2 - 20, GetHeight() - 30);
}

void CColorPicker::Paint(float x, float y, float w, float h)
{
	CRootPanel::PaintRect(x, y, w, h, g_clrBox);

	BaseClass::Paint(x, y, w, h);

	::CRenderingContext r(nullptr, true);

	r.UseProgram("colorwheel");

	r.SetUniform("vecDimensions", Vector4D(CircleX(), CircleY(), CircleW(), CircleH()));

	r.SetUniform("clrSelected", m_clrRGB);
	r.SetUniform("hslSelected", Vector(m_flHue, m_flSaturation, m_flLightness));

	int mx, my;
	RootPanel()->GetFullscreenMousePos(mx, my);
	r.SetUniform("vecMouse", Vector((float)mx, (float)my, 0));

	r.BeginRenderVertexArray(s_iQuad);
	r.SetPositionBuffer((size_t)0u, 24);
	r.SetTexCoordBuffer(12, 24);
	r.SetCustomIntBuffer("iVertex", 1, 20, 24);
	r.EndRenderVertexArray(6);

	r.UseProgram("colorvalue");

	r.SetUniform("vecDimensions", Vector4D(BarX(), BarY(), BarW(), BarH()));

	r.SetUniform("clrSelected", m_clrRGB);
	r.SetUniform("hslSelected", Vector(m_flHue, m_flSaturation, m_flLightness));

	r.SetUniform("vecMouse", Vector((float)mx, (float)my, 0));

	r.BeginRenderVertexArray(s_iQuad);
	r.SetPositionBuffer((size_t)0u, 24);
	r.SetTexCoordBuffer(12, 24);
	r.SetCustomIntBuffer("iVertex", 1, 20, 24);
	r.EndRenderVertexArray(6);
}

void CColorPicker::SetVisible(bool bVisible)
{
	if (bVisible && !m_bVisible)
	{
		if (s_hOpenPicker.Get())
			s_hOpenPicker->Close();

		s_hOpenPicker = this;
	}
	else if (m_bVisible && !bVisible)
	{
		TAssertNoMsg(!s_hOpenPicker || s_hOpenPicker == this);
		if (s_hOpenPicker == this)
			s_hOpenPicker = nullptr;
	}

	BaseClass::SetVisible(bVisible);
}

bool CColorPicker::MousePressed(int code, int mx, int my)
{
	if (code == TINKER_KEY_MOUSE_LEFT)
	{
		if (Update(mx, my))
			return true;
	}

	return BaseClass::MousePressed(code, mx, my);
}

void CColorPicker::CursorMoved(int mx, int my)
{
	if (Application()->IsMouseLeftDown())
		Update(mx, my);
}

bool CColorPicker::Update(int x, int y)
{
	if (x > BarX() && y > BarY() && x < BarX() + BarW() && y < BarY() + BarH())
	{
		m_flLightness = RemapVal((float)y, BarY(), BarY() + BarH(), 1, 0);
		m_clrRGB.SetHSL(m_flHue, m_flSaturation, m_flLightness);

		m_hValue->SetText(sprintf("%i %i %i", m_clrRGB.r(), m_clrRGB.g(), m_clrRGB.b()));

		if (m_pfnChangedCallback)
			m_pfnChangedCallback(m_pChangedListener, sprintf("%f %f %f", GetColorVector().x, GetColorVector().y, GetColorVector().z));

		return true;
	}

	float flRadius = CircleW()/2;
	float flRadiusSqr = flRadius*flRadius;

	Vector vecCenter(CircleX() + flRadius, CircleY() + flRadius, 0);

	Vector vecFromCenter = Vector((float)x, (float)y, 0) - vecCenter;
	float flDistanceSqr = vecFromCenter.LengthSqr();
	if (flDistanceSqr > flRadiusSqr)
		return false;

	float flTan = atan2(vecFromCenter.y, -vecFromCenter.x);

	m_flHue = RemapVal(flTan, -M_PI, M_PI, 0, 360);
	m_flSaturation = sqrt(flDistanceSqr)/flRadius;

	m_clrRGB.SetHSL(m_flHue, m_flSaturation, m_flLightness);

	m_hValue->SetText(sprintf("%i %i %i", m_clrRGB.r(), m_clrRGB.g(), m_clrRGB.b()));

	if (m_pfnChangedCallback)
		m_pfnChangedCallback(m_pChangedListener, sprintf("%f %f %f", GetColorVector().x, GetColorVector().y, GetColorVector().z));

	return true;
}

void CColorPicker::ValueChangedCallback(const tstring& sArgs)
{
	tvector<tstring> asArgs;
	strtok(m_hValue->GetText(), asArgs);

	if (asArgs.size() < 3)
		return;

	m_clrRGB.SetRed(stoi(asArgs[0]));
	m_clrRGB.SetGreen(stoi(asArgs[1]));
	m_clrRGB.SetBlue(stoi(asArgs[2]));

	m_clrRGB.GetHSL(m_flHue, m_flSaturation, m_flLightness);

	if (m_pfnChangedCallback)
		m_pfnChangedCallback(m_pChangedListener, sprintf("%f %f %f", GetColorVector().x, GetColorVector().y, GetColorVector().z));
}

void CColorPicker::Close()
{
	SetVisible(false);
	RootPanel()->RemoveControl(this);

	m_pButton->SetState(false, false);
}

void CColorPicker::SetChangedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	m_pfnChangedCallback = pfnCallback;
	m_pChangedListener = pListener;
}

void CColorPicker::SetColor(const Vector& vecColor)
{
	m_clrRGB = vecColor;

	m_clrRGB.GetHSL(m_flHue, m_flSaturation, m_flLightness);

	m_hValue->SetText(sprintf("%i %i %i", m_clrRGB.r(), m_clrRGB.g(), m_clrRGB.b()));
}

Vector CColorPicker::GetColorVector()
{
	return m_clrRGB;
}
