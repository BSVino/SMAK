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

#include "application.h"

#include <time.h>
#include <GL3/gl3w.h>
#include <GL/glfw3.h>
#include <iostream>
#include <fstream>

#include <strutils.h>
#include <tinker_platform.h>
#include <mtrand.h>
#include <tvector.h>

#include <tinker/keys.h>
#include <tinker/cvar.h>
#include <glgui/rootpanel.h>
#include <tinker/renderer/renderer.h>

#include "console.h"

CApplication* CApplication::s_pApplication = NULL;

CApplication::CApplication(int argc, char** argv)
	: CShell(argc, argv)
{
	s_pApplication = this;

	srand((unsigned int)time(NULL));
	mtsrand((size_t)time(NULL));

	for (int i = 0; i < argc; i++)
		m_apszCommandLine.push_back(argv[i]);

	m_bIsOpen = false;
	m_bMultisampling = false;

	m_pConsole = NULL;

	SetMouseCursorEnabled(true);
	m_bMouseDownInGUI = false;
	m_flLastMousePress = -1;

	for (int i = 1; i < argc; i++)
	{
		if (m_apszCommandLine[i][0] == '+')
			CCommand::Run(&m_apszCommandLine[i][1]);
	}
}

#ifdef _DEBUG
#define GL_DEBUG_VALUE "1"
#else
#define GL_DEBUG_VALUE "0"
#endif

CVar gl_debug("gl_debug", GL_DEBUG_VALUE);

#ifndef CALLBACK
#define CALLBACK
#endif

