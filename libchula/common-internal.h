/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/* All files in libchula are Copyright (C) 2014 Alvaro Lopez Ortega.
 *
 *   Authors:
 *     * Alvaro Lopez Ortega <alvaro@alobbs.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CHULA_COMMON_INTERNAL_H
#define CHULA_COMMON_INTERNAL_H

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#include <libchula/macros.h>
#include <libchula/log.h>

#if defined HAVE_ENDIAN_H
# include <endian.h>
#elif defined HAVE_MACHINE_ENDIAN_H
# include <machine/endian.h>
#elif defined HAVE_SYS_ENDIAN_H
# include <sys/endian.h>
#elif defined HAVE_SYS_MACHINE_H
# include <sys/machine.h>
#elif defined HAVE_SYS_ISA_DEFS_H
# include <sys/isa_defs.h>
#else
# error "Can not include endian.h"
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_SYS_VARARGS_H
# include <sys/varargs.h>
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef HAVE_SYS_FCNTL_H
# include <sys/fcntl.h>
#endif

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#elif defined(HAVE_STDINT_H)
# include <stdint.h>
#else
# error "Can not include inttypes or stdint"
#endif

/* Int limit
 */
#ifndef INT_MAX
# if (SIZEOF_INT == 4)
#  define INT_MAX 0x7fffffffL          /* 2^32 - 1 */
# elif (SIZEOF_INT == 8)
#  define INT_MAX 0x7fffffffffffffffL  /* 2^64 - 1 */
# else
#  error "Can't define INT_MAX"
# endif
#endif

/* Long limit
 */
#ifndef LONG_MAX
# if (SIZEOF_LONG == 4)
#  define LONG_MAX 0x7fffffffL
# elif (SIZEOF_LONG == 8)
#  define LONG_MAX 0x7fffffffffffffffL
# else
#  error "Can't define LONG_MAX"
# endif
#endif

/* time_t limit
 */
#ifndef TIME_MAX
# if (SIZEOF_TIME_T == SIZEOF_INT)
#  define TIME_MAX ((time_t)INT_MAX)
# elif (SIZEOF_TIME_T == SIZEOF_LONG)
#  define TIME_MAX ((time_t)LONG_MAX)
# else
#  error "Can't define TIME_MAX"
# endif
#endif

/* Missing constants
 */
#ifndef O_BINARY
# define O_BINARY 0
#endif

/* Pthread
 */
#define CHULA_MUTEX_T(n)           pthread_mutex_t n
#define CHULA_RWLOCK_T(n)          pthread_rwlock_t n
#define CHULA_THREAD_JOIN(t)       pthread_join(t,NULL)
#define CHULA_THREAD_KILL(t,s)     pthread_kill(t,s)
#define CHULA_THREAD_SELF          pthread_self()

#define CHULA_THREAD_PROP_GET(p)   pthread_getspecific(p)
#define CHULA_THREAD_PROP_SET(p,v) pthread_setspecific(p,v)
#define CHULA_THREAD_PROP_NEW(p,f) pthread_key_create2(p,f)

#define CHULA_MUTEX_LOCK(m)        pthread_mutex_lock(m)
#define CHULA_MUTEX_UNLOCK(m)      pthread_mutex_unlock(m)
#define CHULA_MUTEX_INIT(m,n)      pthread_mutex_init(m,n)
#define CHULA_MUTEX_DESTROY(m)     pthread_mutex_destroy(m)
#define CHULA_MUTEX_TRY_LOCK(m)    pthread_mutex_trylock(m)

#define CHULA_RWLOCK_INIT(m,n)     do {				          \
		                         memset (m, 0, sizeof(pthread_rwlock_t)); \
					 pthread_rwlock_init(m,n);                \
                                       } while(0)
#define CHULA_RWLOCK_READER(m)     pthread_rwlock_rdlock(m)
#define CHULA_RWLOCK_WRITER(m)     pthread_rwlock_wrlock(m)
#define CHULA_RWLOCK_TRYREADER(m)  pthread_rwlock_tryrdlock(m)
#define CHULA_RWLOCK_TRYWRITER(m)  pthread_rwlock_trywrlock(m)
#define CHULA_RWLOCK_UNLOCK(m)     pthread_rwlock_unlock(m)
#define CHULA_RWLOCK_DESTROY(m)    pthread_rwlock_destroy(m)

#ifdef HAVE_SCHED_YIELD
# define CHULA_THREAD_YIELD         sched_yield()
#else
# define CHULA_THREAD_YIELD
#endif

#endif /* CHULA_COMMON_INTERNAL_H */
