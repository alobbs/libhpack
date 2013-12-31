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

#include <check.h>
#include "libchula/buffer.h"

#define check_add(suit,func)                             \
    TCase *testcase_ ## func = tcase_create(#func);      \
    suite_add_tcase (suit, testcase_ ## func);           \
    tcase_add_test (testcase_ ##func, func);

#define run_test(suit)                          \
    SRunner *sr = srunner_create(suit);         \
    srunner_run_all(sr, CK_VERBOSE);            \
    return srunner_ntests_failed(sr);


START_TEST (init_heap)
{
    chula_buffer_t buf;

    /* Init */
    chula_buffer_init (&buf);
    ck_assert (buf.len == 0);
    ck_assert (buf.size == 0);
    ck_assert (buf.buf == NULL);

    /* Not empty */
    chula_buffer_add_str (&buf, "test");
    ck_assert (buf.len > 0);
    ck_assert (buf.buf != NULL);

    /* Empty */
    chula_buffer_clean (&buf);
    ck_assert (buf.len == 0);
    ck_assert (buf.buf != NULL);

    /* Free memory */
    chula_buffer_mrproper (&buf);
    ck_assert (buf.buf == NULL);
}
END_TEST


START_TEST (init_ptr)
{
    chula_buffer_t *buf = NULL;

    /* Instance */
    chula_buffer_new (&buf);
    ck_assert (buf != NULL);

    /* Emptiness */
    ck_assert (buf->len == 0);
    ck_assert (buf->buf == NULL);

    /* Not empty */
    chula_buffer_add_str (buf, "test");
    ck_assert (buf->len > 0);
    ck_assert (buf->buf != NULL);

    /* Empty */
    chula_buffer_clean (buf);
    ck_assert (buf->len == 0);
    ck_assert (buf->buf != NULL);
}
END_TEST

START_TEST (dup)
{
    ret_t           ret;
    chula_buffer_t  b1   = CHULA_BUF_INIT;
    chula_buffer_t *b2   = NULL;

    chula_buffer_add_str (&b1, "testing");
    ret = chula_buffer_dup (&b1, &b2);

    ck_assert (ret == ret_ok);
    ck_assert (&b1 != b2);
    ck_assert (b1.len == b2->len);
    ck_assert (b1.buf != b2->buf);
}
END_TEST

int
buffer_tests (void)
{
    Suite *s1 = suite_create("Buffer");
    check_add (s1, init_heap);
    check_add (s1, init_ptr);
    check_add (s1, dup);
    run_test (s1);
}

int
main (void)
{
    int ret;

    ret = buffer_tests();

    return ret;
}