void CALLBACK GLDebugCallback(GLenum iSource, GLenum iType, GLuint id, GLenum iSeverity, GLsizei iLength, const GLchar* pszMessage, GLvoid* pUserParam)
{
	if (iType != GL_DEBUG_TYPE_PERFORMANCE_ARB)
	{
		TAssert(iSeverity != GL_DEBUG_SEVERITY_HIGH_ARB);
		TAssert(iSeverity != GL_DEBUG_SEVERITY_MEDIUM_ARB);
	}

	if (gl_debug.GetBool())
	{
		tstring sMessage = "OpenGL Debug Message (";

		if (iSource == GL_DEBUG_SOURCE_API_ARB)
			sMessage += "Source: API ";
		else if (iSource == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
			sMessage += "Source: Window System ";
		else if (iSource == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
			sMessage += "Source: Shader Compiler ";
		else if (iSource == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
			sMessage += "Source: Third Party ";
		else if (iSource == GL_DEBUG_SOURCE_APPLICATION_ARB)
			sMessage += "Source: Application ";
		else if (iSource == GL_DEBUG_SOURCE_OTHER_ARB)
			sMessage += "Source: Other ";

		if (iType == GL_DEBUG_TYPE_ERROR_ARB)
			sMessage += "Type: Error ";
		else if (iType == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
			sMessage += "Type: Deprecated Behavior ";
		else if (iType == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)
			sMessage += "Type: Undefined Behavior ";
		else if (iType == GL_DEBUG_TYPE_PORTABILITY_ARB)
			sMessage += "Type: Portability ";
		else if (iType == GL_DEBUG_TYPE_PERFORMANCE_ARB)
			sMessage += "Type: Performance ";
		else if (iType == GL_DEBUG_TYPE_OTHER_ARB)
			sMessage += "Type: Other ";

		if (iSeverity == GL_DEBUG_SEVERITY_HIGH_ARB)
			sMessage += "Severity: High) ";
		else if (iSeverity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
			sMessage += "Severity: Medium) ";
		else if (iSeverity == GL_DEBUG_SEVERITY_LOW_ARB)
			sMessage += "Severity: Low) ";

		sMessage += convertstring<GLchar, tchar>(pszMessage) + "\n";

		TMsg(convertstring<GLchar, tchar>(sMessage).c_str());
	}
}

void CApplication::OpenWindow(size_t iWidth, size_t iHeight, bool bFullscreen, bool bResizeable)
{
	glfwInit();

	m_bFullscreen = bFullscreen;

	if (HasCommandLineSwitch("--fullscreen"))
		m_bFullscreen = true;

	if (HasCommandLineSwitch("--windowed"))
		m_bFullscreen = false;

	m_iWindowWidth = iWidth;
	m_iWindowHeight = iHeight;

    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 0);
    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);

	glfwOpenWindowHint(GLFW_WINDOW_RESIZABLE, bResizeable?GL_TRUE:GL_FALSE);

	if (m_bMultisampling)
		glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);

	if (HasCommandLineSwitch("--debug-gl"))
	{
		glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

		if (!glDebugMessageCallbackARB)
			TMsg("Your drivers do not support GL_ARB_debug_output, so no GL debug output will be shown.\n");
	}

	glfwOpenWindowHint(GLFW_DEPTH_BITS, 16);
	glfwOpenWindowHint(GLFW_RED_BITS, 8);
	glfwOpenWindowHint(GLFW_GREEN_BITS, 8);
	glfwOpenWindowHint(GLFW_BLUE_BITS, 8);
	glfwOpenWindowHint(GLFW_ALPHA_BITS, 8);

	TMsg(sprintf(tstring("Opening %dx%d %s %s window.\n"), iWidth, iHeight, bFullscreen?"fullscreen":"windowed", bResizeable?"resizeable":"fixed-size"));

	if (!(m_pWindow = (size_t)glfwOpenWindow(iWidth, iHeight, m_bFullscreen?GLFW_FULLSCREEN:GLFW_WINDOWED, WindowTitle().c_str(), NULL)))
	{
		glfwTerminate();
		return;
	}

	int iScreenWidth;
	int iScreenHeight;

	GetScreenSize(iScreenWidth, iScreenHeight);

	if (!m_bFullscreen)
	{
		// The taskbar is at the bottom of the screen. Pretend the screen is smaller so the window doesn't clip down into it.
		// Also the window's title bar at the top takes up space.
		iScreenHeight -= 70;

		int iWindowX = (int)(iScreenWidth/2-m_iWindowWidth/2);
		int iWindowY = (int)(iScreenHeight/2-m_iWindowHeight/2);
		glfwSetWindowPos((GLFWwindow)m_pWindow, iWindowX, iWindowY);
	}

	glfwSetWindowCloseCallback(&CApplication::WindowCloseCallback);
	glfwSetWindowSizeCallback(&CApplication::WindowResizeCallback);
	glfwSetKeyCallback(&CApplication::KeyEventCallback);
	glfwSetCharCallback(&CApplication::CharEventCallback);
	glfwSetMousePosCallback(&CApplication::MouseMotionCallback);
	glfwSetMouseButtonCallback(&CApplication::MouseInputCallback);
	glfwSetScrollCallback(&CApplication::MouseWheelCallback);
	glfwSwapInterval( 1 );
	glfwSetTime( 0.0 );

	InitJoystickInput();

	SetMouseCursorEnabled(true);

	GLenum err = gl3wInit();
	if (0 != err)
		exit(0);

	DumpGLInfo();

	if (glDebugMessageCallbackARB)
	{
		glDebugMessageCallbackARB(GLDebugCallback, nullptr);

		tstring sMessage("OpenGL Debug Output Activated");
		glDebugMessageInsertARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, 0, GL_DEBUG_SEVERITY_LOW_ARB, sMessage.length(), sMessage.c_str());
	}

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1.0);

	m_bIsOpen = true;

	m_pRenderer = CreateRenderer();
	m_pRenderer->Initialize();

	glgui::RootPanel()->SetSize((float)m_iWindowWidth, (float)m_iWindowHeight);
}

CApplication::~CApplication()
{
	glfwTerminate();
}

