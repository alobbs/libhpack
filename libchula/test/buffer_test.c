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

#include <check.h>
#include "libchula/buffer.h"

#define check_add(suit,func)                             \
    TCase *testcase_ ## func = tcase_create(#func);      \
    suite_add_tcase (suit, testcase_ ## func);           \
    tcase_add_test (testcase_ ##func, func);

#define run_test(suit)                          \
    SRunner *sr = srunner_create (suit);        \
    srunner_set_fork_status (sr, CK_NOFORK);    \
    srunner_run_all (sr, CK_VERBOSE);           \
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

START_TEST (dup_)
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

START_TEST (add)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_add (&b, "123", 3);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 3);

    ret = chula_buffer_add (&b, "abcd", 4);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 3+4);

    ret = chula_buffer_add (&b, "", 0);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 3+4);
}
END_TEST

START_TEST (slice)
{
    ret_t           ret;
    chula_buffer_t  a    = CHULA_BUF_INIT;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    chula_buffer_add_str (&a, "0123456789");

    /* [2:4] */
    ret = chula_buffer_add_buffer_slice (&b, &a, 2, 4);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 2);
    ck_assert_str_eq (b.buf, "23");

    /* [2:] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, 2, CHULA_BUF_SLIDE_NONE);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 8);
    ck_assert_str_eq (b.buf, "23456789");

    /* [-2:] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, -2, CHULA_BUF_SLIDE_NONE);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 2);
    ck_assert_str_eq (b.buf, "89");

    /* [:4] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, CHULA_BUF_SLIDE_NONE, 4);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 4);
    ck_assert_str_eq (b.buf, "0123");

    /* [:-4] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, CHULA_BUF_SLIDE_NONE, -4);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 6);
    ck_assert_str_eq (b.buf, "012345");

    /* [:] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, CHULA_BUF_SLIDE_NONE, CHULA_BUF_SLIDE_NONE);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 10);
    ck_assert_str_eq (b.buf, "0123456789");

    /* [0:0] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, 0, 0);
    ck_assert (ret == ret_ok);
     ck_assert (b.len == 0);
}
END_TEST

START_TEST (add_fsize)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_fsize (&b, 0);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "0");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 900);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "900");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 1025);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "1.0K");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 2048);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "2.0K");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, (2*1024) + 300);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "2.3K");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 1024*1024);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "1.0M");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 9.5*1024*1024);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "9.5M");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 2.1 * 1024*1024*1024);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "2.1G");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 9.9 * 1024*1024*1024*1024);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "9.9T");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 6.2 * 1024*1024*1024*1024*1024);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "6.2P");
}
END_TEST

START_TEST (add_long10)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_long10 (&b, 1l);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "1");

    /* LONG_MIN: -2^31+1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_long10 (&b, -2147483647);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "-2147483647");

    /* LONG_MAX: 2^31-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_long10 (&b, 2147483647);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "2147483647");

    /* ULONG_MAX: 2^32-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_ulong10 (&b, 4294967295);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "4294967295");
}
END_TEST

START_TEST (add_llong10)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_llong10 (&b, 0);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "0");

    /* LLONG_MIN: -2^63+1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_llong10 (&b, -9223372036854775807);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "-9223372036854775807");

    /* LLONG_MAX: 2^63-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_llong10 (&b, 9223372036854775807);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "9223372036854775807");

    /* ULLONG_MAX: 2^64-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_ullong10 (&b, 18446744073709551615ull);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "18446744073709551615");
}
END_TEST

START_TEST (add_long16)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_ulong16 (&b, 1);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "1");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_ulong16 (&b, 10);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "a");

    /* LONG_MAX: 2^31-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_ulong16 (&b, 2147483647);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "7fffffff");

    /* ULONG_MAX: 2^32-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_ulong16 (&b, 4294967295);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "ffffffff");
}
END_TEST

START_TEST (add_llong16)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_ullong16 (&b, 0);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "0");

    /* LLONG_MAX: 2^63-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_ullong16 (&b, 9223372036854775807ull);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "7fffffffffffffff");

    /* ULLONG_MAX: 2^64-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_ullong16 (&b, 18446744073709551615ull);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "ffffffffffffffff");
}
END_TEST

START_TEST (add_va)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_va (&b, "");
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);

    chula_buffer_clean (&b);
    ret = chula_buffer_add_va (&b, "hi there");
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "hi there");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_va (&b, "%d is not one", 0);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "0 is not one");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_va (&b, "%03d rocks", 7);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "007 rocks");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_va (&b, "%s %%s", "tricky");
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "tricky %s");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_va (&b, "%d - %1.1f - %s", 1, 2.1, "foo");
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "1 - 2.1 - foo");
}
END_TEST

START_TEST (add_char)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_char (&b, 'a');
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 1);
    ck_assert_str_eq (b.buf, "a");

    ret = chula_buffer_add_char (&b, 'b');
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 2);
    ck_assert_str_eq (b.buf, "ab");
}
END_TEST

START_TEST (add_char_n)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_char_n (&b, 'a', -1);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);
    ck_assert (b.buf == NULL);

    ret = chula_buffer_add_char_n (&b, 'a', 0);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);
    ck_assert (b.buf == NULL);

    ret = chula_buffer_add_char_n (&b, 'a', 5);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 5);
    ck_assert_str_eq (b.buf, "aaaaa");
}
END_TEST

START_TEST (prepend)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_prepend (&b, "", 0);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);
    ck_assert (b.buf == NULL);

    ret = chula_buffer_prepend (&b, "!", 1);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 1);
    ck_assert_str_eq (b.buf, "!");

    ret = chula_buffer_prepend (&b, " there", 6);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 7);
    ck_assert_str_eq (b.buf, " there!");

    ret = chula_buffer_prepend (&b, "Hi", 2);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 9);
    ck_assert_str_eq (b.buf, "Hi there!");

    chula_buffer_clean (&b);
    ret = chula_buffer_prepend (&b, "Allocated", 9);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 9);
    ck_assert_str_eq (b.buf, "Allocated");
}
END_TEST

START_TEST (is_ending)
{
    int             re;
    chula_buffer_t  b   = CHULA_BUF_INIT;

    re = chula_buffer_is_ending (&b, ' ');
    ck_assert (re == 0);

    re = chula_buffer_is_ending (&b, 'a');
    ck_assert (re == 0);

    chula_buffer_add_str (&b, "12345");

    re = chula_buffer_is_ending (&b, '4');
    ck_assert (re == 0);

    re = chula_buffer_is_ending (&b, '5');
    ck_assert (re == 1);
}
END_TEST

START_TEST (move_to_begin)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    chula_buffer_add_str (&b, "0123456789");
    ret = chula_buffer_move_to_begin (&b, 11);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);

    chula_buffer_add_str (&b, "0123456789");
    ret = chula_buffer_move_to_begin (&b, 0);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "0123456789");

    ret = chula_buffer_move_to_begin (&b, 3);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "3456789");
}
END_TEST

START_TEST (ensure_size)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ck_assert (b.buf == NULL);
    ck_assert (b.size == 0);

    ret = chula_buffer_ensure_size (&b, 100);
    ck_assert (ret == ret_ok);
    ck_assert (b.buf != NULL);
    ck_assert (b.size == 100);

    ret = chula_buffer_ensure_size (&b, 99);
    ck_assert (ret == ret_ok);
    ck_assert (b.buf != NULL);
    ck_assert (b.size == 100);

    ret = chula_buffer_ensure_size (&b, 200);
    ck_assert (ret == ret_ok);
    ck_assert (b.buf != NULL);
    ck_assert (b.size == 200);
}
END_TEST

START_TEST (drop_ending)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    chula_buffer_add_str (&b, "0123456789");

    ret = chula_buffer_drop_ending (&b, 0);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 10);
    ck_assert_str_eq (b.buf, "0123456789");

    ret = chula_buffer_drop_ending (&b, 2);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 8);
    ck_assert_str_eq (b.buf, "01234567");

    ret = chula_buffer_drop_ending (&b, 999);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);
}
END_TEST

START_TEST (swap_chars)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    chula_buffer_add_str (&b, "0123401234");

    ret = chula_buffer_swap_chars (&b, '2', '*');
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 10);
    ck_assert_str_eq (b.buf, "01*3401*34");

    ret = chula_buffer_swap_chars (&b, 'Z', '$');
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 10);
    ck_assert_str_eq (b.buf, "01*3401*34");

    chula_buffer_clean (&b);
    ret = chula_buffer_swap_chars (&b, 'Z', '$');
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);
}
END_TEST

START_TEST (remove_dups)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_remove_dups (&b, 'Z');
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);

    chula_buffer_add_str (&b, "abcccde");
    ret = chula_buffer_remove_dups (&b, 'c');
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "abcde");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "aaaaaaa");
    ret = chula_buffer_remove_dups (&b, 'a');
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 1);
    ck_assert_str_eq (b.buf, "a");
}
END_TEST

START_TEST (remove_string)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_remove_string (&b, NULL, 0);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);

    chula_buffer_add_str (&b, "hola hola caracola");
    ret = chula_buffer_remove_string (&b, "hola ", 5);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "caracola");

    ret = chula_buffer_remove_string (&b, "bye", 3);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "caracola");

    ret = chula_buffer_remove_string (&b, "raco", 0);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "caracola");
}
END_TEST

START_TEST (remove_chunk)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_remove_chunk (&b, 0, 0);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);
    ck_assert (b.buf == NULL);

    chula_buffer_add_str (&b, "0123456789");
    ret = chula_buffer_remove_chunk (&b, 11, 2);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 10);
    ck_assert_str_eq (b.buf, "0123456789");

    ret = chula_buffer_remove_chunk (&b, 5, 4);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 6);
    ck_assert_str_eq (b.buf, "012349");

    ret = chula_buffer_remove_chunk (&b, 0, 99);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);
}
END_TEST

START_TEST (cmp_buf)
{
    int             re;
    chula_buffer_t  a   = CHULA_BUF_INIT;
    chula_buffer_t  b   = CHULA_BUF_INIT;

    re = chula_buffer_cmp_buf (&a, &b);
    ck_assert (re == 0);

    chula_buffer_add_str (&a, "a");
    re = chula_buffer_cmp_buf (&a, &b);
    ck_assert (re > 0);
    re = chula_buffer_cmp_buf (&b, &a);
    ck_assert (re < 0);

    chula_buffer_add_str (&a, "bcd");
    chula_buffer_add_str (&b, "abcd");
    re = chula_buffer_cmp_buf (&a, &b);
    ck_assert (re == 0);
    re = chula_buffer_cmp_buf (&b, &a);
    ck_assert (re == 0);
}
END_TEST

START_TEST (cmp)
{
    int             re;
    chula_buffer_t  b   = CHULA_BUF_INIT;

    re = chula_buffer_cmp (&b, NULL, 0);
    ck_assert (re == 0);

    re = chula_buffer_cmp (&b, "a", 1);
    ck_assert (re < 0);

    chula_buffer_add_str (&b, "hola");
    re = chula_buffer_cmp (&b, "a", 1);
    ck_assert (re > 0);

    re = chula_buffer_cmp (&b, "ho", 2);
    ck_assert (re > 0);

    re = chula_buffer_cmp (&b, "hola", 2);
    ck_assert (re > 0);

    re = chula_buffer_cmp (&b, "hola", 4);
    ck_assert (re == 0);
}
END_TEST

START_TEST (crc32)
{
    crc_t           crc;
    chula_buffer_t  b   = CHULA_BUF_INIT;

    /* python -c "import zlib; print zlib.crc32('')" */
    crc = chula_buffer_crc32 (&b);
    ck_assert (crc == 0);

    /* python -c "import zlib; print zlib.crc32('h')" */
    chula_buffer_add_str (&b, "h");
    crc = chula_buffer_crc32 (&b);
    ck_assert (crc == -1855256857);

    /* python -c "import zlib; print zlib.crc32('hola')" */
    chula_buffer_add_str (&b, "ola");
    crc = chula_buffer_crc32 (&b);
    ck_assert (crc == 1872820616);
}
END_TEST

