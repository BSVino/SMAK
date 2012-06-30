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

#include "smakwindow_ui.h"

#include <maths.h>
#include <strutils.h>
#include <tinker_platform.h>
#include <tinker/keys.h>
#include <glgui/menu.h>
#include <glgui/rootpanel.h>
#include <glgui/picturebutton.h>
#include <glgui/checkbox.h>
#include <glgui/tree.h>
#include <glgui/textfield.h>
#include <glgui/filedialog.h>
#include <textures/materiallibrary.h>
#include <modelconverter/modelconverter.h>

#include "smakwindow.h"
#include "scenetree.h"
#include "../smak_version.h"
#include "picker.h"
#include "smak_renderer.h"
#include "progressbar.h"
#include "buttonpanel.h"
#include "combogenerator.h"
#include "aogenerator.h"
#include "normalgenerator.h"

using namespace glgui;

void CSMAKWindow::InitUI()
{
	CMenu* pFile = CRootPanel::Get()->AddMenu("File");
	CMenu* pView = CRootPanel::Get()->AddMenu("View");
	CMenu* pTools = CRootPanel::Get()->AddMenu("Tools");
	CMenu* pHelp = CRootPanel::Get()->AddMenu("Help");

	pFile->AddSubmenu("Open...", this, OpenDialog);
	pFile->AddSubmenu("Open Into...", this, OpenIntoDialog);
	pFile->AddSubmenu("Reload", this, Reload);
	pFile->AddSubmenu("Save As...", this, SaveDialog);
	pFile->AddSubmenu("Close", this, Close);
	pFile->AddSubmenu("Exit", this, Exit);

	pView->AddSubmenu("3D view", this, Render3D);
	pView->AddSubmenu("UV view", this, RenderUV);
	pView->AddSubmenu("View wireframe", this, Wireframe);
	pView->AddSubmenu("Toggle light", this, LightToggle);
	pView->AddSubmenu("Toggle texture", this, TextureToggle);
	pView->AddSubmenu("Toggle normal map", this, NormalToggle);
	pView->AddSubmenu("Toggle AO map", this, AOToggle);

	pTools->AddSubmenu("Generate all maps", this, GenerateCombo);
	pTools->AddSubmenu("Generate ambient occlusion map", this, GenerateAO);
	pTools->AddSubmenu("Generate normal from texture", this, GenerateNormal);

	pHelp->AddSubmenu("Help", this, Help);
	pHelp->AddSubmenu("About SMAK", this, About);

	glgui::CControl<CBaseControl> pTopButtons = new CButtonPanel(BA_TOP);
	CControl<CButtonPanel> hTopButtons(pTopButtons);

	m_hRender3D = hTopButtons->AddButton(new CPictureButton("3D", GetSMAKRenderer()->GetSmoothTexture(), true), "Render 3D View", false, this, Render3D);
	m_hRenderUV = hTopButtons->AddButton(new CPictureButton("UV", GetSMAKRenderer()->GetUVTexture(), true), "Render UV View", false, this, RenderUV);

	CRootPanel::Get()->AddControl(pTopButtons);

	glgui::CControl<CBaseControl> pBottomButtons = new CButtonPanel(BA_BOTTOM);
	CControl<CButtonPanel> hBottomButtons(pBottomButtons);

	m_hWireframe = hBottomButtons->AddButton(new CPictureButton("Wire", GetSMAKRenderer()->GetWireframeTexture(), true), "Toggle Wireframe", true, this, Wireframe);
	m_hUVWireframe = hBottomButtons->AddButton(new CPictureButton("Wire", GetSMAKRenderer()->GetUVTexture(), true), "Toggle UVs", true, this, UVWireframe);
	m_hLight = hBottomButtons->AddButton(new CPictureButton("Lght", GetSMAKRenderer()->GetLightTexture(), true), "Toggle Light", false, this, Light);
	m_hTexture = hBottomButtons->AddButton(new CPictureButton("Tex", GetSMAKRenderer()->GetTextureTexture(), true), "Toggle Texture", false, this, Texture);
	m_hNormal = hBottomButtons->AddButton(new CPictureButton("Nrml", GetSMAKRenderer()->GetNormalTexture(), true), "Toggle Normal Map", false, this, Normal);
	m_hAO = hBottomButtons->AddButton(new CPictureButton("AO", GetSMAKRenderer()->GetAOTexture(), true), "Toggle AO Map", false, this, AO);

	CRootPanel::Get()->AddControl(pBottomButtons);

	CSceneTreePanel::Open(&m_Scene);

	CRootPanel::Get()->Layout();
}

