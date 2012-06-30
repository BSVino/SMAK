#ifdef _WIN32
#include <Windows.h>
#endif

#include <GL/glfw3.h>

#include <tinker_platform.h>
#include <strutils.h>

#include <modelconverter/modelconverter.h>
#include "ui/smakwindow.h"
#include "crunch/crunch.h"
#include "crunch/ao.h"
#include "ui/smak_renderer.h"

typedef enum
{
	COMMAND_NONE = 0,
	COMMAND_AO,
} command_t;

class CPrintingWorkListener : public IWorkListener
{
public:
	virtual void BeginProgress() {};
	virtual void SetAction(const tstring& sAction, size_t iTotalProgress)
	{
		printf("\n");
		puts(sAction.c_str());
		if (!iTotalProgress)
			printf("...");

		m_iTotalProgress = iTotalProgress;
		m_iProgress = 0;
	}

	virtual void WorkProgress(size_t iProgress, bool bForceDraw = false)
	{
		if (!m_iTotalProgress)
			return;

		size_t iLastProgress = m_iProgress;
		m_iProgress = iProgress;

		size_t iLastPercent = (iLastProgress * 10 / m_iTotalProgress);
		size_t iPercent = (iProgress * 10 / m_iTotalProgress);

		if (iPercent > iLastPercent)
		{
			printf("%d", (int)iPercent);
			return;
		}

		iLastPercent = (iLastProgress * 40 / m_iTotalProgress);
		iPercent = (iProgress * 40 / m_iTotalProgress);

		if (iPercent > iLastPercent)
			printf(".");
	}

	virtual void EndProgress()
	{
		printf("\n");
	}

public:
	size_t					m_iProgress;
	size_t					m_iTotalProgress;
};