#define MAKE_PARAMETER(name) \
{ #name, name } \

void CApplication::DumpGLInfo()
{
	gl3wInit();

	std::ifstream i(GetAppDataDirectory(AppDirectory(), "glinfo.txt").c_str());
	if (i)
		return;
	i.close();

	std::ofstream o(GetAppDataDirectory(AppDirectory(), "glinfo.txt").c_str());
	if (!o || !o.is_open())
		return;

	o << "Vendor: " << (char*)glGetString(GL_VENDOR) << std::endl;
	o << "Renderer: " << (char*)glGetString(GL_RENDERER) << std::endl;
	o << "Version: " << (char*)glGetString(GL_VERSION) << std::endl;

	char* pszShadingLanguageVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	if (pszShadingLanguageVersion)
		o << "Shading Language Version: " << pszShadingLanguageVersion << std::endl;

	char* pszExtensions = (char*)glGetString(GL_EXTENSIONS);
	tstring sExtensions;
	if (pszExtensions)
		sExtensions = pszExtensions;
	tvector<tstring> asExtensions;
	strtok(sExtensions, asExtensions);
	o << "Extensions:" << std::endl;
	for (size_t i = 0; i < asExtensions.size(); i++)
		o << "\t" << asExtensions[i].c_str() << std::endl;

	typedef struct
	{
		const char* pszName;
		int iParameter;
	} GLParameter;

	GLParameter aParameters[] =
	{
		MAKE_PARAMETER(GL_MAX_TEXTURE_SIZE),
		MAKE_PARAMETER(GL_MAX_VIEWPORT_DIMS),
		MAKE_PARAMETER(GL_MAX_3D_TEXTURE_SIZE),
		MAKE_PARAMETER(GL_MAX_ELEMENTS_VERTICES),
		MAKE_PARAMETER(GL_MAX_ELEMENTS_INDICES),
		MAKE_PARAMETER(GL_MAX_TEXTURE_LOD_BIAS),
		MAKE_PARAMETER(GL_MAX_DRAW_BUFFERS),
		MAKE_PARAMETER(GL_MAX_VERTEX_ATTRIBS),
		MAKE_PARAMETER(GL_MAX_TEXTURE_IMAGE_UNITS),
		MAKE_PARAMETER(GL_MAX_VERTEX_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_VARYING_FLOATS),
		MAKE_PARAMETER(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS),
		MAKE_PARAMETER(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS),
		MAKE_PARAMETER(GL_MAX_CLIP_DISTANCES),
		MAKE_PARAMETER(GL_MAX_ARRAY_TEXTURE_LAYERS),
		MAKE_PARAMETER(GL_MAX_VARYING_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_TEXTURE_BUFFER_SIZE),
		MAKE_PARAMETER(GL_MAX_RECTANGLE_TEXTURE_SIZE),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_OUTPUT_VERTICES),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_VERTEX_OUTPUT_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_INPUT_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_RENDERBUFFER_SIZE),
		MAKE_PARAMETER(GL_MAX_COLOR_ATTACHMENTS),
		MAKE_PARAMETER(GL_MAX_SAMPLES),
		MAKE_PARAMETER(GL_MAX_VERTEX_UNIFORM_BLOCKS),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_UNIFORM_BLOCKS),
		MAKE_PARAMETER(GL_MAX_FRAGMENT_UNIFORM_BLOCKS),
		MAKE_PARAMETER(GL_MAX_COMBINED_UNIFORM_BLOCKS),
		MAKE_PARAMETER(GL_MAX_UNIFORM_BUFFER_BINDINGS),
		MAKE_PARAMETER(GL_MAX_UNIFORM_BLOCK_SIZE),
		MAKE_PARAMETER(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_COLOR_TEXTURE_SAMPLES),
		MAKE_PARAMETER(GL_MAX_DEPTH_TEXTURE_SAMPLES),
		MAKE_PARAMETER(GL_MAX_INTEGER_SAMPLES),
	};

	// Clear it
	glGetError();

	o << std::endl;

	for (size_t i = 0; i < sizeof(aParameters)/sizeof(GLParameter); i++)
	{
		GLint iValue[4];
		glGetIntegerv(aParameters[i].iParameter, &iValue[0]);

		if (glGetError() != GL_NO_ERROR)
			continue;

		o << aParameters[i].pszName << ": " << iValue[0] << std::endl;
	}
}