START_TEST (read_file)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    /* Doesn't exists */
    ret = chula_buffer_read_file (&b, "/it/doesnt/exist");
    ck_assert (ret != ret_ok);

    /* Not a file */
    ret = chula_buffer_read_file (&b, "/");
    ck_assert (ret != ret_ok);

    /* Read an actual file */
    FILE *f        = NULL;
    char *filename = mktemp ("buffer_test");

    f = fopen (filename, "w+");
    ck_assert (f != NULL);

    fwrite ("hola", 1, 4, f);
    fclose(f);

    ret = chula_buffer_read_file (&b, filename);
    unlink (filename);

    ck_assert (ret == ret_ok);
    ck_assert (b.len == 4);
    ck_assert_str_eq (b.buf, "hola");
}
END_TEST

START_TEST (read_from_fd)
{
    int             fd;
    ret_t           ret;
    size_t          did_read = 0;
    chula_buffer_t  b        = CHULA_BUF_INIT;

    /* Doesn't exists */
    fd = open ("/it/doesnt/exist", O_RDONLY);
    ret = chula_buffer_read_from_fd (&b, fd, 0, &did_read);
    close (fd);
    ck_assert (ret == ret_error);

    /* Not a file */
    fd = open ("/", O_RDONLY);
    ret = chula_buffer_read_from_fd (&b, fd, 1, &did_read);
    close (fd);
    ck_assert (ret == ret_error);

    /* Read an actual file */
    FILE *f        = NULL;
    char *filename = mktemp ("buffer_test");

    f = fopen (filename, "w+");
    ck_assert (f != NULL);

    fwrite ("content", 1, 7, f);
    fclose(f);

    fd = open (filename, O_RDONLY);
    ret = chula_buffer_read_from_fd (&b, fd, 9999, &did_read);
    unlink (filename);

    ck_assert (ret == ret_ok);
    ck_assert (b.len == 7);
    ck_assert_str_eq (b.buf, "content");
}
END_TEST

