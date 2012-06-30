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

#ifndef SMAK_MODELWINDOW_H
#define SMAK_MODELWINDOW_H

#include <common.h>

#include <modelconverter/convmesh.h>
#include <glgui/glgui.h>
#include <glgui/basecontrol.h>
#include <worklistener.h>
#include <tinker/application.h>
#include <textures/materialhandle.h>

class CSMAKWindow : public CApplication, glgui::IEventListener, IWorkListener
{
	DECLARE_CLASS(CSMAKWindow, CApplication);

public:
	typedef struct
	{
		Vector vecStart;
		Vector vecEnd;
		Color clrLine;
	} DebugLine;

public:
							CSMAKWindow(int argc, char** argv);
							~CSMAKWindow();

public:
	virtual tstring			WindowTitle() { return "SMAK - Super Model Army Knife"; }
	virtual tstring			AppDirectory() { return "SMAK"; }

	void					OpenWindow();

	void					InitUI();

	void					Run();	// Doesn't return

	void					DestroyAll();
	void					ReadFile(const tchar* pszFile);
	void					ReadFileIntoScene(const tchar* pszFile);
	void					ReloadFromFile();

	void					LoadIntoGL();
	void					LoadModelsIntoGL();
	void					LoadMaterialsIntoGL();

	void					SaveFile(const tchar* pszFile);

	void					Layout();

	virtual void			Render();

	virtual void			WindowResize(int x, int y);
	virtual void			MouseMotion(int x, int y);
	virtual bool			MouseInput(int iButton, tinker_mouse_state_t iState);
	virtual void			MouseWheel(int x, int y);
	virtual bool			DoCharPress(int c);
	virtual bool			DoKeyPress(int c);

	EVENT_CALLBACK(CSMAKWindow, OpenDialog);
	EVENT_CALLBACK(CSMAKWindow, OpenFile);
	EVENT_CALLBACK(CSMAKWindow, OpenIntoDialog);
	EVENT_CALLBACK(CSMAKWindow, OpenIntoFile);
	EVENT_CALLBACK(CSMAKWindow, Reload);
	EVENT_CALLBACK(CSMAKWindow, SaveDialog);
	EVENT_CALLBACK(CSMAKWindow, SaveFile);
	EVENT_CALLBACK(CSMAKWindow, Close);
	EVENT_CALLBACK(CSMAKWindow, Exit);
	EVENT_CALLBACK(CSMAKWindow, Render3D);
	EVENT_CALLBACK(CSMAKWindow, RenderUV);
	EVENT_CALLBACK(CSMAKWindow, SceneTree);
	EVENT_CALLBACK(CSMAKWindow, Wireframe);
	EVENT_CALLBACK(CSMAKWindow, Flat);
	EVENT_CALLBACK(CSMAKWindow, Smooth);
	EVENT_CALLBACK(CSMAKWindow, UVWireframe);
	EVENT_CALLBACK(CSMAKWindow, Light);
	EVENT_CALLBACK(CSMAKWindow, Texture);
	EVENT_CALLBACK(CSMAKWindow, Combo);
	EVENT_CALLBACK(CSMAKWindow, Normal);
	EVENT_CALLBACK(CSMAKWindow, AO);
	EVENT_CALLBACK(CSMAKWindow, ColorAO);
	EVENT_CALLBACK(CSMAKWindow, LightToggle);
	EVENT_CALLBACK(CSMAKWindow, TextureToggle);
	EVENT_CALLBACK(CSMAKWindow, NormalToggle);
	EVENT_CALLBACK(CSMAKWindow, AOToggle);
	EVENT_CALLBACK(CSMAKWindow, ColorAOToggle);
	EVENT_CALLBACK(CSMAKWindow, GenerateCombo);
	EVENT_CALLBACK(CSMAKWindow, GenerateAO);
	EVENT_CALLBACK(CSMAKWindow, GenerateColorAO);
	EVENT_CALLBACK(CSMAKWindow, GenerateNormal);
	EVENT_CALLBACK(CSMAKWindow, Help);
	EVENT_CALLBACK(CSMAKWindow, Register);
	EVENT_CALLBACK(CSMAKWindow, About);

	size_t					GetNextObjectId();

	void					OpenHelpPanel();
	void					OpenRegisterPanel();
	void					OpenAboutPanel();

