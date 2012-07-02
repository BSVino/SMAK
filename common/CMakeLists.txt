include_directories (
	${PROJECT_SOURCE_DIR}/../ext-deps/EASTL/include
	../
	../common
	math
)

if (WIN32)
	include_directories (
		${PROJECT_SOURCE_DIR}/../ext-deps/pthreads-w32-2-8-0-release
	)
endif (WIN32)

set (LWBASE_SOURCES
	configfile.cpp
	mempool.cpp
	mtrand.cpp
	stb_image.cpp
	stb_image_write.cpp
	parallelize.cpp
	sockets/sockets.cpp
	math/color.cpp
	math/matrix.cpp
	math/quaternion.cpp
	math/frustum.cpp
	math/vector.cpp
	math/mesh.cpp

	../datamanager/data.cpp
	../datamanager/dataserializer.cpp
)

if (WITH_EASTL)
	set (LWBASE_SOURCES ${LWBASE_SOURCES}
		eastl.cpp
	)
endif (WITH_EASTL)

if (WIN32)
	set (LWBASE_SOURCES ${LWBASE_SOURCES}
		platform_win32.cpp
	)
endif (WIN32)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (LWBASE_SOURCES ${LWBASE_SOURCES}
		platform_linux.cpp
	)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

add_library(lwbase ${LWBASE_SOURCES})

if (WIN32)
	target_link_libraries(lwbase iphlpapi ws2_32 dbghelp)
endif (WIN32)

if (WITH_EASTL)
	add_subdirectory (eastl)

	target_link_libraries(lwbase eastl)
endif (WITH_EASTL)
