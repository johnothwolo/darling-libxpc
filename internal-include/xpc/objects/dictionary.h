#ifndef _XPC_OBJECTS_DICTIONARY_H_
#define _XPC_OBJECTS_DICTIONARY_H_

#import <xpc/objects/base.h>

#include <sys/queue.h>
#include <mach/mach.h>

@class XPC_CLASS(string);
@class XPC_CLASS(connection);

XPC_CLASS_DECL(dictionary);

typedef struct xpc_dictionary_entry_s* xpc_dictionary_entry_t;
struct xpc_dictionary_entry_s {
	LIST_ENTRY(xpc_dictionary_entry_s) link;
	XPC_CLASS(object)* object;
	char name[];
};

struct xpc_dictionary_s {
	struct xpc_object_s base;
	NSUInteger size;
	LIST_HEAD(, xpc_dictionary_entry_s) head;
	XPC_CLASS(connection)* associatedConnection;
	mach_port_t incoming_port;
	mach_port_t outgoing_port;
};

@interface XPC_CLASS_INTERFACE(dictionary)

// this API is modeled after NSMutableDictionary

@property(readonly) NSUInteger count;
@property(assign) XPC_CLASS(connection)* associatedConnection;

/**
 * The send(-once) port that can be used to reply to the remote peer that sent this dictionary.
 */
@property(assign) mach_port_t incomingPort;

/**
 * The send(-once) port that this dictionary is going to.
 */
@property(assign) mach_port_t outgoingPort;

/**
 * `YES` if this dictionary expects a reply from a remote peer, `NO` otherwise.
 */
@property(readonly) BOOL expectsReply;

/**
 * `YES` if this dictionary is a reply to an earlier message from a remote peer, `NO` otherwise.
 */
@property(readonly) BOOL isReply;

- (instancetype)initWithObjects: (XPC_CLASS(object)* const*)objects forKeys: (const char* const*)keys count: (NSUInteger)count;

- (XPC_CLASS(object)*)objectForKey: (const char*)key;
- (void)setObject: (XPC_CLASS(object)*)object forKey: (const char*)key;
- (void)removeObjectForKey: (const char*)key;
- (void)enumerateKeysAndObjectsUsingBlock: (void (^)(const char* key, XPC_CLASS(object)* obj, BOOL* stop))block;

// unfortunately, no keyed subscripts for this class because `const char*`s aren't valid subscripts

// NOTE: consider these as private methods
- (xpc_dictionary_entry_t)entryForKey: (const char*)key;
- (void)addEntry: (xpc_dictionary_entry_t)entry;
- (void)removeEntry: (xpc_dictionary_entry_t)entry;

// useful extensions:
- (XPC_CLASS(string)*)stringForKey: (const char*)key;

@end

#endif // _XPC_OBJECTS_DICTIONARY_H_
