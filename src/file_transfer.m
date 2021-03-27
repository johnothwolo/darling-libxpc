#import <xpc/objects/file_transfer.h>
#import <xpc/xpc.h>

XPC_CLASS_SYMBOL_DECL(file_transfer);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(file_transfer)

XPC_CLASS_HEADER(file_transfer);

@end

//
// private C API
//

XPC_EXPORT
void xpc_file_transfer_cancel(xpc_file_transfer_t xtransfer) {
	// actually just crashes
};

XPC_EXPORT
dispatch_io_t xpc_file_transfer_copy_io(xpc_file_transfer_t xtransfer) {
	return NULL;
};

XPC_EXPORT
xpc_file_transfer_t xpc_file_transfer_create_with_fd(int fd, void (^completionCallback)(void)) {
	return NULL;
};

XPC_EXPORT
xpc_file_transfer_t xpc_file_transfer_create_with_path(const char* path, void (^completionCallback)(void)) {
	return NULL;
};

XPC_EXPORT
uint64_t xpc_file_transfer_get_size(xpc_file_transfer_t xtransfer) {
	// return type is either `uint64_t` or `size_t`
	return 0;
};

XPC_EXPORT
uint64_t xpc_file_transfer_get_transfer_id(xpc_file_transfer_t xtransfer) {
	return 0;
};

XPC_EXPORT
void xpc_file_transfer_send_finished(xpc_file_transfer_t xtransfer, bool succeeded) {
	// unsure about the second parameter (both the return type and the purpose)
};

XPC_EXPORT
void xpc_file_transfer_set_transport_writing_callbacks(xpc_file_transfer_t xtransfer, void (^writeCallback)(void), void (^sendCallback)(void)) {
	// parameters 2 & 3 are complete guesses;
	// the only thing i know about them is that they're blocks
};

XPC_EXPORT
void xpc_file_transfer_write_finished(xpc_file_transfer_t xtransfer, bool succeeded) {
	// i'm guessing this function is like `xpc_file_transfer_send_finished`
};

XPC_EXPORT
void* xpc_file_transfer_write_to_fd(xpc_file_transfer_t xtransfer, int fd, void (^someCallback)(void)) {
	// no clue what the return type is
	// also unsure about parameter 3's type
	return NULL;
};

XPC_EXPORT
void* xpc_file_transfer_write_to_path(xpc_file_transfer_t xtransfer, const char* path, void (^someCallback)(void)) {
	// same issues as `xpc_file_transfer_write_to_fd`
	return NULL;
};
