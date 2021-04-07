add_darling_executable(libxpc_test_server server.c)
add_darling_executable(libxpc_test_client client.c)

target_link_libraries(libxpc_test_server
	xpc_static
	objc
)

target_link_libraries(libxpc_test_client
	xpc_static
	objc
)

install(
	TARGETS
		libxpc_test_server
		libxpc_test_client
	DESTINATION
		libexec/darling/usr/libexec
)

install(
	FILES
		org.darlinghq.libxpc.test-service.plist
	DESTINATION
		libexec/darling/System/Library/LaunchDaemons
)
