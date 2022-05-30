//
//  trace.m
//  xpc
//
//  Created by John Othwolo on 5/27/22.
//  Copyright © 2022 John Othwolo. All rights reserved.
//

#import <Foundation/Foundation.h>

extern int kdebug_trace(uint32_t code, uint64_t arg1, uint64_t arg2,
                                       uint64_t arg3, uint64_t arg4);

void xpc_ktrace_pid1(unsigned int a1, uint64_t a2){
//     pid_t v2 = getpid();
//     return kdebug_trace(a1, v2, a2, 0, 0);
    return;
 }