void CSMAKWindow::Layout()
{
	CRootPanel::Get()->Layout();
}

void CSMAKWindow::OpenDialogCallback(const tstring& sArgs)
{
	if (m_bLoadingFile)
		return;

	CFileDialog::ShowOpenDialog("", implode(";", CModelConverter::GetReadFormats()), this, OpenFile);
}

void CSMAKWindow::OpenFileCallback(const tstring& sArgs)
{
	ReadFile(sArgs.c_str());
}

void CSMAKWindow::OpenIntoDialogCallback(const tstring& sArgs)
{
	if (m_bLoadingFile)
		return;

	CFileDialog::ShowOpenDialog("", implode(";", CModelConverter::GetReadFormats()), this, OpenIntoFile);
}

void CSMAKWindow::OpenIntoFileCallback(const tstring& sArgs)
{
	ReadFileIntoScene(sArgs.c_str());
}

void CSMAKWindow::ReloadCallback(const tstring& sArgs)
{
	if (m_bLoadingFile)
		return;

	ReloadFromFile();
}

void CSMAKWindow::SaveDialogCallback(const tstring& sArgs)
{
	CFileDialog::ShowSaveDialog("", ".obj;.sia;.dae", this, SaveFile);
}

void CSMAKWindow::SaveFileCallback(const tstring& sArgs)
{
	SaveFile(sArgs.c_str());
}

void CSMAKWindow::CloseCallback(const tstring& sArgs)
{
	if (m_bLoadingFile)
		return;

	DestroyAll();
}

void CSMAKWindow::ExitCallback(const tstring& sArgs)
{
	exit(0);
}

void CSMAKWindow::Render3DCallback(const tstring& sArgs)
{
	SetRenderMode(false);
}

void CSMAKWindow::RenderUVCallback(const tstring& sArgs)
{
	SetRenderMode(true);
}

void CSMAKWindow::SceneTreeCallback(const tstring& sArgs)
{
	CSceneTreePanel::Open(&m_Scene);
}

void CSMAKWindow::WireframeCallback(const tstring& sArgs)
{
	SetDisplayWireframe(m_hWireframe->GetState());
}

void CSMAKWindow::UVWireframeCallback(const tstring& sArgs)
{
	m_bDisplayUV = m_hUVWireframe->GetState();
}

void CSMAKWindow::LightCallback(const tstring& sArgs)
{
	SetDisplayLight(m_hLight->GetState());
}

void CSMAKWindow::TextureCallback(const tstring& sArgs)
{
	SetDisplayTexture(m_hTexture->GetState());
}

void CSMAKWindow::NormalCallback(const tstring& sArgs)
{
	SetDisplayNormal(m_hNormal->GetState());
}

void CSMAKWindow::AOCallback(const tstring& sArgs)
{
	SetDisplayAO(m_hAO->GetState());
}

void CSMAKWindow::ColorAOCallback(const tstring& sArgs)
{
#if 0
	if (CAOPanel::Get(true) && CAOPanel::Get(true)->IsGenerating() && !CAOPanel::Get(true)->DoneGenerating())
	{
		m_hColorAO->SetState(true, false);
		return;
	}

	if (!CAOPanel::Get(true) || !CAOPanel::Get(true)->DoneGenerating())
	{
		CAOPanel::Open(true, &m_Scene);
		m_hColorAO->SetState(false, false);
		return;
	}

	SetDisplayColorAO(m_hColorAO->GetState());
#endif
}

void CSMAKWindow::LightToggleCallback(const tstring& sArgs)
{
	SetDisplayLight(!m_bDisplayLight);
}

void CSMAKWindow::TextureToggleCallback(const tstring& sArgs)
{
	SetDisplayTexture(!m_bDisplayTexture);
}

void CSMAKWindow::NormalToggleCallback(const tstring& sArgs)
{
	SetDisplayNormal(!m_bDisplayNormal);
}

void CSMAKWindow::AOToggleCallback(const tstring& sArgs)
{
	SetDisplayAO(!m_bDisplayAO);
}

void CSMAKWindow::ColorAOToggleCallback(const tstring& sArgs)
{
	SetDisplayColorAO(!m_bDisplayColorAO);
}

void CSMAKWindow::GenerateComboCallback(const tstring& sArgs)
{
	CComboGeneratorPanel::Open(&m_Scene);
}

void CSMAKWindow::GenerateAOCallback(const tstring& sArgs)
{
	CAOPanel::Open(&m_Scene);
}

void CSMAKWindow::GenerateColorAOCallback(const tstring& sArgs)
{
#if 0
	CAOPanel::Open(true, &m_Scene);
#endif
}