START_TEST (multiply)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_multiply (&b, 10);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);

    chula_buffer_add_str (&b, "chula");

    ret = chula_buffer_multiply (&b, 1);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 5);

    ret = chula_buffer_multiply (&b, 3);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 15);
}
END_TEST

START_TEST (get_utf8_len)
{
    ret_t           ret;
    cuint_t         len  = 0;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_get_utf8_len (&b, &len);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);

    chula_buffer_add_str (&b, "hola");
    ret = chula_buffer_get_utf8_len (&b, &len);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 4);

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "ñáñé");
    ret = chula_buffer_get_utf8_len (&b, &len);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 8);
    ck_assert (len == 4);

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "イロハニホヘト");
    ret = chula_buffer_get_utf8_len (&b, &len);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 21);
    ck_assert (len == 7);

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "ψυχοφθόρα");
    ret = chula_buffer_get_utf8_len (&b, &len);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 18);
    ck_assert (len == 9);
}
END_TEST

START_TEST (unescape_uri)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_unescape_uri (&b);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 0);

    chula_buffer_add_str (&b, "http://host/file?key1=one&key2=two");
    ret = chula_buffer_unescape_uri(&b);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "http://host/file?key1=one&key2=two");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "http://host/with%20spaces");
    ret = chula_buffer_unescape_uri(&b);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "http://host/with spaces");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "http://host/100%25");
    ret = chula_buffer_unescape_uri(&b);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "http://host/100%");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "http://host/end%00not");
    ret = chula_buffer_unescape_uri(&b);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "http://host/end not");
}
END_TEST

