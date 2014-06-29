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

#ifndef CHULA_MEM_MGR_H
#define CHULA_MEM_MGR_H

#if !defined(CHULA_QA_H_INSIDE) && !defined (CHULA_QA_COMPILATION)
# error "Only <libchula-qa/libchula-qa.h> can be included directly."
#endif

#include <libchula/libchula.h>

typedef struct {
    void *malloc;
    void *realloc;
    void *free;
} chula_mem_policy_t;

typedef struct {
    chula_mem_policy_t base;
    float              failure_rate;
} chula_mem_policy_random_t;

typedef struct {
    chula_mem_policy_t base;
    uint32_t           n_malloc;
    uint32_t           n_realloc;
    uint32_t           n_free;
} chula_mem_policy_counter_t;

typedef struct {
    chula_mem_policy_t base;
    uint32_t           counter;
    uint32_t           fail_after;
    uint32_t           recover_after;
} chula_mem_policy_sched_fail_t;

typedef struct {
    chula_mem_policy_t system;
    bool               frozen;
} chula_mem_mgr_t;

#define MEM_MGR(m)    ((chula_mem_mgr_t *)(m))
#define MEM_POLICY(m) ((chula_mem_policy_t *)(m))

/* Memory Policy */
ret_t chula_mem_policy_init     (chula_mem_policy_t *policy);
ret_t chula_mem_policy_mrproper (chula_mem_policy_t *policy);

/* Memory Policy: Random Failures */
ret_t chula_mem_policy_random_init     (chula_mem_policy_random_t *polran, float rate);
ret_t chula_mem_policy_random_mrproper (chula_mem_policy_random_t *polran);

/* Memory Policy: Counter */
ret_t chula_mem_policy_counter_init     (chula_mem_policy_counter_t *polcnt);
ret_t chula_mem_policy_counter_mrproper (chula_mem_policy_counter_t *polcnt);

/* Memory Policy: Scheduled Failure */
ret_t chula_mem_policy_sched_fail_init     (chula_mem_policy_sched_fail_t *polschd, uint32_t fail_after, uint32_t recover_after);
ret_t chula_mem_policy_sched_fail_mrproper (chula_mem_policy_sched_fail_t *polschd);

/* Memory Manager */
ret_t chula_mem_mgr_init       (chula_mem_mgr_t *mgr);
ret_t chula_mem_mgr_mrproper   (chula_mem_mgr_t *mgr);

ret_t chula_mem_mgr_reset      (chula_mem_mgr_t *mgr);
ret_t chula_mem_mgr_set_policy (chula_mem_mgr_t *mgr, chula_mem_policy_t *policy);
ret_t chula_mem_mgr_freeze     (chula_mem_mgr_t *mgr);
ret_t chula_mem_mgr_thaw       (chula_mem_mgr_t *mgr);

#define chula_mem_mgr_work(mgr,cmd)             \
    do {                                        \
        chula_mem_mgr_freeze (mgr);             \
        cmd;                                    \
        chula_mem_mgr_thaw (mgr);               \
    } while(false)

#endif /* CHULA_MEM_MGR_H */
