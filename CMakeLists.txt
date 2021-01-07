project(xpc)

cmake_minimum_required(VERSION 2.4.0)
enable_language(C ASM)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2 -msse3 -w -nostdinc -fblocks")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/internal-include
)

add_definitions(-DMACH)

set(xpc_sources
	src/activity.c
	src/array.c
	src/bsm_wrappers.c
	src/connection.c
	src/dictionary.c
	src/error.c
	src/init.c
	src/misc.c
	src/pipe.c
	src/private.c
	src/reboot3.c
	src/serialization.c
	src/subr_sbuf.c
	src/todo.c
	src/transaction.m
	src/type.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libxpc.dylib")
add_circular(xpc FAT
	SOURCES
		${xpc_sources}
	SIBLINGS
		system_c
		system_kernel
		system_blocks
		libdispatch_shared
		platform
		system_malloc
		launch
		system_info
		system_dyld
	UPWARD
		# break an upward dependency on libobjc
		# ---
		# note that we don't actually need to intialize it ourselves by calling `objc_init`;
		# libSystem guarantees that we'll be initialized after libdispatch, and libdispatch already initializes libobjc
		objc
)
#target_link_libraries(xpc system)
install(TARGETS xpc DESTINATION libexec/darling/usr/lib/system)

target_link_options(xpc PRIVATE
	-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/scripts/exported-symbols.exp
)