void CApplication::SwapBuffers()
{
	glfwSwapBuffers();
	glfwPollEvents();

	ProcessJoystickInput();
}

double CApplication::GetTime()
{
	return glfwGetTime();
}

bool CApplication::IsOpen()
{
	return !!glfwIsWindow((GLFWwindow)m_pWindow) && m_bIsOpen;
}

void Quit(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	CApplication::Get()->Close();
}

CCommand quit("quit", ::Quit);

void CApplication::Close()
{
	m_bIsOpen = false;
}

bool CApplication::HasFocus()
{
	return glfwGetWindowParam((GLFWwindow)m_pWindow, GLFW_ACTIVE) == GL_TRUE;
}

void CApplication::Render()
{
}

int CApplication::WindowClose()
{
	return GL_TRUE;
}

void CApplication::WindowResize(int w, int h)
{
	m_iWindowWidth = w;
	m_iWindowHeight = h;

	m_pRenderer->WindowResize(w, h);

	glgui::RootPanel()->SetSize((float)w, (float)h);
	glgui::RootPanel()->Layout();

	Render();

	SwapBuffers();
}

void CApplication::MouseMotion(int x, int y)
{
	glgui::CRootPanel::Get()->CursorMoved(x, y);
}

bool CApplication::MouseInput(int iButton, tinker_mouse_state_t iState)
{
	int mx, my;
	GetMousePosition(mx, my);
	if (iState == TINKER_MOUSE_PRESSED)
	{
		if (glgui::CRootPanel::Get()->MousePressed(iButton, mx, my))
		{
			m_bMouseDownInGUI = true;
			return true;
		}
		else
			m_bMouseDownInGUI = false;
	}
	else if (iState == TINKER_MOUSE_RELEASED)
	{
		if (glgui::CRootPanel::Get()->MouseReleased(iButton, mx, my))
			return true;

		if (m_bMouseDownInGUI)
		{
			m_bMouseDownInGUI = false;
			return true;
		}
	}
	else if (iState == TINKER_MOUSE_DOUBLECLICK)
	{
		if (glgui::CRootPanel::Get()->MouseDoubleClicked(iButton, mx, my))
			return true;

		if (m_bMouseDownInGUI)
		{
			m_bMouseDownInGUI = false;
			return true;
		}
	}

	return false;
}

