#import <xpc/xpc.h>
#import <xpc/util.h>

XPC_EXPORT const char* XPC_COALITION_INFO_KEY_BUNDLE_IDENTIFIER = "bundle_identifier";
XPC_EXPORT const char* XPC_COALITION_INFO_KEY_CID = "cid";
XPC_EXPORT const char* XPC_COALITION_INFO_KEY_NAME = "name";
XPC_EXPORT const char* XPC_COALITION_INFO_KEY_RESOURCE_USAGE_BLOB = "resource-usage-blob";

//
// private C API
//

XPC_EXPORT
xpc_object_t xpc_coalition_copy_info(uint64_t cid) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
int xpc_coalition_history_pipe_async(int flags) {
	xpc_stub();
	return -1;
};
