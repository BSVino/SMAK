if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(LibXml2 REQUIRED)
	include_directories(${LIBXML2_INCLUDE_DIR})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories (
	${PROJECT_SOURCE_DIR}/../ext-deps/EASTL/include
	${PROJECT_SOURCE_DIR}/../ext-deps/fcollada/FCollada
	${PROJECT_SOURCE_DIR}/../ext-deps/fcollada/FCollada/LibXML/include
	${PROJECT_SOURCE_DIR}/../ext-deps/fcolladaCE/FCollada
	${PROJECT_SOURCE_DIR}/../ext-deps/assimp/include
	../common
	../common/math
)

set (MODELCONVERTER_SOURCES_FORMATS
	formats/collada.cpp
	formats/obj.cpp
	formats/silo.cpp
	formats/smd.cpp
)

if (WITH_ASSIMP)
	set (MODELCONVERTER_SOURCES_FORMATS ${MODELCONVERTER_SOURCES_FORMATS}
		formats/assimp.cpp
	)
endif (WITH_ASSIMP)


source_group(Formats FILES ${MODELCONVERTER_SOURCES_FORMATS})

add_library (modelconverter
	convmesh.cpp
	modelconverter.cpp
	${MODELCONVERTER_SOURCES_FORMATS}
)