tinker_keys_t MapKey(int c)
{
	switch (c)
	{
	case GLFW_KEY_ESC:
		return TINKER_KEY_ESCAPE;

	case GLFW_KEY_F1:
		return TINKER_KEY_F1;

	case GLFW_KEY_F2:
		return TINKER_KEY_F2;

	case GLFW_KEY_F3:
		return TINKER_KEY_F3;

	case GLFW_KEY_F4:
		return TINKER_KEY_F4;

	case GLFW_KEY_F5:
		return TINKER_KEY_F5;

	case GLFW_KEY_F6:
		return TINKER_KEY_F6;

	case GLFW_KEY_F7:
		return TINKER_KEY_F7;

	case GLFW_KEY_F8:
		return TINKER_KEY_F8;

	case GLFW_KEY_F9:
		return TINKER_KEY_F9;

	case GLFW_KEY_F10:
		return TINKER_KEY_F10;

	case GLFW_KEY_F11:
		return TINKER_KEY_F11;

	case GLFW_KEY_F12:
		return TINKER_KEY_F12;

	case GLFW_KEY_UP:
		return TINKER_KEY_UP;

	case GLFW_KEY_DOWN:
		return TINKER_KEY_DOWN;

	case GLFW_KEY_LEFT:
		return TINKER_KEY_LEFT;

	case GLFW_KEY_RIGHT:
		return TINKER_KEY_RIGHT;

	case GLFW_KEY_LSHIFT:
		return TINKER_KEY_LSHIFT;

	case GLFW_KEY_RSHIFT:
		return TINKER_KEY_RSHIFT;

	case GLFW_KEY_LCTRL:
		return TINKER_KEY_LCTRL;

	case GLFW_KEY_RCTRL:
		return TINKER_KEY_RCTRL;

	case GLFW_KEY_LALT:
		return TINKER_KEY_LALT;

	case GLFW_KEY_RALT:
		return TINKER_KEY_RALT;

	case GLFW_KEY_TAB:
		return TINKER_KEY_TAB;

	case GLFW_KEY_ENTER:
		return TINKER_KEY_ENTER;

	case GLFW_KEY_BACKSPACE:
		return TINKER_KEY_BACKSPACE;

	case GLFW_KEY_INSERT:
		return TINKER_KEY_INSERT;

	case GLFW_KEY_DEL:
		return TINKER_KEY_DEL;

	case GLFW_KEY_PAGEUP:
		return TINKER_KEY_PAGEUP;

	case GLFW_KEY_PAGEDOWN:
		return TINKER_KEY_PAGEDOWN;

	case GLFW_KEY_HOME:
		return TINKER_KEY_HOME;

	case GLFW_KEY_END:
		return TINKER_KEY_END;

	case GLFW_KEY_KP_0:
		return TINKER_KEY_KP_0;

	case GLFW_KEY_KP_1:
		return TINKER_KEY_KP_1;

	case GLFW_KEY_KP_2:
		return TINKER_KEY_KP_2;

	case GLFW_KEY_KP_3:
		return TINKER_KEY_KP_3;

	case GLFW_KEY_KP_4:
		return TINKER_KEY_KP_4;

	case GLFW_KEY_KP_5:
		return TINKER_KEY_KP_5;

	case GLFW_KEY_KP_6:
		return TINKER_KEY_KP_6;

	case GLFW_KEY_KP_7:
		return TINKER_KEY_KP_7;

	case GLFW_KEY_KP_8:
		return TINKER_KEY_KP_8;

	case GLFW_KEY_KP_9:
		return TINKER_KEY_KP_9;

	case GLFW_KEY_KP_DIVIDE:
		return TINKER_KEY_KP_DIVIDE;

	case GLFW_KEY_KP_MULTIPLY:
		return TINKER_KEY_KP_MULTIPLY;

	case GLFW_KEY_KP_SUBTRACT:
		return TINKER_KEY_KP_SUBTRACT;

	case GLFW_KEY_KP_ADD:
		return TINKER_KEY_KP_ADD;

	case GLFW_KEY_KP_DECIMAL:
		return TINKER_KEY_KP_DECIMAL;

	case GLFW_KEY_KP_EQUAL:
		return TINKER_KEY_KP_EQUAL;

	case GLFW_KEY_KP_ENTER:
		return TINKER_KEY_KP_ENTER;
	}

	if (c < 256)
		return (tinker_keys_t)TranslateKeyToQwerty(c);

	return TINKER_KEY_UKNOWN;
}

tinker_keys_t MapMouseKey(int c)
{
	switch (c)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
		return TINKER_KEY_MOUSE_LEFT;

	case GLFW_MOUSE_BUTTON_RIGHT:
		return TINKER_KEY_MOUSE_RIGHT;

	case GLFW_MOUSE_BUTTON_MIDDLE:
		return TINKER_KEY_MOUSE_MIDDLE;
	}

	return TINKER_KEY_UKNOWN;
}

