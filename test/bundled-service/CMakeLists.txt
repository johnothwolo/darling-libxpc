add_darling_executable(libxpc_test_bundled_service service.c)

set_target_properties(libxpc_test_bundled_service PROPERTIES
	OUTPUT_NAME "XPCBundledService"
)

target_link_libraries(libxpc_test_bundled_service
	xpc_static
	objc
)

install(
	TARGETS
		libxpc_test_bundled_service
	DESTINATION
		libexec/darling/usr/libexec/test/xpc/XPCBundledService.xpc/Contents/MacOS
)

install(
	FILES
		Info.plist
	DESTINATION
		libexec/darling/usr/libexec/test/xpc/XPCBundledService.xpc/Contents
)
