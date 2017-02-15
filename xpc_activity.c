/*
This file is part of Darling.

Copyright (C) 2017 Darling developers

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Block.h>

#include "xpc/xpc.h"
#include "xpc_internal.h"


const char *XPC_ACTIVITY_INTERVAL = "Interval";
const char *XPC_ACTIVITY_REPEATING = "Repeating";
const char *XPC_ACTIVITY_DELAY = "Delay";
const char *XPC_ACTIVITY_GRACE_PERIOD = "GracePeriod";

const int64_t XPC_ACTIVITY_INTERVAL_1_MIN = 60;
const int64_t XPC_ACTIVITY_INTERVAL_5_MIN = 300;
const int64_t XPC_ACTIVITY_INTERVAL_15_MIN = 900;
const int64_t XPC_ACTIVITY_INTERVAL_30_MIN = 1800;
const int64_t XPC_ACTIVITY_INTERVAL_1_HOUR = 3600;
const int64_t XPC_ACTIVITY_INTERVAL_4_HOURS = 14400;
const int64_t XPC_ACTIVITY_INTERVAL_8_HOURS = 28800;
const int64_t XPC_ACTIVITY_INTERVAL_1_DAY = 86400;
const int64_t XPC_ACTIVITY_INTERVAL_7_DAYS = 604800;

const char *XPC_ACTIVITY_PRIORITY = "Priority";
const char *XPC_ACTIVITY_PRIORITY_MAINTENANCE = "Maintenance";
const char *XPC_ACTIVITY_PRIORITY_UTILITY = "Utility";
const char *XPC_ACTIVITY_ALLOW_BATTERY = "AllowBattery";
const char *XPC_ACTIVITY_REQUIRE_SCREEN_SLEEP = "RequireScreenSleep";
const char *XPC_ACTIVITY_REQUIRE_BATTERY_LEVEL = "RequireBatteryLevel";
const char *XPC_ACTIVITY_REQUIRE_HDD_SPINNING = "RequireHDDSpinning";

static const struct xpc_object _xpc_activity_check_in = {
	.xo_xpc_type = _XPC_TYPE_STRING,
	.xo_size = 10,		/* strlen("<CHECK-IN>") */
	.xo_refcnt = 1,
	.xo_u = {
		.str = "<CHECK-IN>"
	}
};

const xpc_object_t XPC_ACTIVITY_CHECK_IN = &_xpc_activity_check_in;

void
xpc_activity_register(const char *identifier, xpc_object_t criteria,
	xpc_activity_handler_t handler)
{
	/* TODO: global activity registry, XPC_ACTIVITY_CHECK_IN, actual scheduling */

	xpc_u value;

	xpc_activity_t activity = _xpc_prim_create(_XPC_TYPE_ACTIVITY, value, 0);
	xpc_activity_set_criteria(activity, criteria);
	xpc_activity_set_state(activity, XPC_ACTIVITY_STATE_WAIT);

	xpc_activity_handler_t handler_copy =
		(xpc_activity_handler_t) Block_copy(handler);

	dispatch_queue_t q = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0);
	dispatch_async(q, ^ {
		xpc_activity_set_state(activity, XPC_ACTIVITY_STATE_RUN);
		handler_copy(activity);
		Block_release(handler_copy);
		xpc_release(activity);
	});
}

xpc_object_t
xpc_activity_copy_criteria(xpc_activity_t activity)
{
	struct xpc_object *xo;

	xo = activity;
	return xo->xo_activity.criteria;
}

void
xpc_activity_set_criteria(xpc_activity_t activity, xpc_object_t criteria)
{
	struct xpc_object *xo;

	xo = activity;
	xpc_release(xo->xo_activity.criteria);
	xpc_retain(criteria);
	xo->xo_activity.criteria = criteria;
}

xpc_activity_state_t
xpc_activity_get_state(xpc_activity_t activity)
{
	struct xpc_object *xo;

	xo = activity;
	return xo->xo_activity.state;
}

bool
xpc_activity_set_state(xpc_activity_t activity, xpc_activity_state_t state)
{
	struct xpc_object *xo;

	xo = activity;
	xo->xo_activity.state = state;
	return true;
}

bool
xpc_activity_should_defer(xpc_activity_t activity)
{
	return false;
}

void
xpc_activity_unregister(const char *identifier)
{

}
