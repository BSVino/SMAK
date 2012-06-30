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

#ifndef TINKER_SELECTOR_H
#define TINKER_SELECTOR_H

#include "rootpanel.h"
#include "panel.h"

namespace glgui
{
	template <typename T>
	class CScrollSelection
	{
	public:
		CScrollSelection(T oParam, const tstring& sLabel)
		{
			m_oParam = oParam;
			m_sLabel = sLabel;
		}

		T									m_oParam;
		tstring						m_sLabel;
	};

	template <typename T>
	class CScrollSelector : public CPanel
	{
		DECLARE_CLASS(CScrollSelector, CPanel);

	public:
		CScrollSelector(const tstring& sFont="sans-serif", size_t iSize=13)
			: CPanel(0, 0, 100, 16)
		{
			m_flHandlePositionGoal = 0;
			m_flHandlePosition = 0;
			m_bMovingHandle = false;

			m_iSelection = 0;

			m_pfnSelectedCallback = NULL;
			m_pSelectedListener = NULL;

			m_sFont = sFont;
			m_iFontSize = iSize;

			m_hOption = AddControl(new CLabel(0, 0, 100, 100, ""));
			m_hOption->SetWrap(false);
			m_hOption->SetFont(m_sFont, m_iFontSize);
		}

		virtual void Layout()
		{
			m_hOption->SetSize(1, 1);

			// Make sure there's some text to be fit to.
			if (m_hOption->GetText() == "")
				m_hOption->SetText("-");

			m_hOption->EnsureTextFits();
			m_hOption->SetPos(GetWidth()/2 - m_hOption->GetWidth()/2, GetHeight()-15);

			CPanel::Layout();
		}

		virtual void Think()
		{
			if (m_bMovingHandle)
			{
				int mx, my;
				CRootPanel::GetFullscreenMousePos(mx, my);

				float x, y, w, h;
				GetAbsDimensions(x, y, w, h);

				m_flHandlePositionGoal = RemapValClamped((float)mx, (float)x, (float)(x + w), 0.0f, 1.0f);
			}
			else
			{
				if (m_aSelections.size() < 2)
					m_flHandlePositionGoal = ((float)GetWidth()*(float)m_iSelection)/GetWidth();
				else
					m_flHandlePositionGoal = ((float)GetWidth()/((float)m_aSelections.size()-1)*(float)m_iSelection)/GetWidth();
			}

			m_flHandlePosition = Approach(m_flHandlePositionGoal, m_flHandlePosition, (float)CRootPanel::Get()->GetFrameTime()*10);

			int iSelection = SelectionByHandle();
			m_hOption->SetText(m_aSelections[iSelection].m_sLabel.c_str());

			if (iSelection != m_iSelection)
			{
				m_iSelection = iSelection;
				if (m_pSelectedListener)
					m_pfnSelectedCallback(m_pSelectedListener, "");
			}
		}

#define HANDLE_SIZE 12

		virtual void Paint(float x, float y, float w, float h)
		{
			//CRootPanel::PaintRect(x, y, w, h, g_clrBoxHi);

			float flLeft = x+HANDLE_SIZE/2;
			float flWidth = w-HANDLE_SIZE;

			CRootPanel::PaintRect(flLeft, y+h/2, flWidth, 1, Color(200, 200, 200, 255));

			if (m_aSelections.size() < 2)
			{
				CRootPanel::PaintRect(flLeft, y+h/2-5, 1, 10, Color(200, 200, 200, 255));
				CRootPanel::PaintRect(flLeft + flWidth, y+h/2-5, 1, 10, Color(200, 200, 200, 255));
			}
			else
			{
				for (size_t i = 0; i < m_aSelections.size(); i++)
					CRootPanel::PaintRect(flLeft + flWidth*(int)i/((int)m_aSelections.size()-1), y+h/2-5, 1, 10, Color(200, 200, 200, 255));
			}

			CRootPanel::PaintRect(HandleX()+2, HandleY()+2, HANDLE_SIZE-4, HANDLE_SIZE-4, g_clrBoxHi, 2);

			CPanel::Paint(x, y, w, h);
		}

