/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/* All files in libchula are Copyright (C) 2014 Alvaro Lopez Ortega.
 *
 *   Authors:
 *     * Alvaro Lopez Ortega <alvaro@gnu.org>
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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <libchula-qa/libchula-qa.h>
#include <libchula-qa/testing_macros-internal.h>


static void
do_md5 (chula_buffer_t *buf, size_t size)
{
    ret_t ret;

    chula_buffer_add_str (buf, " ");
    chula_buffer_multiply (buf, size);

    ret = chula_buffer_encode_md5_digest (buf);
    ck_assert (ret == ret_ok);
}

START_TEST (_64Kb)
{
    chula_buffer_t buf = CHULA_BUF_INIT;

    do_md5 (&buf, 64*1024);
    ch_assert_str_eq (buf.buf, "c858ba08b3d55c2e7e2f31a7412a7bd5");

    chula_buffer_mrproper (&buf);
}
END_TEST

START_TEST (_1Mb)
{
    chula_buffer_t buf = CHULA_BUF_INIT;

    do_md5 (&buf, 1024*1024);
    ch_assert_str_eq (buf.buf, "c178bdb56a620d47b0e0b167c665c873");

    chula_buffer_mrproper (&buf);
}
END_TEST


int
md5_tests (void)
{
    Suite *s1 = suite_create("MD5");
    check_add (s1, _64Kb);
    check_add (s1, _1Mb);
    run_test (s1);
}