START_TEST (escape_uri)
{
    ret_t           ret;
    chula_buffer_t  a    = CHULA_BUF_INIT;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_escape_uri (&b, &a);
    ck_assert (ret == ret_ok);
    ck_assert (a.len == 0);
    ck_assert (b.len == 0);

    chula_buffer_add_str (&a, "http://host/file?key1=one&key2=two");
    ret = chula_buffer_escape_uri (&b, &a);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "http://host/file%3fkey1=one&key2=two");

    chula_buffer_clean (&a);
    chula_buffer_clean (&b);
    chula_buffer_add_str (&a, "http://host/with spaces");
    ret = chula_buffer_escape_uri(&b, &a);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "http://host/with%20spaces");

    chula_buffer_clean (&a);
    chula_buffer_clean (&b);
    chula_buffer_add_str (&a, "\x00\x01\x1f\x7f\xff");
    ret = chula_buffer_escape_uri(&b, &a);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "%00%01%1f%7f%ff");
}
END_TEST

START_TEST (escape_uri_delims)
{
    ret_t           ret;
    chula_buffer_t  a    = CHULA_BUF_INIT;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_escape_uri_delims (&b, &a);
    ck_assert (ret == ret_ok);
    ck_assert (a.len == 0);
    ck_assert (b.len == 0);

    chula_buffer_add_str (&a, "hola \x00\x01\x1f :?#[]@ \x7f\xff adios!<>{}-_");
    ret = chula_buffer_escape_uri_delims(&b, &a);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "hola%20%00%01%1f%20%3a%3f%23%5b%5d%40%20%7f%ff%20adios!<>{}-_");
}
END_TEST

