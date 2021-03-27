#import <xpc/xpc.h>

extern void bootstrap_init(void); // in liblaunch

XPC_EXPORT
void _libxpc_initializer(void)
{
	bootstrap_init();
}

XPC_EXPORT
void xpc_atfork_child(void)
{
	bootstrap_init();
}

XPC_EXPORT
void xpc_atfork_parent(void)
{
}

XPC_EXPORT
void xpc_atfork_prepare(void)
{
}


