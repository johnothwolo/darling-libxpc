
extern void bootstrap_init(void); // in liblaunch

void _libxpc_initializer(void)
{
	bootstrap_init();
}

void xpc_atfork_child(void)
{
	bootstrap_init();
}

void xpc_atfork_parent(void)
{
}

void xpc_atfork_prepare(void)
{
}