		virtual bool MousePressed(int code, int mx, int my)
		{
			float x, y, w, h;
			GetAbsDimensions(x, y, w, h);

			float hx, hy;
			hx = HandleX();
			hy = HandleY();

			if (mx >= hx && mx < hx + HANDLE_SIZE && my >= hy && my < hy + HANDLE_SIZE)
				m_bMovingHandle = true;
			else
			{
				m_flHandlePositionGoal = RemapValClamped((float)mx, (float)x + HANDLE_SIZE/2, (float)(x + w - HANDLE_SIZE/2), 0.0f, 1.0f);
				m_iSelection = SelectionByHandle();

				if (m_pSelectedListener)
					m_pfnSelectedCallback(m_pSelectedListener, "");
			}

			return true;
		}

		virtual bool MouseReleased(int code, int mx, int my)
		{
			float x, y, w, h;
			GetAbsDimensions(x, y, w, h);

			if (m_bMovingHandle)
			{
				DoneMovingHandle();
				return true;
			}

			return CPanel::MouseReleased(code, mx, my);
		}

		virtual void CursorOut()
		{
			if (m_bMovingHandle)
			{
				int mx, my;
				CRootPanel::GetFullscreenMousePos(mx, my);

				float x, y, w, h;
				GetAbsDimensions(x, y, w, h);

				// If the mouse went out of the left or right side, make sure we're all the way to that side.
				m_flHandlePositionGoal = RemapValClamped((float)mx, (float)x, (float)(x + w), 0.0f, 1.0f);

				DoneMovingHandle();
			}
		}

		virtual void DoneMovingHandle()
		{
			m_bMovingHandle = false;

			m_iSelection = SelectionByHandle();

			if (m_pSelectedListener)
				m_pfnSelectedCallback(m_pSelectedListener, "");
		}

		virtual void AddSelection(const CScrollSelection<T>& oSelection)
		{
			m_aSelections.push_back(oSelection);
		}

		virtual size_t GetNumSelections()
		{
			return m_aSelections.size();
		}
		
		virtual void RemoveAllSelections()
		{
			m_aSelections.clear();
		}

		virtual void SetSelection(size_t i)
		{
			if (i >= m_aSelections.size())
				i = m_aSelections.size() - 1;

			if (i == m_iSelection)
				return;

			m_iSelection = i;
			m_flHandlePositionGoal = m_flHandlePosition = ((float)GetWidth()/((float)m_aSelections.size()-1)*(float)m_iSelection)/GetWidth();

			if (m_pSelectedListener)
				m_pfnSelectedCallback(m_pSelectedListener, "");
		}

		virtual T GetSelectionValue()
		{
			return m_aSelections[m_iSelection].m_oParam;
		}

		virtual size_t FindClosestSelectionValue(float flValue)
		{
			size_t iClosest;
			T flClosestValue;
			for (size_t i = 0; i < m_aSelections.size(); i++)
			{
				if (i == 0)
				{
					flClosestValue = m_aSelections[0].m_oParam;
					iClosest = 0;
					continue;
				}

				if (fabs((float)(m_aSelections[i].m_oParam - flValue)) < fabs((float)(flClosestValue - flValue)))
				{
					flClosestValue = m_aSelections[i].m_oParam;
					iClosest = i;
				}
			}

			return iClosest;
		}

		virtual int SelectionByHandle()
		{
			int iSelection = (int)(m_flHandlePositionGoal*m_aSelections.size());

			if (iSelection < 0)
				return 0;

			if (iSelection >= (int)m_aSelections.size())
				return (int)m_aSelections.size()-1;

			return iSelection;
		}

		virtual float HandleX()
		{
			float x, y, w, h;
			GetAbsDimensions(x, y, w, h);

			float flLeft = x+HANDLE_SIZE/2;
			float flWidth = w-HANDLE_SIZE;
			return flLeft + flWidth*m_flHandlePosition - HANDLE_SIZE/2;
		}

		virtual float HandleY()
		{
			float x, y, w, h;
			GetAbsDimensions(x, y, w, h);

			return y+h/2-HANDLE_SIZE/2;
		}

		void SetSelectedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
		{
			m_pfnSelectedCallback = pfnCallback;
			m_pSelectedListener = pListener;
		}

	protected:
		tstring								m_sFont;
		size_t								m_iFontSize;

		tvector<CScrollSelection<T> >		m_aSelections;

		CControl<CLabel>					m_hOption;

		size_t								m_iSelection;

		float								m_flHandlePosition;
		float								m_flHandlePositionGoal;

		bool								m_bMovingHandle;

		IEventListener::Callback			m_pfnSelectedCallback;
		IEventListener*						m_pSelectedListener;
	};
};

#endif