void CSMAKWindow::GenerateNormalCallback(const tstring& sArgs)
{
	CNormalPanel::Open(&m_Scene);
}

void CSMAKWindow::HelpCallback(const tstring& sArgs)
{
	OpenHelpPanel();
}

void CSMAKWindow::AboutCallback(const tstring& sArgs)
{
	OpenAboutPanel();
}

void CSMAKWindow::OpenHelpPanel()
{
	CHelpPanel::Open();
}

void CSMAKWindow::OpenAboutPanel()
{
	CAboutPanel::Open();
}

void CSMAKWindow::BeginProgress()
{
	CProgressBar::Get()->SetVisible(true);
}

void CSMAKWindow::SetAction(const tstring& sAction, size_t iTotalProgress)
{
	CProgressBar::Get()->SetTotalProgress(iTotalProgress);
	CProgressBar::Get()->SetAction(sAction);
	WorkProgress(0, true);
}

void CSMAKWindow::WorkProgress(size_t iProgress, bool bForceDraw)
{
	static double flLastTime = 0;

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && GetTime() - flLastTime < 0.3f)
		return;

	CProgressBar::Get()->SetProgress(iProgress);

	CSMAKWindow::Get()->Render();

	SwapBuffers();

	flLastTime = GetTime();
}

void CSMAKWindow::EndProgress()
{
	CProgressBar::Get()->SetVisible(false);
}

glgui::CControl<CHelpPanel> CHelpPanel::s_hHelpPanel;

CHelpPanel::CHelpPanel()
	: CMovablePanel("Help")
{
	m_hInfo = AddControl(new CLabel(0, 0, 100, 100, ""));

	Layout();
}

void CHelpPanel::Layout()
{
	if (GetParent())
	{
		float px, py, pw, ph;
		GetParent()->GetAbsDimensions(px, py, pw, ph);

		SetSize(600, 200);
		SetPos(pw/2 - GetWidth()/2, ph/2 - GetHeight()/2);
	}

	m_hInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_hInfo->SetSize(GetWidth(), GetHeight());
	m_hInfo->SetPos(0, 30);

	m_hInfo->SetText("CONTROLS:\n");
	m_hInfo->AppendText("Left Mouse Button - Move the camera\n");
	m_hInfo->AppendText("Right Mouse Button - Zoom in and out\n");
	m_hInfo->AppendText("Ctrl-LMB - Rotate the light\n");
	m_hInfo->AppendText(" \n");
	m_hInfo->AppendText("For in-depth help information please visit the website, http://www.getsmak.net/\n");

	CMovablePanel::Layout();
}

void CHelpPanel::Paint(float x, float y, float w, float h)
{
	CMovablePanel::Paint(x, y, w, h);
}

bool CHelpPanel::MousePressed(int iButton, int mx, int my)
{
	if (CMovablePanel::MousePressed(iButton, mx, my))
		return true;

	Close();

	return false;
}

void CHelpPanel::Open()
{
	if (!s_hHelpPanel)
		s_hHelpPanel = new CHelpPanel();

	s_hHelpPanel->SetVisible(true);
	s_hHelpPanel->Layout();
}


glgui::CControl<CAboutPanel> CAboutPanel::s_hAboutPanel;

CAboutPanel::CAboutPanel()
	: CMovablePanel("About SMAK")
{
	m_hInfo = AddControl(new CLabel(0, 0, 100, 100, ""));

	Layout();
}

