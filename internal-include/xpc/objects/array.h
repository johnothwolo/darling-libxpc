#ifndef _XPC_OBJECTS_ARRAY_H_
#define _XPC_OBJECTS_ARRAY_H_

#import <xpc/objects/base.h>
#import <Foundation/NSEnumerator.h>

XPC_CLASS_DECL(array);

struct xpc_array_s {
	struct xpc_object_s base;
	NSUInteger size;
	XPC_CLASS(object)** array;
};

@interface XPC_CLASS_INTERFACE(array)

// this API is modeled after NSMutableArray

@property(readonly) NSUInteger count;

- (instancetype)initWithObjects: (XPC_CLASS(object)* const*)objects count: (NSUInteger)count;

- (XPC_CLASS(object)*)objectAtIndex: (NSUInteger)index;
- (void)addObject: (XPC_CLASS(object)*)object;
- (void)replaceObjectAtIndex: (NSUInteger)index withObject: (XPC_CLASS(object)*)object;
- (void)enumerateObjectsUsingBlock: (void (^)(XPC_CLASS(object)* object, NSUInteger index, BOOL* stop))block;
- (XPC_CLASS(object)*)objectAtIndexedSubscript: (NSUInteger)index;
- (void)setObject: (XPC_CLASS(object)*)object atIndexedSubscript: (NSUInteger)index;
- (NSUInteger)countByEnumeratingWithState: (NSFastEnumerationState*)state objects: (id __unsafe_unretained [])objects count: (NSUInteger)count;

@end

#endif // _XPC_OBJECTS_ARRAY_H_