tinker_keys_t MapJoystickKey(int c)
{
	switch (c)
	{
	case GLFW_JOYSTICK_1:
		return TINKER_KEY_JOYSTICK_1;

	case GLFW_JOYSTICK_2:
		return TINKER_KEY_JOYSTICK_2;

	case GLFW_JOYSTICK_3:
		return TINKER_KEY_JOYSTICK_3;

	case GLFW_JOYSTICK_4:
		return TINKER_KEY_JOYSTICK_4;

	case GLFW_JOYSTICK_5:
		return TINKER_KEY_JOYSTICK_5;

	case GLFW_JOYSTICK_6:
		return TINKER_KEY_JOYSTICK_6;

	case GLFW_JOYSTICK_7:
		return TINKER_KEY_JOYSTICK_7;

	case GLFW_JOYSTICK_8:
		return TINKER_KEY_JOYSTICK_8;

	case GLFW_JOYSTICK_9:
		return TINKER_KEY_JOYSTICK_9;

	case GLFW_JOYSTICK_10:
		return TINKER_KEY_JOYSTICK_10;
	}

	return TINKER_KEY_UKNOWN;
}

void CApplication::MouseInputCallback(void*, int iButton, int iState)
{
	Get()->MouseInputCallback(MapMouseKey(iButton), (tinker_mouse_state_t)iState);
}

void CApplication::MouseInputCallback(int iButton, tinker_mouse_state_t iState)
{
	if (iState == 1)
	{
		if (m_flLastMousePress < 0 || GetTime() - m_flLastMousePress > 0.25f)
			MouseInput(iButton, iState);
		else
			MouseInput(iButton, TINKER_MOUSE_DOUBLECLICK);
		m_flLastMousePress = GetTime();
	}
	else
		MouseInput(iButton, iState);
}

void CApplication::MouseWheelCallback(void*, int x, int y)
{
	Get()->MouseWheel(x, y);
}

void CApplication::KeyEvent(int c, int e)
{
	if (e == GLFW_PRESS)
		KeyPress(MapKey(c));
	else
		KeyRelease(MapKey(c));
}

void CApplication::CharEvent(int c)
{
	if (c == '`')
	{
		ToggleConsole();
		return;
	}

	if (glgui::CRootPanel::Get()->CharPressed(c))
		return;

	DoCharPress(c);
}

bool CApplication::KeyPress(int c)
{
	if (glgui::CRootPanel::Get()->KeyPressed(c, IsCtrlDown()))
		return true;

	if (c == TINKER_KEY_F4 && IsAltDown())
		exit(0);

	return DoKeyPress(c);
}

void CApplication::KeyRelease(int c)
{
	DoKeyRelease(c);
}

bool CApplication::IsCtrlDown()
{
	return glfwGetKey((GLFWwindow)m_pWindow, GLFW_KEY_LCTRL) || glfwGetKey((GLFWwindow)m_pWindow, GLFW_KEY_RCTRL);
}

bool CApplication::IsAltDown()
{
	return glfwGetKey((GLFWwindow)m_pWindow, GLFW_KEY_LALT) || glfwGetKey((GLFWwindow)m_pWindow, GLFW_KEY_RALT);
}

bool CApplication::IsShiftDown()
{
	return glfwGetKey((GLFWwindow)m_pWindow, GLFW_KEY_LSHIFT) || glfwGetKey((GLFWwindow)m_pWindow, GLFW_KEY_RSHIFT);
}

