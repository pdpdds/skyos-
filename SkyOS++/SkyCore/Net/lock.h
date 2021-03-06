/*
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef _KERNEL_LOCK_H
#define _KERNEL_LOCK_H

//#include <kernel/kernel.h>
//#include <kernel/debug.h>
#include "windef.h"

typedef struct recursive_lock {
	thread_id holder;
	sem_id sem;
	int recursion;
} recursive_lock;

int recursive_lock_create(recursive_lock *lock);
void recursive_lock_destroy(recursive_lock *lock);
bool recursive_lock_lock(recursive_lock *lock);
bool recursive_lock_unlock(recursive_lock *lock);
int recursive_lock_get_recursion(recursive_lock *lock);

#define ASSERT_LOCKED_RECURSIVE(r) { ASSERT(thread_get_current_thread_id() == (r)->holder); }

typedef struct mutex {
	thread_id holder;
	sem_id sem;
} mutex;

int mutex_init(mutex *m, const char *name);
void mutex_destroy(mutex *m);
void mutex_lock(mutex *m);
void mutex_unlock(mutex *m);

#define ASSERT_LOCKED_MUTEX(m) { ASSERT(thread_get_current_thread_id() == (m)->holder); }

#endif