	// UI
	bool					GetRenderMode() { return m_bRenderUV; };
	void					SetRenderMode(bool bUV);
	void					SetDisplayWireframe(bool bWire);
	void					SetDisplayUVWireframe(bool bWire);
	void					SetDisplayLight(bool bLight);
	void					SetDisplayTexture(bool bTexture);
	void					SetDisplayNormal(bool bNormal);
	void					SetDisplayAO(bool bAO);
	void					SetDisplayColorAO(bool bAO);

	void					SaveNormal(size_t iMaterial, const tstring& sFilename);

	int						GetWindowWidth() { return (int)m_iWindowWidth; };
	int						GetWindowHeight() { return (int)m_iWindowHeight; };

	float					GetCameraDistance() { return m_flCameraDistance; };
	float					GetCameraYaw() { return m_flCameraYaw; };
	float					GetCameraPitch() { return m_flCameraPitch; };
	float					GetCameraUVZoom() { return m_flCameraUVZoom; };
	float					GetCameraUVX() { return m_flCameraUVX; };
	float					GetCameraUVY() { return m_flCameraUVY; };
	float					GetLightYaw() { return m_flLightYaw; };
	float					GetLightPitch() { return m_flLightPitch; };

	static CSMAKWindow*		Get() { return s_pSMAKWindow; };

	CConversionScene*			GetScene() { return &m_Scene; };
	tvector<CMaterialHandle>&	GetMaterials() { return m_ahMaterials; };

	class CSMAKRenderer*	GetSMAKRenderer();
	class CRenderer*		CreateRenderer();

	void					ClearDebugLines();
	void					AddDebugLine(Vector vecStart, Vector vecEnd, Color clrLine = Color(150, 150, 150));
	tvector<DebugLine>&		GetDebugLines() { return m_aDebugLines; }

	bool					IsRenderingLight() { return m_bDisplayLight; }
	bool					IsRenderingUV() { return m_bRenderUV; }
	bool					IsRenderingWireframe() { return m_bDisplayWireframe; }
	bool					IsRenderingUVWireframe() { return m_bDisplayUV; }
	bool					IsRenderingTexture() { return m_bDisplayTexture; }
	bool					IsRenderingNormal() { return m_bDisplayNormal; }
	bool					IsRenderingAO() { return m_bDisplayAO; }
	bool					IsRenderingColorAO() { return m_bDisplayColorAO; }

	void					BeginProgress();
	void					SetAction(const tstring& sAction, size_t iTotalProgress);
	void					WorkProgress(size_t iProgress, bool bForceDraw = false);
	void					EndProgress();

protected:
	CConversionScene		m_Scene;
	bool					m_bLoadingFile;
	tstring					m_sFileLoaded;

	tvector<size_t>			m_aiObjects;
	size_t					m_iObjectsCreated;

	tvector<CMaterialHandle>	m_ahMaterials;

	float					m_flCameraDistance;

	bool					m_bCameraRotating;
	bool					m_bCameraDollying;
	bool					m_bCameraPanning;
	bool					m_bLightRotating;

	int						m_iMouseStartX;
	int						m_iMouseStartY;

	float					m_flCameraYaw;
	float					m_flCameraPitch;

	float					m_flLightYaw;
	float					m_flLightPitch;

	float					m_flCameraUVX;
	float					m_flCameraUVY;
	float					m_flCameraUVZoom;

	tvector<DebugLine>		m_aDebugLines;

	// Options
	bool					m_bRenderUV;
	bool					m_bDisplayWireframe;
	bool					m_bDisplayUV;
	bool					m_bDisplayLight;
	bool					m_bDisplayTexture;
	bool					m_bDisplayNormal;
	bool					m_bDisplayAO;
	bool					m_bDisplayColorAO;

	// Controls
	glgui::CControl<glgui::CButton>			m_hRender3D;
	glgui::CControl<glgui::CButton>			m_hRenderUV;

	glgui::CControl<glgui::CButton>			m_hWireframe;
	glgui::CControl<glgui::CButton>			m_hFlat;
	glgui::CControl<glgui::CButton>			m_hSmooth;
	glgui::CControl<glgui::CButton>			m_hUVWireframe;
	glgui::CControl<glgui::CButton>			m_hLight;
	glgui::CControl<glgui::CButton>			m_hTexture;
	glgui::CControl<glgui::CButton>			m_hNormal;
	glgui::CControl<glgui::CButton>			m_hAO;
	glgui::CControl<glgui::CButton>			m_hColorAO;

	static CSMAKWindow*		s_pSMAKWindow;
};

inline CSMAKWindow* SMAKWindow()
{
	return CSMAKWindow::Get();
}

#endif