bool CApplication::IsMouseLeftDown()
{
	return glfwGetMouseButton((GLFWwindow)m_pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}

bool CApplication::IsMouseRightDown()
{
	return glfwGetMouseButton((GLFWwindow)m_pWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
}

bool CApplication::IsMouseMiddleDown()
{
	return glfwGetMouseButton((GLFWwindow)m_pWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
}

void CApplication::GetMousePosition(int& x, int& y)
{
	glfwGetMousePos((GLFWwindow)m_pWindow, &x, &y);
}

class CJoystick
{
public:
	CJoystick()
	{
		m_bPresent = false;
	}

public:
	bool					m_bPresent;
	tvector<float>			m_aflAxis;
	unsigned char			m_iButtons;
	unsigned long long		m_aiButtonStates;
};

static tvector<CJoystick> g_aJoysticks;
static const size_t MAX_JOYSTICKS = 16; // This is how many GLFW supports.

void CApplication::InitJoystickInput()
{
	g_aJoysticks.resize(MAX_JOYSTICKS);

	for (size_t i = 0; i < MAX_JOYSTICKS; i++)
	{
		if (glfwGetJoystickParam(GLFW_JOYSTICK_1 + i, GLFW_PRESENT) == GL_TRUE)
		{
			g_aJoysticks[i].m_bPresent = true;
			g_aJoysticks[i].m_aflAxis.resize(glfwGetJoystickParam(GLFW_JOYSTICK_1 + i, GLFW_AXES));

			for (size_t j = 0; j < g_aJoysticks[i].m_aflAxis.size(); j++)
				g_aJoysticks[i].m_aflAxis[j] = 0;

			g_aJoysticks[i].m_iButtons = glfwGetJoystickParam(GLFW_JOYSTICK_1 + i, GLFW_BUTTONS);
			g_aJoysticks[i].m_aiButtonStates = 0;

			TAssert(g_aJoysticks[i].m_iButtons < sizeof(g_aJoysticks[i].m_aiButtonStates)*8);
		}
	}
}

void CApplication::ProcessJoystickInput()
{
	if (g_aJoysticks.size() != MAX_JOYSTICKS)
		return;

	for (size_t i = 0; i < MAX_JOYSTICKS; i++)
	{
		CJoystick& oJoystick = g_aJoysticks[i];

		if (!oJoystick.m_bPresent)
			continue;

		static tvector<float> aflAxis;
		aflAxis.resize(oJoystick.m_aflAxis.size());
		glfwGetJoystickPos(i, &aflAxis[0], oJoystick.m_aflAxis.size());

		for (size_t j = 0; j < oJoystick.m_aflAxis.size(); j++)
		{
			if (aflAxis[j] != oJoystick.m_aflAxis[j])
				JoystickAxis(i, j, aflAxis[j], aflAxis[j]-oJoystick.m_aflAxis[j]);
		}

		oJoystick.m_aflAxis = aflAxis;

		static tvector<unsigned char> aiButtons;
		aiButtons.resize(oJoystick.m_iButtons);
		glfwGetJoystickButtons(i, &aiButtons[0], oJoystick.m_iButtons);

		for (size_t j = 0; j < oJoystick.m_iButtons; j++)
		{
			unsigned long long iButtonMask = (1<<j);
			if (aiButtons[j] == GLFW_PRESS && !(oJoystick.m_aiButtonStates&iButtonMask))
				JoystickButtonPress(i, MapJoystickKey(j));
			else if (aiButtons[j] == GLFW_RELEASE && (oJoystick.m_aiButtonStates&iButtonMask))
				JoystickButtonRelease(i, MapJoystickKey(j));

			if (aiButtons[j] == GLFW_PRESS)
				oJoystick.m_aiButtonStates |= iButtonMask;
			else
				oJoystick.m_aiButtonStates &= ~iButtonMask;
		}
	}
}

void CApplication::SetMouseCursorEnabled(bool bEnabled)
{
	if (bEnabled)
		glfwSetCursorMode( (GLFWwindow)m_pWindow, GLFW_CURSOR_NORMAL );
	else
		glfwSetCursorMode( (GLFWwindow)m_pWindow, GLFW_CURSOR_CAPTURED );

	m_bMouseEnabled = bEnabled;
}

bool CApplication::IsMouseCursorEnabled()
{
	return m_bMouseEnabled;
}

void CApplication::PrintConsole(const tstring& sText)
{
	GetConsole()->PrintConsole(sText);
}

void CApplication::PrintError(const tstring& sText)
{
	tstring sTrimmedText = sText;
	sTrimmedText.trim();

	GetConsole()->PrintConsole(tstring("[color=FF0000]ERROR: ") + sTrimmedText + "[/color]" + (sText.endswith("\n")?"\n":""));
}
