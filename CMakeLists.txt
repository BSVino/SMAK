cmake_minimum_required (VERSION 2.8)

set (CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_overrides.cmake)
set (CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_overrides.cmake)

include (${CMAKE_CURRENT_SOURCE_DIR}/cmake/utils.cmake)

project (SMAK)

if (WIN32)
	set (CMAKE_EXE_LINKER_FLAGS_DEBUG				"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL" CACHE STRING "" FORCE)
	set (CMAKE_EXE_LINKER_FLAGS_MINSIZEREL			"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE				"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO		"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_MODULE_LINKER_FLAGS_DEBUG			"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL" CACHE STRING "" FORCE)
	set (CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL		"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_MODULE_LINKER_FLAGS_RELEASE			"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO	"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_SHARED_LINKER_FLAGS_DEBUG			"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL" CACHE STRING "" FORCE)
	set (CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL		"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_SHARED_LINKER_FLAGS_RELEASE			"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO	"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL:NO" CACHE STRING "" FORCE)
endif (WIN32)

option(WITH_EASTL "Use the super-fast (even in debug mode) EASTL STL library." ON)
option(WITH_ASSIMP "Use the unfortunately named AssImp library to load more formats." ON)

if (WITH_EASTL)
	add_definitions(-DWITH_EASTL)
endif (WITH_EASTL)

if (WITH_ASSIMP)
	add_definitions(-DWITH_ASSIMP)
endif ()

if (CMAKE_COMPILER_IS_GNUCC)
	add_definitions(-std=c++0x)
endif (CMAKE_COMPILER_IS_GNUCC)

if (WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	add_definitions(-D_CRT_NON_CONFORMING_SWPRINTFS)
	add_definitions(-DEA_CHAR16_NATIVE=1)
	add_definitions(-D_UNICODE)
	add_definitions(-DFTGL_LIBRARY_STATIC)
endif (WIN32)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_definitions(-DFCOLLADA_DLL)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-D_DEBUG)
endif (CMAKE_BUILD_TYPE MATCHES Debug)

add_subdirectory (common)
add_subdirectory (modelconverter)
add_subdirectory (raytracer)
add_subdirectory (tinker)
add_subdirectory (smak)
add_subdirectory (tests)
