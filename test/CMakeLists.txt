set(TEST_SOURCES
	array.m
	base.m
	bool.c
	data.c
	dictionary.m
	main.c
	string.c
	type.c
)

include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/ctest"
)

add_darling_executable(test_libxpc ${TEST_SOURCES})

target_link_libraries(test_libxpc
	# link statically to ensure we always use our copy of libxpc for testing, not the system's
	xpc_static
	objc
)

install(TARGETS test_libxpc DESTINATION libexec/darling/usr/libexec)
