#ifndef _XPC_PRIVATE_DATE_H_
#define _XPC_PRIVATE_DATE_H_

#include <xpc/xpc.h>

__BEGIN_DECLS

xpc_object_t xpc_date_create_absolute(double value);
double xpc_date_get_value_absolute(xpc_object_t xdate);
bool xpc_date_is_int64_range(xpc_object_t xdate);

__END_DECLS

#endif // _XPC_PRIVATE_DATE_H_
