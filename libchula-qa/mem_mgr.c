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

#include "config.h"
#include "mem_mgr.h"

#include <unistd.h>

#ifdef HAVE_MALLOC_MALLOC_H
# include <malloc/malloc.h>
#elif defined (HAVE_MALLOC_H)
# include <malloc.h>
#endif

#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif

/* ANSI colors
 */
#define ESC   "\x1b["
#define RESET ESC "0m"

#define red(s)   ESC "0;31m" s RESET
#define green(s) ESC "0;32m" s RESET


/* Memory Manager
 */

static chula_mem_policy_t *current_policy  = NULL;
static chula_mem_mgr_t    *current_manager = NULL;

ret_t
chula_mem_mgr_init (chula_mem_mgr_t *mgr)
{
    /* Properties */
    mgr->frozen = false;

#ifdef HAVE_MALLOC_DEFAULT_ZONE
    malloc_zone_t *zone = malloc_default_zone();

    /* Store original functions */
    mgr->system.malloc  = zone->malloc;
    mgr->system.realloc = zone->realloc;
    mgr->system.free    = zone->free;

#elif defined(HAVE_MALLOC_HOOK)
    /* Store original hook functions */
    mgr->system.malloc  = __malloc_hook;
    mgr->system.realloc = __realloc_hook;
    mgr->system.free    = __free_hook;
#endif

   return ret_ok;
}

ret_t
chula_mem_mgr_mrproper (chula_mem_mgr_t *mgr)
{
    UNUSED(mgr);
    return ret_ok;
}

#ifdef HAVE_MALLOC_DEFAULT_ZONE
# define FUNC_MALLOC(name)  static void *_ ## name ## _malloc  (struct _malloc_zone_t *zone, size_t size)
# define FUNC_REALLOC(name) static void *_ ## name ## _realloc (struct _malloc_zone_t *zone, void *ptr, size_t size)
# define FUNC_FREE(name)    static void  _ ## name ## _free    (struct _malloc_zone_t *zone, void *ptr)
# define CALL_MALLOC                                                    \
    ({                                                                  \
        void* (*orig)(struct _malloc_zone_t *, size_t ) =               \
            current_manager->system.malloc;                             \
        orig (zone, size);                                              \
    })
# define CALL_REALLOC                                                   \
    ({                                                                  \
        void * (*orig)(struct _malloc_zone_t *, void *, size_t) =       \
            current_manager->system.realloc;                            \
        orig (zone, ptr, size);                                         \
    })
# define CALL_FREE                                                      \
    ({                                                                  \
        void (*orig)(struct _malloc_zone_t *, void *) =                 \
            current_manager->system.free;                               \
        orig (zone, ptr);                                               \
    })

#elif defined(HAVE_MALLOC_HOOK)
# define FUNC_MALLOC(name)  static void *_ ## name ## _malloc  (size_t size, const void *caller)
# define FUNC_REALLOC(name) static void *_ ## name ## _realloc (void *ptr, size_t size, const void *caller)
# define FUNC_FREE(name)    static void  _ ## name ## _free    (void *ptr, const void *caller)
# define CALL_MALLOC                                                    \
    ({                                                                  \
        void* (*orig)(size_t, const void *) =                           \
            current_manager->system.malloc;                             \
        orig (size, caller);                                            \
    })
# define CALL_REALLOC                                                   \
    ({                                                                  \
        void * (*orig)(void *, size_t, const void *) =                  \
            current_manager->system.realloc;                            \
        orig (ptr, size, caller);                                       \
    })
# define CALL_FREE                                                      \
    ({                                                                  \
        void (*orig)(void *, const void *) =                            \
            current_manager->system.free;                               \
        orig (ptr, caller);                                             \
    })
#endif

#define PRINT(...) chula_mem_mgr_work (current_manager, printf(__VA_ARGS__))

ret_t
chula_mem_mgr_set_policy (chula_mem_mgr_t    *mgr,
                          chula_mem_policy_t *policy)
{
    /* Assign globals */
    current_manager = mgr;
    current_policy  = policy;

    /* Set new functions (Darwin)
     */
#ifdef HAVE_MALLOC_DEFAULT_ZONE
    int            re;
    malloc_zone_t *zone = malloc_default_zone();

    re = mprotect (zone, getpagesize(), PROT_READ|PROT_WRITE);
    if (re != 0) return ret_error;

    zone->malloc  = policy->malloc;
    zone->realloc = policy->realloc;
    zone->free    = policy->free;

    /* Set new functions (Linux)
     */
#elif defined(HAVE_MALLOC_HOOK)
    __malloc_hook  = policy->malloc;
    __realloc_hook = policy->realloc;
    __free_hook    = policy->free;
#endif

    return ret_ok;
}

ret_t
chula_mem_mgr_reset (chula_mem_mgr_t *mgr)
{
    return chula_mem_mgr_set_policy (mgr, &mgr->system);
}

ret_t
chula_mem_mgr_freeze (chula_mem_mgr_t *mgr)
{
    mgr->frozen = true;
    return ret_ok;
}

