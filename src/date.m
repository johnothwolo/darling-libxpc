/**
 * This file is part of Darling.
 *
 * Copyright (C) 2021 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#import <xpc/objects/date.h>
#import <xpc/util.h>
#import <xpc/xpc.h>
#import <Foundation/NSDate.h>
#import <xpc/serialization.h>

#include <time.h>

XPC_CLASS_SYMBOL_DECL(date);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(date)

XPC_CLASS_HEADER(date);

- (char*)xpcDescription
{
	char* output = NULL;
	asprintf(&output, "<%s: %lld>", xpc_class_name(self), self.value);
	return output;
}

- (int64_t)value
{
	XPC_THIS_DECL(date);
	if (this->is_absolute) {
		return (int64_t)((this->absolute_value + (double)NSTimeIntervalSince1970) * (double)NSEC_PER_SEC);
	} else {
		return this->value;
	}
}

- (void)setValue: (int64_t)value
{
	XPC_THIS_DECL(date);
	this->value = value;
	this->is_absolute = false;
}

- (double)absoluteValue
{
	XPC_THIS_DECL(date);
	if (this->is_absolute) {
		return this->absolute_value;
	} else {
		return ((double)this->value / (double)NSEC_PER_SEC) - (double)NSTimeIntervalSince1970;
	}
}

- (void)setAbsoluteValue: (double)absoluteValue
{
	XPC_THIS_DECL(date);
	this->absolute_value = absoluteValue;
	this->is_absolute = true;
}

- (instancetype)initWithValue: (int64_t)value
{
	if (self = [super init]) {
		XPC_THIS_DECL(date);
		this->value = value;
	}
	return self;
}

- (instancetype)initWithAbsoluteValue: (double)value
{
	if (self = [super init]) {
		XPC_THIS_DECL(date);
		this->absolute_value = value;
		this->is_absolute = true;
	}
	return self;
}

- (NSUInteger)hash
{
	XPC_THIS_DECL(date);
	return xpc_raw_data_hash(&this->value, sizeof(this->value));
}

@end

XPC_WRAPPER_CLASS_SERIAL_IMPL(date, int64_t, DATE, U64, uint64_t);

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_date_create(int64_t value) {
	return [[XPC_CLASS(date) alloc] initWithValue: value];
};

XPC_EXPORT
xpc_object_t xpc_date_create_from_current(void) {
	return xpc_date_create((int64_t)clock_gettime_nsec_np(CLOCK_REALTIME));
};

XPC_EXPORT
int64_t xpc_date_get_value(xpc_object_t xdate) {
	TO_OBJC_CHECKED(date, xdate, date) {
		return date.value;
	}
	return 0;
};

//
// private C API
//

XPC_EXPORT
xpc_object_t xpc_date_create_absolute(double value) {
	return [[XPC_CLASS(date) alloc] initWithAbsoluteValue: value];
};

XPC_EXPORT
double xpc_date_get_value_absolute(xpc_object_t xdate) {
	TO_OBJC_CHECKED(date, xdate, date) {
		return date.absoluteValue;
	}
	return 0;
};

XPC_EXPORT
bool xpc_date_is_int64_range(xpc_object_t xdate) {
	TO_OBJC_CHECKED(date, xdate, date) {
		return true;
	}
	return false;
};
