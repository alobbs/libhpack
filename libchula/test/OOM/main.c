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

#include <libchula/libchula.h>
#include <assert.h>

int
buffer_tests (void)
{
    ret_t                     ret;
    chula_mem_mgr_t           mgr;
    chula_mem_policy_random_t policy;

    /* Set a custom mem-mgr */
    chula_mem_mgr_init (&mgr);
    chula_mem_policy_random_init (&policy, 0.01);
    chula_mem_mgr_set_policy (&mgr, MEM_POLICY(&policy));

    /* Testing */
    for (uint32_t i=0; i<1000; i++) {
        chula_buffer_t *buf = NULL;

        ret = chula_buffer_new (&buf);
        assert ((ret == ret_ok) || (ret == ret_nomem));

        chula_buffer_free(buf);
    }

    /* Restore mem-mgr */
    chula_mem_mgr_reset(&mgr);

    /* Clean up */
    chula_mem_mgr_mrproper (&mgr);
    chula_mem_policy_random_mrproper (&policy);

    printf ("Everything did work\n");
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
    re += buffer_tests();

    return re;
}
