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

#include "oom.h"

chula_mem_mgr_t mgr;

int
exec_sched_fail (int (*test)(void))
{
    int                           re = 0;
    uint32_t                      total_calls;
    chula_mem_policy_counter_t    policy_c;
    chula_mem_policy_sched_fail_t policy_f;

    /* Count */
    chula_mem_policy_counter_init (&policy_c);
    chula_mem_mgr_set_policy (&mgr, MEM_POLICY(&policy_c));
    test();
    total_calls = policy_c.n_malloc + policy_c.n_realloc + policy_c.n_free;
    chula_mem_policy_counter_mrproper (&policy_c);

    /* Execute: Fail after N */
    PRINT ("Permanent Fail:\n");
    for (uint32_t n=0; n<=total_calls; n++) {
        PRINT ("\t% 3d of % 3d: ", n, total_calls);
        chula_mem_policy_sched_fail_init (&policy_f, n, INT_MAX);
        chula_mem_mgr_set_policy (&mgr, MEM_POLICY(&policy_f));
        re += test();
        PRINT ("\n");
    }

    chula_mem_mgr_reset (&mgr);

    /* Execute: Fail and recover after N */
    uint32_t total   = 0;
    uint32_t current = 1;

    PRINT ("Temporary Fail & Recover:\n");

    for (uint32_t n=0; n<=total_calls; n++)
        for (uint32_t l=1; l<=total_calls-n; l++)
            total++;

    for (uint32_t n=0; n<=total_calls; n++) {
        for (uint32_t l=1; l<=total_calls-n; l++) {
            PRINT ("\t% 3d of % 3d: ", current++, total);
            chula_mem_policy_sched_fail_init (&policy_f, n, l);
            chula_mem_mgr_set_policy (&mgr, MEM_POLICY(&policy_f));
            re += test();
            PRINT ("\n");
        }
    }

    /* Clean up */
    chula_mem_policy_counter_mrproper (&policy_c);
    chula_mem_policy_sched_fail_mrproper (&policy_f);
    return re;
}
