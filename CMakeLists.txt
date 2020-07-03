project(xpc)

cmake_minimum_required(VERSION 2.4.0)
enable_language(C ASM)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2 -msse3 -w -nostdinc -fblocks")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)

add_definitions(-DMACH)

set(xpc_sources
	init.c
	todo.c
	mpack.c
	msgio.c
	bsm_wrappers.c
	xpc_activity.c
	xpc_array.c
	xpc_connection.c
	xpc_dictionary.c
	xpc_errors.c
	xpc_misc.c
	xpc_private.c
	xpc_type.c
	subr_sbuf.c
	xpc_pipe.c
	reboot3.c
	transaction_private.c
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
)
#target_link_libraries(xpc system)
install(TARGETS xpc DESTINATION libexec/darling/usr/lib/system)

