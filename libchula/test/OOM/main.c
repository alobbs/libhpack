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

#include <libchula-qa/libchula-qa.h>
#include <assert.h>

static chula_mem_mgr_t mgr;

#define WORK(...)  chula_mem_mgr_work (&mgr, __VA_ARGS__)
#define PRINT(...) chula_mem_mgr_work (&mgr, printf(__VA_ARGS__))

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
    PRINT ("policy_c.n_malloc  %d\n", policy_c.n_malloc);
    PRINT ("policy_c.n_realloc %d\n", policy_c.n_realloc);
    PRINT ("policy_c.n_free    %d\n", policy_c.n_free);
    total_calls = policy_c.n_malloc + policy_c.n_realloc + policy_c.n_free;
    PRINT ("total_calls %d\n", total_calls);
    chula_mem_policy_counter_mrproper (&policy_c);

    /* Execute */
    for (uint32_t n=0; n<total_calls; n++) {
        printf ("n = %d\n", n);
        chula_mem_policy_sched_fail_init (&policy_f, n);
        chula_mem_mgr_set_policy (&mgr, MEM_POLICY(&policy_f));
        re += test();
    }

    /* Clean up */
    chula_mem_policy_counter_mrproper (&policy_c);
    chula_mem_policy_sched_fail_mrproper (&policy_f);
    return re;
}

int
buffer_new (void)
{
    ret_t           ret;
    chula_buffer_t *buf  = NULL;

    ret = chula_buffer_new (&buf);
    assert (((ret == ret_ok) && (buf != NULL)) ||
            ((ret == ret_nomem) && (buf == NULL)));

    chula_buffer_free(buf);
    return 0;
}

int
buffer_add_str (void)
{
    ret_t           ret;
    uint32_t        len;
    chula_buffer_t *buf  = NULL;

    WORK(chula_buffer_new (&buf));

    len = buf->len;
    ret = chula_buffer_add_str (buf, "testing");
    assert (((ret == ret_ok) && (buf->len > len)) ||
            ((ret == ret_nomem) && (buf->len <= len)));

    chula_buffer_free(buf);
    return 0;
}

int
main (int argc, char *argv[])
{
    int re = 0;

    UNUSED(argc);
    UNUSED(argv);

    /* It'll need random numbers */
    chula_random_seed();

    /* Tests */
    chula_mem_mgr_init (&mgr);
    re += exec_sched_fail(buffer_new);
    re += exec_sched_fail(buffer_add_str);

    /* Clean up */
    chula_mem_mgr_reset(&mgr);
    chula_mem_mgr_mrproper (&mgr);

    printf ("Everything did work\n");
    return re;
}
