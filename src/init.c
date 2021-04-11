#import <xpc/xpc.h>

void xpc_stub_init(void);

extern void bootstrap_init(void); // in liblaunch

XPC_EXPORT
void _libxpc_initializer(void)
{
	xpc_stub_init();
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