START_TEST (escape_arg)
{
    ret_t           ret;
    chula_buffer_t  a    = CHULA_BUF_INIT;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_escape_arg (&b, &a);
    ck_assert (ret == ret_ok);
    ck_assert (a.len == 0);
    ck_assert (b.len == 0);

    chula_buffer_add_str (&a, "hola \x00\x01\x1f #%&+? \x7f\xff adios!@[]<>");
    ret = chula_buffer_escape_arg(&b, &a);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "hola%20%00%01%1f%20%23%25%26%2b%3f%20%7f%ff%20adios!@[]<>");
}
END_TEST

START_TEST (add_escape_html)
{
    ret_t           ret;
    chula_buffer_t  a    = CHULA_BUF_INIT;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_escape_html (&b, &a);
    ck_assert (ret == ret_ok);
    ck_assert (a.len == 0);
    ck_assert (b.len == 0);

    chula_buffer_add_str (&a, "<this>");
    ret = chula_buffer_add_escape_html (&b, &a);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "&lt;this&gt;");

    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&a, "a & b < c > d \" e ' f / g # @[]{}-_$!");
    ret = chula_buffer_add_escape_html (&b, &a);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "a &amp; b &lt; c &gt; d &quot; e &#39; f &#47; g &#35; @[]{}-_$!");
}
END_TEST

START_TEST (base64)
{
    ret_t           ret;
    chula_buffer_t  a    = CHULA_BUF_INIT;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_encode_base64 (&a, &b);
    ck_assert (ret == ret_ok);
    ck_assert (a.len == 0);
    ck_assert (b.len == 0);

    ret = chula_buffer_decode_base64 (&a);
    ck_assert (ret == ret_ok);
    ck_assert (a.len == 0);

    /* Text string */
    chula_buffer_add_str (&a, "testing");
    ret = chula_buffer_encode_base64 (&a, &b);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "dGVzdGluZw==");
    ck_assert (b.len == 12);

    ret = chula_buffer_decode_base64 (&b);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "testing");
    ck_assert (b.len ==7);

    /* Binary */
    chula_buffer_clean (&a);
    chula_buffer_clean (&a);

    chula_buffer_add_str (&a, "\x00\x01\x03\x04\xfd\xfe\xff");
    ret = chula_buffer_encode_base64 (&a, &b);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (b.buf, "AAEDBP3+/w==");
    ck_assert (b.len == 12);

    ret = chula_buffer_decode_base64 (&b);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 7);
}
END_TEST

