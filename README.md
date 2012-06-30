SMAK - The Super Model Army Knife
=================================

This is the source code for [SMAK](http://getsmak.net), a tool that generates maps for 3D artists.

Goal
----

I started writing SMAK because I couldn't find any other ambient occlusion generator with a reasonable combination of these characteristics:

* Price
* Speed of generation
* Ease of use

So I wrote SMAK. For a while I sold it for what I thought was reasonable, but now I would rather let everyone use it and contribute to it, and so I'm releasing it under an open source license. I hope that in the future the project will continue to:

* Be incredibly easy to use
* Be pretty, let the user preview while they generate
* Gain more features and types of maps it can generate

If you want to contribute in any way, I would love that! Fork the project and send me pull requests, I'd be happy to include them. I don't work on SMAK full time, so help is greatly appreciated.

Overview
--------

You'll find these directories:

**common** - These files are general utility files. tstring.h is important - it defines a tstring class which is the string format you should use for applications based on Tinker. There is also tmap.h and tvector.h, for template maps and vectors. These are built on EASTL, which is super fast even in debug mode, much faster than Microsoft's template libraries. Avoid using windows.h or other platform-specific libraries, use tinker_platform.h from this directory instead.

**datamanager** - Sometimes data needs to be serialized to and from the hard drive. data.h contains a CData class that is a generic data container, and dataserializer.h can serialize any CData to and from a stream. Data is stored in a simple format that looks like this:

	key: value
	{
		// Sub-datas
		key: value
		key2: value

		// Values are optional
		key3
		{
			subdata
		}
	}

This format is used for things like shader .txt files (see below) and material .mat files.

**tinker** - Application classes and utility functions. The console, the profiler, etc.

These files can be used to create new applications. The creator of a project is responsible for the main() function of his application. Subclass the CApplication class (or CGameWindow if creating a game) and call OpenWindow() and then Run() to start your application.

**tinker/glgui** - This contains files for a GUI that renders directly to OpenGL. Tinker will automatically create the root panel for this GUI. To create a control use this syntax:

	glgui::CControl<glgui::CButton> hButton = hPanel->AddControl(new glgui::CButton("Push me!"));

Controls are handled by smart pointers and will de-allocate themselves once they're removed from their parent panel, or the parent panel is destroyed. Don't free a control's memory after it's passed into AddControl.

The ```Layout()``` function is responsible for laying out items on a screen. Mostly it should be used to set positions and sizes for child controls or itself. Layout is usually called when a panel is created, resized or changed somehow. The ```Think()``` function is called every frame. After all controls think then the ```Paint(x, y, w, h)``` function is called, all painting should be done here. Input events are handled through a variety of functions such as ```KeyPressed()``` and ```MousePressed()```. Things like button presses are handled with callbacks, set up when the button is created.

**tinker/renderer** - Shaders and rendering are carried out here. All OpenGL code is contained in these files. To render something, first create a rendering context:

	CRenderingContext c(SMAKRenderer(), true):

The rendering context retains a state similar to OpenGL 1/2's immediate mode states. Creating a rendering context is a bit like calling glPushAttrib()/glPushMatrix() and when one is destroyed it's a bit like calling glPopAttrib()/glPopMatrix(). Passing "true" as the second parameter tells the context that you wish to retain the previous context's state, otherwise passing "false" resets all state to defaults, including transformation matrices.

Shaders are loaded from text files in the ```shaders``` directory in the SMAK distribution. The shader library will read every .txt file in this directory and compile it as a new shader. The .txt file specifies which file should be used as the vertex and fragment shaders, so files can be reused between shaders. It also specifies parameters which can be specified in the material .mat files. The files ```header.si``` and ```functions.si``` are imported for you at the beginning of every shader. Some attributes and uniforms are hard-coded. For vertex shaders:

* ```uniform mat4x4 mProjection```
* ```uniform mat4x4 mView```
* ```uniform mat4x4 mGlobal```

And for fragment shaders:

* ```out vec4 vecOutputColor```

```vecOutputColor``` is the value to which the output value for a fragment shader should be assigned. The rest are up to you.

**tinker/textures** - Contains libraries for materials and textures. Both materials and textures are reference-counted and zero-count objects are periodically purged automatically. Materials can be loaded from .mat files, the parameters of which is specified by the shader .txt files, or they can be generated programatically like so:

	CData oMaterialData;
	CData* pShader = oMaterialData.AddChild("Shader", "model");

	pShader->AddChild("DiffuseTexture", "diffuse.png");
	pShader->AddChild("Diffuse", "0.5 0.5 0.5");
	pShader->AddChild("Ambient", "0.1 0.1 0.1");
	pShader->AddChild("Emissive", "0 0 0");
	pShader->AddChild("Specular", "0.1 0.1 0.1");
	pShader->AddChild("Shininess", "0.2");
	CMaterialHandle hMaterial = CMaterialLibrary::AddMaterial(&oMaterialData);

The ```CData``` values mirror the structure of a shader .txt file.

**modelconverter** - Can save and load different model formats.

**raytracer** - A raytracer.

Compiling
---------

SMAK relies on these packages as dependencies:

* EASTL (optional) [https://github.com/BSVino/EASTL.git]
* FCollada [https://github.com/BSVino/fcolladaCE.git]
* Freetype
* FTGL-GL3 [https://github.com/BSVino/ftgl-gl3.git]
* GL3W
* GLFW [https://github.com/BSVino/glfw.git]
* pthreads-w32 (win32 only)
* AssImp (optional)

Many of these packages require special versions with modifications so that they work with SMAK. I've denoted these libraries by putting their location in square brackets.

**Compiling on Linux** - See the file compiling.linux for a shell script that can be run to install the dependencies you'll need to build SMAK. Go to your source directory and type this:

	./compiling.linux

and then wait a while. Then type

	cmake .
	make

SMAK will be built and set up.

_Note: As of right now, the Linux port doesn't entirely work yet! I'm getting to it, sorry! You may have success in building it but it may not run. I'd love some help here!_

**Compiling for Windows** - Download the dependency packages and place them in a directory next to your source tree named ext-deps, which is where the CMake files will look for them when you compile.

Tinker uses CMake to generate its project files. Download and install the latest version of CMake (make sure to add it to your environment path) and create a "build" directory. (Google "out of source builds" for information.) Then use the CMake GUI to generate project files for you and open them with Visual Studio. Only Visual Studio is supported at the moment.

Tinker
------

SMAK is built on top of Tinker, Lunar Workshop's internal game engine. Only the application engine part is available in this release of SMAK.

The source code is fully featured with a shader and material system, and other cool stuff like a console and GUI. It's well profiled and runs on OpenGL 3.0.

Licenses
--------

Tinker is licensed under a permissive MIT-style license, meaning you may use it to build your own projects with no requirement to distribute the source code. This applies to all source files *not* in the ```smak``` subdirectory.

All source files in the ```smak``` subdirectory are licensed with GPL 3. This means that if you build a derivative work using these source files, you're required to distribute the source code alongside the compiled program.

The full texts of these licenses is included in the files ```LICENSE.Tinker``` and ```LICENSE.SMAK```
