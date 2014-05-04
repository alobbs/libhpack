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

#include "common-internal.h"
#include "mem_mgr.h"
#include "log.h"
#include "cstrings.h"

#include <unistd.h>

#ifdef HAVE_MALLOC_MALLOC_H
# include <malloc/malloc.h>
#endif

#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif


/* Memory Manager
 */

static chula_mem_policy_t *current_policy  = NULL;
static chula_mem_mgr_t    *current_manager = NULL;

ret_t
chula_mem_mgr_init (chula_mem_mgr_t *mgr)
{
#ifdef HAVE_MALLOC_DEFAULT_ZONE
    int            re;
    malloc_zone_t *zone = malloc_default_zone();

    /* Store original functions */
    mgr->system.malloc  = zone->malloc;
    mgr->system.realloc = zone->realloc;
    mgr->system.free    = zone->free;
#endif

   return ret_ok;
}

ret_t
chula_mem_mgr_mrproper (chula_mem_mgr_t *mgr)
{
    return ret_ok;
}

ret_t
chula_mem_mgr_set_policy (chula_mem_mgr_t    *mgr,
                          chula_mem_policy_t *policy)
{
#ifdef HAVE_MALLOC_DEFAULT_ZONE
    int            re;
    malloc_zone_t *zone = malloc_default_zone();

    /* Set new functions */
    re = mprotect (zone, getpagesize(), PROT_READ|PROT_WRITE);
    if (re != 0) return ret_error;

    zone->malloc  = policy->malloc;
    zone->realloc = policy->realloc;
    zone->free    = policy->free;

    current_manager = mgr;
    current_policy  = policy;
#endif

    return ret_ok;
}

ret_t
chula_mem_mgr_reset (chula_mem_mgr_t *mgr)
{
    return chula_mem_mgr_set_policy (mgr, &mgr->system);
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
    return ret_ok;
}


/* Random Failures Memory Policy
 */

static void *
_random_malloc (struct _malloc_zone_t *zone, size_t size)
{
    void * (*orig)(struct _malloc_zone_t *zone, size_t size) = current_manager->system.malloc;
    return orig (zone, size);
}

static void *
_random_realloc (struct _malloc_zone_t *zone, void *ptr, size_t size)
{
    void * (*orig)(struct _malloc_zone_t *zone, void *ptr, size_t size) = current_manager->system.realloc;
    return orig (zone, ptr, size);
}

static void
_random_free (struct _malloc_zone_t *zone, void *ptr)
{
    void * (*orig)(struct _malloc_zone_t *zone, void *ptr) = current_manager->system.free;
    orig (zone, ptr);
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


/*
    chula_mem_mgr_t           mgr;
    chula_mem_policy_random_t policy;

    chula_mem_policy_random_init (&policy, 0.5);

    chula_mem_mgr_init (&mgr);
    chula_mem_mgr_set_policy (&mgr, MEM_POLICY(&policy));
 */
