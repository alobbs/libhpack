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
#include "list.h"
#include "buffer.h"

#define SCHED_FAIL(func)                        \
    do {                                        \
        PRINT ("* %s\n", #func);                \
        fflush (stdout);                        \
        re += exec_sched_fail(func);            \
        PRINT ("\n");                           \
    } while (false)


int
main (int argc, char *argv[])
{
    int re = 0;

    UNUSED(argc);
    UNUSED(argv);

    /* Init */
    chula_mem_mgr_init (&mgr);

    /* Tests */
    SCHED_FAIL (buffer_new);
    SCHED_FAIL (buffer_add);
    SCHED_FAIL (buffer_dup);
    SCHED_FAIL (buffer_operations);
    SCHED_FAIL (buffer_encoders);
    SCHED_FAIL (buffer_repr);

    SCHED_FAIL (list_add);

    /* Clean up */
    chula_mem_mgr_reset(&mgr);
    chula_mem_mgr_mrproper (&mgr);

    printf ("Everything did work\n");
    return re;
}
