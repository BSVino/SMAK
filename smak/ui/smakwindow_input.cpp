/*
SMAK - The Super Model Army Knife
Copyright (C) 2012, Lunar Workshop, Inc

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "smakwindow.h"

#include <tinker/keys.h>
#include <glgui/glgui.h>
#include <glgui/rootpanel.h>

#include "crunch/crunch.h"
#include "smakwindow_ui.h"
#include "smak_renderer.h"
#include "aogenerator.h"
#include "normalgenerator.h"

using namespace glgui;

void CSMAKWindow::MouseMotion(int x, int y)
{
	if (m_bRenderUV)
	{
		if (m_bCameraDollying)
		{
			m_flCameraUVZoom += (float)(m_iMouseStartY - y)/100;

			if (m_flCameraUVZoom < 0.01f)
				m_flCameraUVZoom = 0.01f;

			m_iMouseStartY = y;
		}

		if (m_bCameraPanning)
		{
			m_flCameraUVX += (float)(m_iMouseStartX - x)/500;
			m_flCameraUVY += (float)(y - m_iMouseStartY)/500;

			m_iMouseStartX = x;
			m_iMouseStartY = y;
		}

		if (m_bLightRotating)
		{
			GetSMAKRenderer()->MoveUVLight((float)(x - m_iMouseStartX)/100, (float)(m_iMouseStartY - y)/100);

			m_iMouseStartX = x;
			m_iMouseStartY = y;
		}
	}
	else
	{
		if (m_bCameraRotating)
		{
			m_flCameraPitch -= ((float)y - (float)m_iMouseStartY)/2.0f;
			m_flCameraYaw += ((float)x - (float)m_iMouseStartX)/2.0f;

			while (m_flCameraPitch > 89)
				m_flCameraPitch = 89;
			while (m_flCameraPitch < -89)
				m_flCameraPitch = -89;

			m_iMouseStartX = x;
			m_iMouseStartY = y;
		}

		if (m_bCameraDollying)
		{
			m_flCameraDistance += ((float)y - (float)m_iMouseStartY)*(m_flCameraDistance/100);

			if (m_flCameraDistance < 1)
				m_flCameraDistance = 1;

			m_iMouseStartY = y;
		}

		if (m_bLightRotating)
		{
			m_flLightYaw += (m_iMouseStartX - x);
			m_flLightPitch -= (m_iMouseStartY - y);

			while (m_flLightYaw >= 180)
				m_flLightYaw -= 360;
			while (m_flLightYaw < -180)
				m_flLightYaw += 360;
			while (m_flLightPitch > 89)
				m_flLightPitch = 89;
			while (m_flLightPitch < -89)
				m_flLightPitch = -89;

			m_iMouseStartX = x;
			m_iMouseStartY = y;
		}
	}

	glgui::CRootPanel::Get()->CursorMoved(x, y);
}

bool CSMAKWindow::MouseInput(int iButton, tinker_mouse_state_t iState)
{
	if (BaseClass::MouseInput(iButton, iState))
		return true;

	int x, y;
	GetMousePosition(x, y);

	if (m_bRenderUV)
	{
		if (IsCtrlDown() && iButton == TINKER_KEY_MOUSE_LEFT)
		{
			if (iState == TINKER_MOUSE_PRESSED)
			{
				m_bLightRotating = 1;
				m_iMouseStartX = x;
				m_iMouseStartY = y;
				return true;
			}
			else if (iState == TINKER_MOUSE_RELEASED)
			{
				m_bLightRotating = 0;
				return true;
			}
		}
		else if (iButton == TINKER_KEY_MOUSE_LEFT)
		{
			if (iState == TINKER_MOUSE_PRESSED)
			{
				m_bCameraPanning = 1;
				m_iMouseStartX = x;
				m_iMouseStartY = y;
				return true;
			}
			else if (iState == TINKER_MOUSE_RELEASED)
			{
				m_bCameraPanning = 0;
				return true;
			}
		}
		else
		{
			if (iState == TINKER_MOUSE_PRESSED)
			{
				m_bCameraDollying = 1;
				m_iMouseStartX = x;
				m_iMouseStartY = y;
				return true;
			}
			else if (iState == TINKER_MOUSE_RELEASED)
			{
				m_bCameraDollying = 0;
				return true;
			}
		}
	}
	else
	{
		if (IsCtrlDown() && iButton == TINKER_KEY_MOUSE_LEFT)
		{
			if (iState == TINKER_MOUSE_PRESSED)
			{
				m_bLightRotating = 1;
				m_iMouseStartX = x;
				m_iMouseStartY = y;
				return true;
			}
			else if (iState == TINKER_MOUSE_RELEASED)
			{
				m_bLightRotating = 0;
				return true;
			}
		}
		else if (iButton == TINKER_KEY_MOUSE_LEFT)
		{
			if (iState == TINKER_MOUSE_PRESSED)
			{
				m_bCameraRotating = 1;
				m_iMouseStartX = x;
				m_iMouseStartY = y;
				return true;
			}
			else if (iState == TINKER_MOUSE_RELEASED)
			{
				m_bCameraRotating = 0;
				return true;
			}
		}
		else if (iButton == TINKER_KEY_MOUSE_RIGHT)
		{
			if (iState == TINKER_MOUSE_PRESSED)
			{
				m_bCameraDollying = 1;
				m_iMouseStartX = x;
				m_iMouseStartY = y;
				return true;
			}
			else if (iState == TINKER_MOUSE_RELEASED)
			{
				m_bCameraDollying = 0;
				return true;
			}
		}
	}

	return false;
}

void CSMAKWindow::MouseWheel(int x, int y)
{
	static int iOldState = 0;

	if (m_bRenderUV)
	{
		if (x > iOldState)
		{
			m_flCameraUVZoom += 0.01f;
		}
		else
		{
			m_flCameraUVZoom -= 0.01f;

			if (m_flCameraUVZoom < 0.01f)
				m_flCameraUVZoom = 0.01f;
		}
	}
	else
	{
		if (x > iOldState)
		{
			m_flCameraDistance += 10.0f;
		}
		else
		{
			m_flCameraDistance -= 10.0f;

			if (m_flCameraDistance < 1.0f)
				m_flCameraDistance = 1.0f;
		}
	}

	iOldState = x;
}

bool CSMAKWindow::DoCharPress(int c)
{
	if (c == 'a')
	{
		CAOPanel::Open(&m_Scene);
		return true;
	}

	if (c == 'n')
	{
		CNormalPanel::Open(&m_Scene);
		return true;
	}

	if (c == 'r' && IsCtrlDown())
	{
		ReloadFromFile();
		return true;
	}

	if (glgui::CRootPanel::Get()->CharPressed(c))
		return true;

	return false;
}

bool CSMAKWindow::DoKeyPress(int c)
{
	if (glgui::CRootPanel::Get()->KeyPressed(c, IsCtrlDown()))
		return true;

	if (c == 27)
		exit(0);

	if (c == TINKER_KEY_F4 && IsAltDown())
		exit(0);

	if (c == TINKER_KEY_F5)
	{
		ReloadFromFile();
		return true;
	}

	return false;
}
