add_definitions(-DEA_CHAR16_NATIVE=0)

set (EASTL_SOURCES
	../../../ext-deps/EASTL/src/allocator.cpp
	../../../ext-deps/EASTL/src/assert.cpp
	../../../ext-deps/EASTL/src/fixed_pool.cpp
	../../../ext-deps/EASTL/src/hashtable.cpp
	../../../ext-deps/EASTL/src/red_black_tree.cpp
	../../../ext-deps/EASTL/src/string.cpp
)

add_library(eastl ${EASTL_SOURCES})
