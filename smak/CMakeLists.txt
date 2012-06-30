include_directories (
	${PROJECT_SOURCE_DIR}/../ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}/../ext-deps/glfw/include
	.
	../
	../common
	../common/math
	../tinker
)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/gl3w/include)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/glfw-1.3/include)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/pthreads-w32-2-8-0-release)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/ftgl-gl3/src)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/freetype-2.3.11/include)
endif (WIN32)

set (SMAK_SOURCES
	main.cpp
	models/loadsource.cpp
	models/models.cpp
)

set (SMAK_SOURCES_CRUNCH
	crunch/ao.cpp
	crunch/ao_raytrace.cpp
	crunch/crunch.cpp
	crunch/normal.cpp
)

set (SMAK_SOURCES_UI
	ui/smakwindow.cpp
	ui/smakwindow_input.cpp
	ui/smakwindow_ui.cpp
	ui/smak_renderer.cpp
	ui/picker.cpp
	ui/scenetree.cpp
	ui/aogenerator.cpp
	ui/buttonpanel.cpp
	ui/combogenerator.cpp
	ui/normalgenerator.cpp
	ui/progressbar.cpp
)

source_group(UI FILES ${SMAK_SOURCES_UI})
source_group(Crunch FILES ${SMAK_SOURCES_CRUNCH})

if (WIN32)
	set (SMAK_SOURCES ${SMAK_SOURCES}
		smak.rc
	)
endif (WIN32)

add_executable(smak WIN32 ${SMAK_SOURCES} ${SMAK_SOURCES_CRUNCH} ${SMAK_SOURCES_UI})
target_link_libraries (smak tinker modelconverter raytracer)

if (WIN32)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/../ext-deps/glfw/src/Debug/GLFW.lib)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/../ext-deps/ftgl-gl3/msvc/build/ftgl_static_D.lib)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/../ext-deps/freetype-2.3.11/objs/win32/vc2010/freetype2311MT_D.lib)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/../ext-deps/pthreads-w32-2-8-0-release/Debug/pthread.lib)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Debug/fcollada-2010.lib)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Debug/farchivexml-2010.lib)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Debug/libxml-2010.lib)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/../ext-deps/assimp/code/Debug/assimp.lib)

	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/../ext-deps/glfw/src/RelWithDebInfo/GLFW.lib)
	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/../ext-deps/ftgl-gl3/msvc/build/ftgl_static.lib)
	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/../ext-deps/freetype-2.3.11/objs/win32/vc2010/freetype2311MT.lib)
	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/../ext-deps/pthreads-w32-2-8-0-release/Release/pthread.lib)
	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Release/fcollada-2010.lib)
	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Release/farchivexml-2010.lib)
	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Release/libxml-2010.lib)
	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/../ext-deps/assimp/code/RelWithDebInfo/assimp.lib)
	target_link_libraries(smak optimized dbghelp.lib)

	target_link_libraries(smak opengl32 glu32 iphlpapi ws2_32)
endif (WIN32)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries (smak ${IL_LIBRARIES} ${ILU_LIBRARIES})

	target_link_libraries(smak ${FREETYPE_LIBRARIES} ${LIBXML2_LIBRARIES})
	target_link_libraries(smak ${CMAKE_SOURCE_DIR}/../ext-deps/fcolladaCE/FCollada/libfcollada.so)
	target_link_libraries(smak ${CMAKE_SOURCE_DIR}/../ext-deps/fcolladaCE/FColladaPlugins/FArchiveXML/libfarchivexml.so)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
