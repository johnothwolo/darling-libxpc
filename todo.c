#include <xpc/private.h>

#include <stddef.h>
#include <stdio.h>

#define STUB() printf("STUB %s called\n", __func__)

xpc_object_t xpc_create_from_plist(void *data, size_t size)
{
	STUB();
	return NULL;
}

void xpc_connection_set_target_uid(xpc_connection_t connection, uid_t uid)
{
	STUB();
}

void xpc_connection_set_instance(xpc_connection_t connection, uuid_t uid)
{
	STUB();
}

/* already implemented!
void xpc_dictionary_set_mach_send(xpc_object_t object, char *type, int port)
{
	STUB();
}
*/