int CreateApplication(int argc, char** argv)
{
	tstring sFile;
	command_t eCommand = COMMAND_NONE;
	aomethod_t eMethod = AOMETHOD_SHADOWMAP;
	size_t iSize = 1024;
	size_t iBleed = 1;
	size_t iLights = 3000;
	size_t iSamples = 20;
	float flRayFalloff = 1.0f;
	bool bRandomize = false;
	bool bCrease = false;
	bool bGroundOcclusion = false;
	tstring sOutput;

	CRenderer::AllowNPO2TextureLoads();

	if (argc >= 2)
	{
		for (int i = 1; i < argc; i++)
		{
			tstring sToken = argv[i];

			if (sToken[0] == '-')
			{
				// It's an argument
				if (sToken == "--command")
				{
					i++;
					tstring sToken = argv[i];
					if (sToken == "ao")
						eCommand = COMMAND_AO;
				}
				else if (sToken == "--method")
				{
					i++;
					tstring sToken = argv[i];
					if (sToken == "shadowmap")
						eMethod = AOMETHOD_SHADOWMAP;
					else if (sToken == "raytrace")
						eMethod = AOMETHOD_RAYTRACE;
					else if (sToken == "color")
						eMethod = AOMETHOD_RENDER;
					else
						printf("ERROR: Unrecognized method.\n");
				}
				else if (sToken == "--size")
				{
					i++;
					tstring sToken = argv[i];
					iSize = stoi(sToken);
					if (iSize < 64)
						iSize = 64;
					else if (iSize > 2048)
						iSize = 2048;
				}
				else if (sToken == "--bleed")
				{
					i++;
					tstring sToken = argv[i];
					iBleed = stoi(sToken);
					if (iBleed < 0)
						iBleed = 0;
					else if (iBleed > 10)
						iBleed = 10;
				}
				else if (sToken == "--lights")
				{
					i++;
					tstring sToken = argv[i];
					iLights = stoi(sToken);
					if (iLights < 500)
						iLights = 500;
					else if (iSize > 3000)
						iLights = 3000;
				}
				else if (sToken == "--samples")
				{
					i++;
					tstring sToken = argv[i];
					iSamples = stoi(sToken);
					if (iSamples < 5)
						iSamples = 5;
					else if (iSamples > 25)
						iSamples = 25;
				}
				else if (sToken == "--falloff")
				{
					i++;
					tstring sToken = argv[i];
					if (sToken == "none")
						flRayFalloff = -1.0f;
					else
					{
						flRayFalloff = (float)stof(sToken);
						if (flRayFalloff < 0.0001f)
							flRayFalloff = 0.0001f;
					}
				}
				else if (sToken == "--randomize")
				{
					bRandomize = true;
				}
				else if (sToken == "--crease")
				{
					bCrease = true;
				}
				else if (sToken == "--groundocclusion")
				{
					bGroundOcclusion = true;
				}
				else if (sToken == "--output")
				{
					i++;
					tstring sToken = argv[i];
					sOutput = sToken;
				}
			}
			else
			{
				// It's a file
				sFile = sToken;
			}
		}

		switch (eCommand)
		{
		case COMMAND_AO:
		{
			puts((tstring("Generating ambient occlusion map for ") + sFile + "\n").c_str());
			switch (eMethod)
			{
			case AOMETHOD_RENDER:
				printf("Method: Color AO\n");
				break;

			case AOMETHOD_RAYTRACE:
				printf("Method: Raytrace\n");
				break;

			case AOMETHOD_SHADOWMAP:
				printf("Method: Shadow mapping\n");
				break;
			}
			printf("Size: %dx%d\n", (int)iSize, (int)iSize);
			printf("Bleed: %d\n", (int)iBleed);
			if (eMethod == AOMETHOD_SHADOWMAP)
				printf("Lights: %d\n", (int)iLights);
			else if (eMethod == AOMETHOD_RAYTRACE)
				printf("Samples: %d\n", (int)iSamples);
			puts((tstring("Output file: ") + sOutput + "\n").c_str());

			CConversionScene s;
			CModelConverter c(&s);

			CPrintingWorkListener l;

			c.SetWorkListener(&l);

			if (!c.ReadModel(sFile.c_str()))
			{
				printf("Unsupported model format.\n");
				return 1;
			}

			if (eMethod == AOMETHOD_SHADOWMAP || eMethod == AOMETHOD_RENDER)
			{
				glfwInit();

				// The easy way to get a "windowless" context.
				GLFWwindow i = glfwOpenWindow(100, 100, GLFW_WINDOWED, "SMAK a Batch", nullptr);
				glfwIconifyWindow(i);
			}

			// If this is the color AO method, we need to load the textures.
			//tvector<CMaterial> aMaterials;
			if (eMethod == AOMETHOD_RENDER)
			{
				TUnimplemented();
#ifdef OPENGL2
				for (size_t i = 0; i < s.GetNumMaterials(); i++)
				{
					CConversionMaterial* pMaterial = s.GetMaterial(i);

					aMaterials.push_back(CMaterial(0));

					TAssert(aMaterials.size()-1 == i);

					size_t iTexture = CSMAKWindow::LoadTextureIntoGL(pMaterial->GetDiffuseTexture());

					if (iTexture)
						aMaterials[i].m_iBase = iTexture;
				}

				if (!aMaterials.size())
				{
					aMaterials.push_back(CMaterial(0));
				}
#endif
			}

			CAOGenerator ao(&s);

			ao.SetMethod(eMethod);
			ao.SetSize(iSize, iSize);
			ao.SetBleed(iBleed);
			ao.SetUseTexture(true);
			if (eMethod == AOMETHOD_SHADOWMAP)
				ao.SetSamples(iLights);
			else if (eMethod == AOMETHOD_RAYTRACE)
				ao.SetSamples(iSamples);
			ao.SetRandomize(bRandomize);
			ao.SetCreaseEdges(bCrease);
			ao.SetGroundOcclusion(bGroundOcclusion);

			ao.SetWorkListener(&l);

			ao.Generate();

			printf("Saving results...\n");
			if (sOutput.length())
				ao.SaveToFile(sOutput.c_str());
			else
				ao.SaveToFile("ao-output.png");

			printf("Done.\n");

			return 0;
		}
		}
	}

	CSMAKWindow oWindow(argc, argv);

	oWindow.OpenWindow();

	if (sFile.length())
		oWindow.ReadFile(sFile.c_str());

	oWindow.Run();

	return 0;
}

int main(int argc, char** argv)
{
#ifdef _WIN32
	// Make sure we open up an assert messagebox window instead of just aborting like it does for console apps.
	_set_error_mode(_OUT_TO_MSGBOX);

#ifndef _DEBUG
	__try
	{
#endif
#endif

	return CreateApplication(argc, argv);

#if defined(_WIN32) && !defined(_DEBUG)
	}
	__except (CreateMinidump(GetExceptionInformation(), "SMAK"), EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif
}