ret_t
chula_mem_mgr_thaw (chula_mem_mgr_t *mgr)
{
    mgr->frozen = false;
    return ret_ok;
}


/* Policy
 */

ret_t
chula_mem_policy_init (chula_mem_policy_t *policy)
{
    policy->malloc  = NULL;
    policy->realloc = NULL;
    policy->free    = NULL;
    return ret_ok;
}

ret_t
chula_mem_policy_mrproper (chula_mem_policy_t *policy)
{
    UNUSED(policy);
    return ret_ok;
}


/* Random Failures Memory Policy
 */


FUNC_MALLOC (random)
{
    chula_mem_policy_random_t *policy = (chula_mem_policy_random_t *)current_policy;

    if (! current_manager->frozen) {
        if ((chula_random() & 0xff) <= (0xff * policy->failure_rate)) {
            return NULL;
        }
    }

    return CALL_MALLOC;
}

FUNC_REALLOC (random)
{
    chula_mem_policy_random_t *policy = (chula_mem_policy_random_t *)current_policy;

    if (! current_manager->frozen) {
        if ((chula_random() & 0xff) <= (0xff * policy->failure_rate)) {
            return NULL;
        }
    }

    return CALL_REALLOC;
}

FUNC_FREE (random)
{
    CALL_FREE;
}

ret_t
chula_mem_policy_random_init (chula_mem_policy_random_t *policy,
                              float                      rate)
{
    chula_mem_policy_init (&policy->base);

    policy->base.malloc  = _random_malloc;
    policy->base.realloc = _random_realloc;
    policy->base.free    = _random_free;
    policy->failure_rate = rate;

    return ret_ok;
}

ret_t
chula_mem_policy_random_mrproper (chula_mem_policy_random_t *policy)
{
    return chula_mem_policy_mrproper (&policy->base);
}


/* Counter Memory Policy
 */

FUNC_MALLOC (counter)
{
    chula_mem_policy_counter_t *policy = (chula_mem_policy_counter_t *)current_policy;

    if (! current_manager->frozen) {
        policy->n_malloc++;
    }

    return CALL_MALLOC;
}

FUNC_REALLOC (counter)
{
    chula_mem_policy_counter_t *policy = (chula_mem_policy_counter_t *)current_policy;

    if (! current_manager->frozen) {
        policy->n_realloc++;
    }

    return CALL_REALLOC;
}

FUNC_FREE (counter)
{
    chula_mem_policy_counter_t *policy = (chula_mem_policy_counter_t *)current_policy;

    if (! current_manager->frozen) {
        policy->n_free++;
    }

    CALL_FREE;
}

ret_t
chula_mem_policy_counter_init (chula_mem_policy_counter_t *polcnt)
{
    chula_mem_policy_init (&polcnt->base);

    polcnt->n_malloc     = 0;
    polcnt->n_realloc    = 0;
    polcnt->n_free       = 0;
    polcnt->base.malloc  = _counter_malloc;
    polcnt->base.realloc = _counter_realloc;
    polcnt->base.free    = _counter_free;

    return ret_ok;
}

ret_t
chula_mem_policy_counter_mrproper (chula_mem_policy_counter_t *polcnt)
{
    return chula_mem_policy_mrproper (&polcnt->base);
}


/* Scheduled Failure Memory Policy
 */

FUNC_MALLOC (after)
{
    chula_mem_policy_sched_fail_t *policy = (chula_mem_policy_sched_fail_t *)current_policy;

    if (! current_manager->frozen) {
        policy->counter++;
        if (policy->counter > policy->fail_after) {
            PRINT (red("M"));
            return NULL;
        }
        PRINT (green("M"));
    }

    return CALL_MALLOC;
}

FUNC_REALLOC (after)
{
    chula_mem_policy_sched_fail_t *policy = (chula_mem_policy_sched_fail_t *)current_policy;

    if (! current_manager->frozen) {
        policy->counter++;
        if (policy->counter > policy->fail_after) {
            PRINT (red("R"));
            return NULL;
        }
        PRINT (green("R"));
    }

    return CALL_REALLOC;
}

FUNC_FREE (after)
{
    chula_mem_policy_sched_fail_t *policy = (chula_mem_policy_sched_fail_t *)current_policy;

    if (! current_manager->frozen) {
        policy->counter++;
    }

    CALL_FREE;
}


ret_t
chula_mem_policy_sched_fail_init (chula_mem_policy_sched_fail_t *polschd, uint32_t fail_after)
{
    chula_mem_policy_init (&polschd->base);

    polschd->counter      = 0;
    polschd->fail_after   = fail_after;

    polschd->base.malloc  = _after_malloc;
    polschd->base.realloc = _after_realloc;
    polschd->base.free    = _after_free;

    return ret_ok;
}

ret_t
chula_mem_policy_sched_fail_mrproper (chula_mem_policy_sched_fail_t *polschd)
{
    return chula_mem_policy_mrproper (&polschd->base);
}
