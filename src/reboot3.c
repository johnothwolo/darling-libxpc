#import <reboot2.h>
#import <xpc/xpc.h>

XPC_EXPORT
void *reboot3(uint64_t flags) {
	/* Let's just call reboot2 */
	/* It is defined in liblaunch */
	/* printf("libxpc reboot3 called with howto: %d\n", howto); */
	return reboot2(flags);
}