START_TEST (md5_digest)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_encode_md5_digest (&b);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 32);
    ck_assert_str_eq (b.buf, "d41d8cd98f00b204e9800998ecf8427e");

    /* Text */
    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "hola");
    ret = chula_buffer_encode_md5_digest (&b);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 32);
    ck_assert_str_eq (b.buf, "4d186321c1a7f0f354b297e8914ab240");

    /* Binary */
    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "\x00\x01\xfe\xff");
    ret = chula_buffer_encode_md5_digest (&b);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 32);
    ck_assert_str_eq (b.buf, "f129ac2d8a0ee74923012d1c8ed83a86");
}
END_TEST

START_TEST (encode_md5)
{
    ret_t           ret;
    chula_buffer_t  a    = CHULA_BUF_INIT;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_encode_md5 (&a, &b);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 16);
    ck_assert_str_eq (b.buf, "\xd4\x1d\x8c\xd9\x8f\x00\xb2\x04\xe9\x80\x09\x98\xec\xf8\x42\x7e");

    /* Binary */
    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&b, "\x00\x01\xfe\xff");
    ret = chula_buffer_encode_md5 (&a, &b);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 16);
    ck_assert_str_eq (b.buf, "\xd4\x1d\x8c\xd9\x8f\x00\xb2\x04\xe9\x80\x09\x98\xec\xf8\x42\x7e");
}
END_TEST

START_TEST (encode_sha1)
{
    ret_t           ret;
    chula_buffer_t  a    = CHULA_BUF_INIT;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_encode_sha1 (&a, &b);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 20);
    ck_assert_str_eq (b.buf,
                      "\xda\x39" "\xa3\xee" "\x5e\x6b" "\x4b\x0d" "\x32\x55"
                      "\xbf\xef" "\x95\x60" "\x18\x90" "\xaf\xd8" "\x07\x09");

    /* Binary */
    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&b, "\x00\x01\xfe\xff");
    ret = chula_buffer_encode_sha1 (&a, &b);
    ck_assert (ret == ret_ok);
    ck_assert (b.len == 20);
    ck_assert_str_eq (b.buf,
                      "\xda\x39" "\xa3\xee" "\x5e\x6b" "\x4b\x0d" "\x32\x55"
                      "\xbf\xef" "\x95\x60" "\x18\x90" "\xaf\xd8" "\x07\x09");
}
END_TEST


int
buffer_tests (void)
{
    Suite *s1 = suite_create("Buffer");
    check_add (s1, init_heap);
    check_add (s1, init_ptr);
    check_add (s1, dup_);
    check_add (s1, add);
    check_add (s1, slice);
    check_add (s1, add_fsize);
    check_add (s1, add_long10);
    check_add (s1, add_llong10);
    check_add (s1, add_long16);
    check_add (s1, add_llong16);
    check_add (s1, add_va);
    check_add (s1, add_char);
    check_add (s1, add_char_n);
    check_add (s1, prepend);
    check_add (s1, is_ending);
    check_add (s1, move_to_begin);
    check_add (s1, ensure_size);
    check_add (s1, drop_ending);
    check_add (s1, swap_chars);
    check_add (s1, remove_dups);
    check_add (s1, remove_string);
    check_add (s1, remove_chunk);
    check_add (s1, cmp_buf);
    check_add (s1, cmp);
    check_add (s1, crc32);
    check_add (s1, read_file);
    check_add (s1, read_from_fd);
    check_add (s1, multiply);
    check_add (s1, get_utf8_len);
    check_add (s1, unescape_uri);
    check_add (s1, escape_uri);
    check_add (s1, escape_uri_delims);
    check_add (s1, escape_arg);
    check_add (s1, add_escape_html);
    check_add (s1, base64);
    check_add (s1, md5_digest);
    check_add (s1, encode_md5);
    check_add (s1, encode_sha1);
    run_test (s1);
}


int
main (void)
{
    int ret;

    ret = buffer_tests();

    return ret;
}