void CAboutPanel::Layout()
{
	if (GetParent())
	{
		float px, py, pw, ph;
		GetParent()->GetAbsDimensions(px, py, pw, ph);

		SetSize(600, 250);
		SetPos(pw/2 - GetWidth()/2, ph/2 - GetHeight()/2);
	}

	SetVerticalScrollBarEnabled(true);

	m_hInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_hInfo->SetSize(GetWidth(), GetHeight()-10);
	m_hInfo->SetPos(5, 30);

	m_hInfo->SetText("SMAK - The Super Model Army Knife\n");
	m_hInfo->AppendText("Version " SMAK_VERSION "\n");
	m_hInfo->AppendText("Written by Jorge Rodriguez <jorge@lunarworkshop.com>\n");
	m_hInfo->AppendText("\n");
	m_hInfo->AppendText("Copyright © 2012, Lunar Workshop, Inc\n");
	m_hInfo->AppendText("\n");
	m_hInfo->AppendText("This program is free software: you can redistribute it and/or modify\n");
	m_hInfo->AppendText("it under the terms of the GNU General Public License as published by\n");
	m_hInfo->AppendText("the Free Software Foundation, either version 3 of the License, or\n");
	m_hInfo->AppendText("(at your option) any later version.\n");
	m_hInfo->AppendText("\n");
	m_hInfo->AppendText("This program is distributed in the hope that it will be useful,\n");
	m_hInfo->AppendText("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	m_hInfo->AppendText("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	m_hInfo->AppendText("GNU General Public License for more details.\n");
	m_hInfo->AppendText("\n");
	m_hInfo->AppendText("\n");
	m_hInfo->AppendText("\n");
	m_hInfo->AppendText("Lunar Workshop's Tinker Application Engine\n");
	m_hInfo->AppendText("\n");
	m_hInfo->AppendText("Copyright © 2012, Lunar Workshop, Inc\n");
	m_hInfo->AppendText("\n");
	m_hInfo->AppendText("Redistribution and use in source and binary forms, with or without\n");
	m_hInfo->AppendText("modification, are permitted provided that the following conditions are met:\n");
	m_hInfo->AppendText("1. Redistributions of source code must retain the above copyright\n");
	m_hInfo->AppendText("   notice, this list of conditions and the following disclaimer.\n");
	m_hInfo->AppendText("2. Redistributions in binary form must reproduce the above copyright\n");
	m_hInfo->AppendText("   notice, this list of conditions and the following disclaimer in the\n");
	m_hInfo->AppendText("   documentation and/or other materials provided with the distribution.\n");
	m_hInfo->AppendText("3. All advertising materials mentioning features or use of this software\n");
	m_hInfo->AppendText("   must display the following acknowledgement:\n");
	m_hInfo->AppendText("   This product includes software developed by Lunar Workshop, Inc.\n");
	m_hInfo->AppendText("4. Neither the name of the Lunar Workshop nor the\n");
	m_hInfo->AppendText("   names of its contributors may be used to endorse or promote products\n");
	m_hInfo->AppendText("   derived from this software without specific prior written permission.\n");
	m_hInfo->AppendText("\n");
	m_hInfo->AppendText("THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY\n");
	m_hInfo->AppendText("EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n");
	m_hInfo->AppendText("WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n");
	m_hInfo->AppendText("DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY\n");
	m_hInfo->AppendText("DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n");
	m_hInfo->AppendText("(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n");
	m_hInfo->AppendText("LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND\n");
	m_hInfo->AppendText("ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n");
	m_hInfo->AppendText("(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n");
	m_hInfo->AppendText("SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n");
	m_hInfo->AppendText("\n");
	m_hInfo->AppendText("\n");
	m_hInfo->AppendText("\n");
	m_hInfo->AppendText("Third Party Libraries:\n");
#ifdef WITH_ASSIMP
	m_hInfo->AppendText("AssImp copyright © 2012, Alexander Gessler\n");
#endif
	m_hInfo->AppendText("FCollada copyright © 2006, Feeling Software\n");
#ifdef WITH_EASTL
	m_hInfo->AppendText("EASTL copyright © 2009-2010, Electronic Arts\n");
#endif
//	m_hInfo->AppendText("DevIL copyright © 2001-2009, Denton Woods\n");	// I might go back to it in the future, I'll keep this here for now
	m_hInfo->AppendText("FTGL copyright © 2001-2003, Henry Maddocks\n");
	m_hInfo->AppendText("GLFW copyright © 2002-2007, Camilla Berglund\n");
#ifdef _WIN32
	m_hInfo->AppendText("pthreads-win32 copyright © 2001, 2006 Ross P. Johnson\n");
#endif
	m_hInfo->AppendText("GL3W (public domain) created by Slavomir Kaslev and others\n");
	m_hInfo->AppendText("Freetype copyright © 1996-2002, 2006 by David Turner, Robert Wilhelm, and Werner Lemberg\n");

	m_hInfo->SetSize(GetWidth()-10, 0);
	m_hInfo->SetHeight(m_hInfo->GetTextHeight()+4);

	CMovablePanel::Layout();

	SetScissoring(true);
}

void CAboutPanel::Paint(float x, float y, float w, float h)
{
	CMovablePanel::Paint(x, y, w, h);
}

bool CAboutPanel::MousePressed(int iButton, int mx, int my)
{
	if (CMovablePanel::MousePressed(iButton, mx, my))
		return true;

	Close();

	return false;
}

void CAboutPanel::Open()
{
	if (!s_hAboutPanel)
		s_hAboutPanel = new CAboutPanel();

	s_hAboutPanel->SetVisible(true);
	s_hAboutPanel->Layout();
}
