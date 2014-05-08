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
#include <arpa/inet.h>

#include <libchula-qa/libchula-qa.h>
#include <libchula-qa/testing_macros-internal.h>


START_TEST (init_heap)
{
    chula_buffer_t buf = CHULA_BUF_INIT;

    /* Init */
    ch_assert (buf.len == 0);
    ch_assert (buf.size == 0);
    ch_assert (buf.buf == NULL);

    /* Not empty */
    chula_buffer_add_str (&buf, "test");
    ch_assert (buf.len > 0);
    ch_assert (buf.buf != NULL);

    /* Empty */
    chula_buffer_clean (&buf);
    ch_assert (buf.len == 0);
    ch_assert (buf.buf != NULL);

    /* Free memory */
    chula_buffer_mrproper (&buf);
    ch_assert (buf.buf == NULL);
}
END_TEST


START_TEST (init_ptr)
{
    chula_buffer_t *buf = NULL;

    /* Instance */
    chula_buffer_new (&buf);
    ch_assert (buf != NULL);

    /* Emptiness */
    ch_assert (buf->len == 0);
    ch_assert (buf->buf == NULL);

    /* Not empty */
    chula_buffer_add_str (buf, "test");
    ch_assert (buf->len > 0);
    ch_assert (buf->buf != NULL);

    /* Empty */
    chula_buffer_clean (buf);
    ch_assert (buf->len == 0);
    ch_assert (buf->buf != NULL);

    chula_buffer_free (buf);
}
END_TEST

START_TEST (dup_)
{
    ret_t           ret;
    chula_buffer_t  b1   = CHULA_BUF_INIT;
    chula_buffer_t *b2   = NULL;

    chula_buffer_add_str (&b1, "testing");
    ret = chula_buffer_dup (&b1, &b2);

    ch_assert (ret == ret_ok);
    ch_assert (&b1 != b2);
    ch_assert (b1.len == b2->len);
    ch_assert (b1.buf != b2->buf);

    chula_buffer_mrproper(&b1);
    chula_buffer_free(b2);
}
END_TEST

START_TEST (add)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_add (&b, "123", 3);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 3);

    ret = chula_buffer_add (&b, "abcd", 4);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 3+4);

    ret = chula_buffer_add (&b, "", 0);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 3+4);

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (slice)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    chula_buffer_add_str (&a, "0123456789");

    /* [2:4] */
    ret = chula_buffer_add_buffer_slice (&b, &a, 2, 4);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 2);
    ch_assert_str_eq (b.buf, "23");

    /* [2:] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, 2, CHULA_BUF_SLIDE_NONE);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 8);
    ch_assert_str_eq (b.buf, "23456789");

    /* [-2:] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, -2, CHULA_BUF_SLIDE_NONE);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 2);
    ch_assert_str_eq (b.buf, "89");

    /* [:4] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, CHULA_BUF_SLIDE_NONE, 4);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 4);
    ch_assert_str_eq (b.buf, "0123");

    /* [:-4] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, CHULA_BUF_SLIDE_NONE, -4);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 6);
    ch_assert_str_eq (b.buf, "012345");

    /* [:] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, CHULA_BUF_SLIDE_NONE, CHULA_BUF_SLIDE_NONE);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 10);
    ch_assert_str_eq (b.buf, "0123456789");

    /* [0:0] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, 0, 0);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    /* [1:-1] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, 1, -1);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 8);
    ch_assert_str_eq (b.buf, "12345678");

    /* [-4:-2] */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_buffer_slice (&b, &a, -4, -2);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 2);
    ch_assert_str_eq (b.buf, "67");

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (add_fsize)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_fsize (&b, 0);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "0");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 900);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "900");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 1025);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "1.0K");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 2048);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "2.0K");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, (2*1024) + 300);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "2.3K");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 1024*1024);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "1.0M");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 9.5*1024*1024);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "9.5M");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 2.1 * 1024*1024*1024);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "2.1G");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 9.9 * 1024*1024*1024*1024);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "9.9T");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_fsize (&b, 6.2 * 1024*1024*1024*1024*1024);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "6.2P");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (add_long10)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_long10 (&b, 1l);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "1");

    /* LONG_MIN: -2^31+1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_long10 (&b, -2147483647);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "-2147483647");

    /* LONG_MAX: 2^31-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_long10 (&b, 2147483647);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "2147483647");

    /* ULONG_MAX: 2^32-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_ulong10 (&b, 4294967295);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "4294967295");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (add_llong10)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_llong10 (&b, 0);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "0");

    /* LLONG_MIN: -2^63+1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_llong10 (&b, -9223372036854775807);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "-9223372036854775807");

    /* LLONG_MAX: 2^63-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_llong10 (&b, 9223372036854775807);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "9223372036854775807");

    /* ULLONG_MAX: 2^64-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_ullong10 (&b, 18446744073709551615ull);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "18446744073709551615");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (add_long16)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_ulong16 (&b, 1);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "1");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_ulong16 (&b, 10);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "a");

    /* LONG_MAX: 2^31-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_ulong16 (&b, 2147483647);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "7fffffff");

    /* ULONG_MAX: 2^32-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_ulong16 (&b, 4294967295);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "ffffffff");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (add_llong16)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_ullong16 (&b, 0);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "0");

    /* LLONG_MAX: 2^63-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_ullong16 (&b, 9223372036854775807ull);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "7fffffffffffffff");

    /* ULLONG_MAX: 2^64-1 */
    chula_buffer_clean (&b);
    ret = chula_buffer_add_ullong16 (&b, 18446744073709551615ull);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "ffffffffffffffff");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (add_uint16be)
{
    ret_t          ret;
    uint16_t       num;
    chula_buffer_t b    = CHULA_BUF_INIT;

    num = htons(0);
    ret = chula_buffer_add_uint16be (&b, 0);
    ch_assert (ret == ret_ok);
    ch_assert (memcmp (&num, b.buf, sizeof(uint16_t)) == 0);

    num = htons(999);
    chula_buffer_clean (&b);
    ret = chula_buffer_add_uint16be (&b, 999);
    ch_assert (ret == ret_ok);
    ch_assert (memcmp (&num, b.buf, b.len) == 0);

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (add_uint32be)
{
    ret_t          ret;
    uint32_t       num;
    chula_buffer_t b    = CHULA_BUF_INIT;

    num = htons(0);
    ret = chula_buffer_add_uint32be (&b, 0);
    ch_assert (ret == ret_ok);
    ch_assert (memcmp (&num, b.buf, sizeof(uint32_t)) == 0);

    /* 2^32 - 1 */
    num = htonl(4294967295);
    chula_buffer_clean (&b);
    ret = chula_buffer_add_uint32be (&b, 4294967295);
    ch_assert (ret == ret_ok);
    ch_assert (memcmp (&num, b.buf, b.len) == 0);

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (add_va)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_va (&b, "");
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_clean (&b);
    ret = chula_buffer_add_va (&b, "hi there");
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "hi there");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_va (&b, "%d is not one", 0);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "0 is not one");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_va (&b, "%03d rocks", 7);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "007 rocks");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_va (&b, "%s %%s", "tricky");
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "tricky %s");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_va (&b, "%d - %1.1f - %s", 1, 2.1, "foo");
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "1 - 2.1 - foo");

    chula_buffer_clean (&b);
    ret = chula_buffer_add_va (&b, "%d %lu %llu %c %o %f %p %x", -99, 123, 456, 'z', 8, 0.123456, (void*)0x4040, 0xff);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "-99 123 456 z 10 0.123456 0x4040 ff");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (add_char)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_char (&b, 'a');
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 1);
    ch_assert_str_eq (b.buf, "a");

    ret = chula_buffer_add_char (&b, 'b');
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 2);
    ch_assert_str_eq (b.buf, "ab");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (add_char_n)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_char_n (&b, 'a', -1);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);
    ch_assert (b.buf == NULL);

    ret = chula_buffer_add_char_n (&b, 'a', 0);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);
    ch_assert (b.buf == NULL);

    ret = chula_buffer_add_char_n (&b, 'a', 5);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 5);
    ch_assert_str_eq (b.buf, "aaaaa");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (prepend)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_prepend (&b, "", 0);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);
    ch_assert (b.buf == NULL);

    ret = chula_buffer_prepend (&b, "!", 1);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 1);
    ch_assert_str_eq (b.buf, "!");

    ret = chula_buffer_prepend (&b, " there", 6);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 7);
    ch_assert_str_eq (b.buf, " there!");

    ret = chula_buffer_prepend (&b, "Hi", 2);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 9);
    ch_assert_str_eq (b.buf, "Hi there!");

    chula_buffer_clean (&b);
    ret = chula_buffer_prepend (&b, "Allocated", 9);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 9);
    ch_assert_str_eq (b.buf, "Allocated");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (is_ending)
{
    int            re;
    chula_buffer_t b   = CHULA_BUF_INIT;

    re = chula_buffer_is_ending (&b, ' ');
    ch_assert (re == 0);

    re = chula_buffer_is_ending (&b, 'a');
    ch_assert (re == 0);

    chula_buffer_add_str (&b, "12345");

    re = chula_buffer_is_ending (&b, '4');
    ch_assert (re == 0);

    re = chula_buffer_is_ending (&b, '5');
    ch_assert (re == 1);

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (move_to_begin)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    chula_buffer_add_str (&b, "0123456789");
    ret = chula_buffer_move_to_begin (&b, 11);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&b, "0123456789");
    ret = chula_buffer_move_to_begin (&b, 0);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "0123456789");

    ret = chula_buffer_move_to_begin (&b, 3);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "3456789");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (ensure_size)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ch_assert (b.buf == NULL);
    ch_assert (b.size == 0);

    ret = chula_buffer_ensure_size (&b, 100);
    ch_assert (ret == ret_ok);
    ch_assert (b.buf != NULL);
    ch_assert (b.size == 100);

    ret = chula_buffer_ensure_size (&b, 99);
    ch_assert (ret == ret_ok);
    ch_assert (b.buf != NULL);
    ch_assert (b.size == 100);

    ret = chula_buffer_ensure_size (&b, 200);
    ch_assert (ret == ret_ok);
    ch_assert (b.buf != NULL);
    ch_assert (b.size == 200);

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (retract)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ch_assert (b.buf == NULL);
    ch_assert (b.size == 0);

    ret = chula_buffer_retract (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.buf == NULL);
    ch_assert (b.size == 0);

    ret = chula_buffer_ensure_size (&b, 100);
    ch_assert (ret == ret_ok);
    ch_assert (b.buf != NULL);
    ch_assert (b.size == 100);

    ret = chula_buffer_add_str (&b, "abcd");
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 4);
    ch_assert (b.size == 100);

    ret = chula_buffer_retract (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.buf != NULL);
    ch_assert (b.len == 4);
    ch_assert (b.size == 5);
    ch_assert_str_eq (b.buf, "abcd");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (drop_ending)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ch_assert (b.buf == NULL);
    ch_assert (b.len == 0);
    ret = chula_buffer_drop_ending (&b, 1);
    ch_assert (ret == ret_ok);
    ch_assert (b.buf == NULL);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&b, "0123456789");

    ret = chula_buffer_drop_ending (&b, 0);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 10);
    ch_assert_str_eq (b.buf, "0123456789");

    ret = chula_buffer_drop_ending (&b, 2);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 8);
    ch_assert_str_eq (b.buf, "01234567");

    ret = chula_buffer_drop_ending (&b, 999);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (swap_chars)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ch_assert (b.buf == NULL);
    ch_assert (b.len == 0);
    ret = chula_buffer_drop_ending (&b, 1);
    ch_assert (ret == ret_ok);
    ch_assert (b.buf == NULL);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&b, "0123401234");

    ret = chula_buffer_swap_chars (&b, '2', '*');
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 10);
    ch_assert_str_eq (b.buf, "01*3401*34");

    ret = chula_buffer_swap_chars (&b, 'Z', '$');
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 10);
    ch_assert_str_eq (b.buf, "01*3401*34");

    chula_buffer_clean (&b);
    ret = chula_buffer_swap_chars (&b, 'Z', '$');
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (remove_dups)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_remove_dups (&b, 'Z');
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&b, "abcccde");
    ret = chula_buffer_remove_dups (&b, 'c');
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "abcde");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "aaaaaaa");
    ret = chula_buffer_remove_dups (&b, 'a');
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 1);
    ch_assert_str_eq (b.buf, "a");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (remove_string)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_remove_string (&b, NULL, 0);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&b, "hola hola caracola");
    ret = chula_buffer_remove_string (&b, "hola ", 5);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "caracola");

    ret = chula_buffer_remove_string (&b, "bye", 3);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "caracola");

    ret = chula_buffer_remove_string (&b, "raco", 0);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "caracola");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (remove_chunk)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_remove_chunk (&b, 0, 0);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);
    ch_assert (b.buf == NULL);

    chula_buffer_add_str (&b, "0123456789");
    ret = chula_buffer_remove_chunk (&b, 11, 2);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 10);
    ch_assert_str_eq (b.buf, "0123456789");

    ret = chula_buffer_remove_chunk (&b, 5, 4);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 6);
    ch_assert_str_eq (b.buf, "012349");

    ret = chula_buffer_remove_chunk (&b, 0, 99);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (cmp_buf)
{
    int            re;
    chula_buffer_t a   = CHULA_BUF_INIT;
    chula_buffer_t b   = CHULA_BUF_INIT;

    re = chula_buffer_cmp_buf (&a, &b);
    ch_assert (re == 0);

    /* Case sensitive */
    chula_buffer_add_str (&a, "a");
    re = chula_buffer_cmp_buf (&a, &b);
    ch_assert (re > 0);
    re = chula_buffer_cmp_buf (&b, &a);
    ch_assert (re < 0);

    chula_buffer_add_str (&a, "bcd");
    chula_buffer_add_str (&b, "abcd");
    re = chula_buffer_cmp_buf (&a, &b);
    ch_assert (re == 0);
    re = chula_buffer_cmp_buf (&b, &a);
    ch_assert (re == 0);

    /* Case insensitive */
    chula_buffer_mrproper (&a);
    chula_buffer_mrproper (&b);

    chula_buffer_add_str (&a, "A");
    re = chula_buffer_case_cmp_buf (&a, &b);
    ch_assert (re > 0);
    re = chula_buffer_case_cmp_buf (&b, &a);
    ch_assert (re < 0);

    chula_buffer_add_str (&a, "BcD");
    chula_buffer_add_str (&b, "aBcD");
    re = chula_buffer_case_cmp_buf (&a, &b);
    ch_assert (re == 0);
    re = chula_buffer_case_cmp_buf (&b, &a);
    ch_assert (re == 0);

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (cmp)
{
    int            re;
    chula_buffer_t b   = CHULA_BUF_INIT;

    /* Case sensitive */
    re = chula_buffer_cmp (&b, NULL, 0);
    ch_assert (re == 0);

    re = chula_buffer_cmp (&b, "a", 1);
    ch_assert (re < 0);

    chula_buffer_add_str (&b, "hola");
    re = chula_buffer_cmp (&b, "a", 1);
    ch_assert (re > 0);

    re = chula_buffer_cmp (&b, "ho", 2);
    ch_assert (re > 0);

    re = chula_buffer_cmp (&b, "hola", 2);
    ch_assert (re > 0);

    re = chula_buffer_cmp (&b, "hola", 4);
    ch_assert (re == 0);

    /* Case insensitive */
    chula_buffer_mrproper (&b);

    re = chula_buffer_case_cmp (&b, NULL, 0);
    ch_assert (re == 0);

    re = chula_buffer_case_cmp (&b, "A", 1);
    ch_assert (re < 0);

    chula_buffer_add_str (&b, "hola");
    re = chula_buffer_case_cmp (&b, "A", 1);
    ch_assert (re > 0);

    re = chula_buffer_case_cmp (&b, "hO", 2);
    ch_assert (re > 0);

    re = chula_buffer_case_cmp (&b, "HolA", 2);
    ch_assert (re > 0);

    re = chula_buffer_case_cmp (&b, "hoLa", 4);
    ch_assert (re == 0);

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (crc32)
{
    crc_t          crc;
    chula_buffer_t b   = CHULA_BUF_INIT;

    /* python -c "import zlib; print zlib.crc32('')" */
    crc = chula_buffer_crc32 (&b);
    ch_assert (crc == 0);

    /* python -c "import zlib; print zlib.crc32('h')" */
    chula_buffer_add_str (&b, "h");
    crc = chula_buffer_crc32 (&b);
    ch_assert (crc == 2439710439);

    /* python -c "import zlib; print zlib.crc32('hola')" */
    chula_buffer_add_str (&b, "ola");
    crc = chula_buffer_crc32 (&b);
    ch_assert (crc == 1872820616);

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (read_file)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    /* Doesn't exists */
    ret = chula_buffer_read_file (&b, "/it/doesnt/exist");
    ch_assert (ret != ret_ok);

    /* Not a file */
    ret = chula_buffer_read_file (&b, "/");
    ch_assert (ret != ret_ok);

    /* Read an actual file */
    FILE *f        = NULL;
    char *template = strdup ("buffer_XXXXXX");
    char *filename = mktemp (template);
    ch_assert (filename != NULL);

    f = fopen (filename, "w+");
    ch_assert (f != NULL);

    fwrite ("hola", 1, 4, f);
    fclose(f);

    ret = chula_buffer_read_file (&b, filename);
    unlink (filename);

    ch_assert (ret == ret_ok);
    ch_assert (b.len == 4);
    ch_assert_str_eq (b.buf, "hola");

    free (template);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (read_from_fd)
{
    int            fd;
    ret_t          ret;
    size_t         did_read = 0;
    chula_buffer_t b        = CHULA_BUF_INIT;

    /* Doesn't exists */
    fd = open ("/it/doesnt/exist", O_RDONLY);
    ch_assert (fd == -1);
    ret = chula_buffer_read_from_fd (&b, fd, 0, &did_read);
    ch_assert (ret == ret_error);

    /* Not a file */
    fd = open ("/", O_RDONLY);
    ret = chula_buffer_read_from_fd (&b, fd, 1, &did_read);
    close (fd);
    ch_assert (ret == ret_error);

    /* Read an actual file */
    FILE *f        = NULL;
    char *template = strdup ("buffer_XXXXXX");
    char *filename = mktemp (template);

    f = fopen (filename, "w+");
    ch_assert (f != NULL);

    fwrite ("content", 1, 7, f);
    fclose(f);

    fd = open (filename, O_RDONLY);
    ret = chula_buffer_read_from_fd (&b, fd, 9999, &did_read);
    chula_fd_close (fd);
    unlink (filename);

    ch_assert (ret == ret_ok);
    ch_assert (b.len == 7);
    ch_assert_str_eq (b.buf, "content");

    free (template);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (multiply)
{
    ret_t           ret;
    chula_buffer_t  b    = CHULA_BUF_INIT;

    ret = chula_buffer_multiply (&b, 10);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&b, "chula");

    ret = chula_buffer_multiply (&b, 1);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 5);

    ret = chula_buffer_multiply (&b, 3);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 15);

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (get_utf8_len)
{
    ret_t          ret;
    uint32_t       len  = 0;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_get_utf8_len (&b, &len);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&b, "hola");
    ret = chula_buffer_get_utf8_len (&b, &len);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 4);

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "ñáñé");
    ret = chula_buffer_get_utf8_len (&b, &len);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 8);
    ch_assert (len == 4);

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "イロハニホヘト");
    ret = chula_buffer_get_utf8_len (&b, &len);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 21);
    ch_assert (len == 7);

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "ψυχοφθόρα");
    ret = chula_buffer_get_utf8_len (&b, &len);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 18);
    ch_assert (len == 9);

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (unescape_uri)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_unescape_uri (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&b, "http://host/file?key1=one&key2=two");
    ret = chula_buffer_unescape_uri(&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "http://host/file?key1=one&key2=two");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "http://host/with%20spaces");
    ret = chula_buffer_unescape_uri(&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "http://host/with spaces");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "http://host/100%25");
    ret = chula_buffer_unescape_uri(&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "http://host/100%");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "http://host/end%00not");
    ret = chula_buffer_unescape_uri(&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "http://host/end not");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (escape_uri)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_escape_uri (&b, &a);
    ch_assert (ret == ret_ok);
    ch_assert (a.len == 0);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&a, "http://host/file?key1=one&key2=two");
    ret = chula_buffer_escape_uri (&b, &a);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "http://host/file%3fkey1=one&key2=two");

    chula_buffer_clean (&a);
    chula_buffer_clean (&b);
    chula_buffer_add_str (&a, "http://host/with spaces");
    ret = chula_buffer_escape_uri(&b, &a);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "http://host/with%20spaces");

    chula_buffer_clean (&a);
    chula_buffer_clean (&b);
    chula_buffer_add_str (&a, "\x00\x01\x1f\x7f\xff");
    ret = chula_buffer_escape_uri(&b, &a);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "%00%01%1f%7f%ff");

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (escape_uri_delims)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_escape_uri_delims (&b, &a);
    ch_assert (ret == ret_ok);
    ch_assert (a.len == 0);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&a, "hola \x00\x01\x1f :?#[]@ \x7f\xff adios!<>{}-_");
    ret = chula_buffer_escape_uri_delims(&b, &a);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "hola%20%00%01%1f%20%3a%3f%23%5b%5d%40%20%7f%ff%20adios!<>{}-_");

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (escape_arg)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_escape_arg (&b, &a);
    ch_assert (ret == ret_ok);
    ch_assert (a.len == 0);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&a, "hola \x00\x01\x1f #%&+? \x7f\xff adios!@[]<>");
    ret = chula_buffer_escape_arg(&b, &a);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "hola%20%00%01%1f%20%23%25%26%2b%3f%20%7f%ff%20adios!@[]<>");

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (add_escape_html)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_escape_html (&b, &a);
    ch_assert (ret == ret_ok);
    ch_assert (a.len == 0);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&a, "<this>");
    ret = chula_buffer_add_escape_html (&b, &a);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "&lt;this&gt;");

    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&a, "a & b < c > d \" e ' f / g # @[]{}-_$!");
    ret = chula_buffer_add_escape_html (&b, &a);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "a &amp; b &lt; c &gt; d &quot; e &#39; f &#47; g &#35; @[]{}-_$!");

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (base64)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_encode_base64 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (a.len == 0);
    ch_assert (b.len == 0);

    ret = chula_buffer_decode_base64 (&a);
    ch_assert (ret == ret_ok);
    ch_assert (a.len == 0);

    /* Text string */
    chula_buffer_add_str (&a, "testing");
    ret = chula_buffer_encode_base64 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "dGVzdGluZw==");
    ch_assert (b.len == 12);

    ret = chula_buffer_decode_base64 (&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "testing");
    ch_assert (b.len ==7);

    /* Binary */
    chula_buffer_clean (&a);
    chula_buffer_clean (&a);

    chula_buffer_add_str (&a, "\x00\x01\x03\x04\xfd\xfe\xff");
    ret = chula_buffer_encode_base64 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "AAEDBP3+/w==");
    ch_assert (b.len == 12);

    ret = chula_buffer_decode_base64 (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 7);

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (md5_digest)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_encode_md5_digest (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 32);
    ch_assert_str_eq (b.buf, "d41d8cd98f00b204e9800998ecf8427e");

    /* Text */
    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "hola");
    ret = chula_buffer_encode_md5_digest (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 32);
    ch_assert_str_eq (b.buf, "4d186321c1a7f0f354b297e8914ab240");

    /* Binary */
    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "\x00\x01\xfe\xff");
    ret = chula_buffer_encode_md5_digest (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 32);
    ch_assert_str_eq (b.buf, "f129ac2d8a0ee74923012d1c8ed83a86");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (encode_md5)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_encode_md5 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 16);
    ch_assert_str_eq (b.buf, "\xd4\x1d\x8c\xd9\x8f\x00\xb2\x04\xe9\x80\x09\x98\xec\xf8\x42\x7e");

    /* Binary */
    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&b, "\x00\x01\xfe\xff");
    ret = chula_buffer_encode_md5 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 16);
    ch_assert_str_eq (b.buf, "\xd4\x1d\x8c\xd9\x8f\x00\xb2\x04\xe9\x80\x09\x98\xec\xf8\x42\x7e");

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (encode_sha1)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_encode_sha1 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 20);
    ch_assert_str_eq (b.buf,
                      "\xda\x39" "\xa3\xee" "\x5e\x6b" "\x4b\x0d" "\x32\x55"
                      "\xbf\xef" "\x95\x60" "\x18\x90" "\xaf\xd8" "\x07\x09");

    /* Binary */
    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&b, "\x00\x01\xfe\xff");
    ret = chula_buffer_encode_sha1 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 20);
    ch_assert_str_eq (b.buf,
                      "\xda\x39" "\xa3\xee" "\x5e\x6b" "\x4b\x0d" "\x32\x55"
                      "\xbf\xef" "\x95\x60" "\x18\x90" "\xaf\xd8" "\x07\x09");

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (sha1_digest)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_encode_sha1_digest (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 40);
    ch_assert_str_eq (b.buf, "da39a3ee5e6b4b0d3255bfef95601890afd80709");

    /* Text */
    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "abc 123");
    ret = chula_buffer_encode_sha1_digest (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 40);
    ch_assert_str_eq (b.buf, "e2d0a343442ba7bd2c0537659a05e61668575f2b");

    /* Binary */
    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "\x00\x01\xfe\xff");
    ret = chula_buffer_encode_sha1_digest (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 40);
    ch_assert_str_eq (b.buf, "302c1f256c8e9ebb5edf0822b473d0cd3d2ce84c");

    /* Large */
    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "abc");
    chula_buffer_multiply (&b, 64);
    ret = chula_buffer_encode_sha1_digest (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 40);
    ch_assert_str_eq (b.buf, "c8d58115d6878bdade7e3f3ab1a202cd771a0ee4");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (sha1_base64)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_encode_sha1_base64 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 28);
    ch_assert_str_eq (b.buf, "2jmj7l5rSw0yVb/vlWAYkK/YBwk=");

    /* Text */
    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&a, "abc 123");
    ret = chula_buffer_encode_sha1_base64 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 28);
    ch_assert_str_eq (b.buf, "4tCjQ0Qrp70sBTdlmgXmFmhXXys=");

    /* Binary */
    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&a, "\x00\x01\xfe\xff");
    ret = chula_buffer_encode_sha1_base64 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 28);
    ch_assert_str_eq (b.buf, "MCwfJWyOnrte3wgitHPQzT0s6Ew=");

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (encode_sha512)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    /* Empty:
     * shasum -a 512 </dev/null
     */
    ret = chula_buffer_encode_sha512 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 64);
    ch_assert_str_eq (b.buf,
                      "\xcf\x83\xe1\x35\x7e\xef\xb8\xbd\xf1\x54\x28\x50\xd6\x6d"
                      "\x80\x07\xd6\x20\xe4\x05\x0b\x57\x15\xdc\x83\xf4\xa9\x21"
                      "\xd3\x6c\xe9\xce\x47\xd0\xd1\x3c\x5d\x85\xf2\xb0\xff\x83"
                      "\x18\xd2\x87\x7e\xec\x2f\x63\xb9\x31\xbd\x47\x41\x7a\x81"
                      "\xa5\x38\x32\x7a\xf9\x27\xda\x3e");

    /* Test 1
     * echo -n "The quick brown fox jumps over the lazy dog" | shasum -a 512
     */
    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&a, "The quick brown fox jumps over the lazy dog");
    ret = chula_buffer_encode_sha512 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 64);
    chula_print_repr (chula, buffer, &b);
    ch_assert (memcmp (b.buf,
                       "\x07\xe5\x47\xd9\x58\x6f\x6a\x73\xf7\x3f\xba\xc0\x43\x5e"
                       "\xd7\x69\x51\x21\x8f\xb7\xd0\xc8\xd7\x88\xa3\x09\xd7\x85"
                       "\x43\x6b\xbb\x64\x2e\x93\xa2\x52\xa9\x54\xf2\x39\x12\x54"
                       "\x7d\x1e\x8a\x3b\x5e\xd6\xe1\xbf\xd7\x09\x78\x21\x23\x3f"
                       "\xa0\x53\x8f\x3d\xb8\x54\xfe\xe6", b.len) == 0);

    /* Test 2
     * echo -n "The quick brown fox jumps over the lazy dog." | shasum -a 512
     */
    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&a, "The quick brown fox jumps over the lazy dog.");
    ret = chula_buffer_encode_sha512 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 64);
    ch_assert (memcmp (b.buf,
                       "\x91\xea\x12\x45\xf2\x0d\x46\xae\x9a\x03\x7a\x98\x9f\x54"
                       "\xf1\xf7\x90\xf0\xa4\x76\x07\xee\xb8\xa1\x4d\x12\x89\x0c"
                       "\xea\x77\xa1\xbb\xc6\xc7\xed\x9c\xf2\x05\xe6\x7b\x7f\x2b"
                       "\x8f\xd4\xc7\xdf\xd3\xa7\xa8\x61\x7e\x45\xf3\xc4\x63\xd4"
                       "\x81\xc7\xe5\x86\xc3\x9a\xc1\xed", b.len) == 0);

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (sha512_digest)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_encode_sha512_digest (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 128);
    ch_assert_str_eq (b.buf, "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");

    /* Text */
    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "abc 123");
    ret = chula_buffer_encode_sha512_digest (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 128);
    ch_assert_str_eq (b.buf, "af5c7e6e0028bce466814d8b8f5651ddd82497b6f006bdd818194e14903114a6549ed4ef5e2480591f1e13f3bc0bb8879c3b8f7750349aaecd0f7ee11331c052");

    /* Binary */
    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "\x00\x01\xfe\xff");
    ret = chula_buffer_encode_sha512_digest (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 128);
    ch_assert_str_eq (b.buf, "4be83ccadf8eac0eae59651f3c2eb4a60342235ee1bd08366f3b2a40bd045c5910e69380c53454090c243965b1571eab2760663d9c52f291f9be20d82e4dc45a");

    /* Large */
    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "abc");
    chula_buffer_multiply (&b, 64);
    ret = chula_buffer_encode_sha512_digest (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 128);
    ch_assert_str_eq (b.buf, "450e94ad0bd1f53aafcc83f4ce4a3e0998ec265ab633584c1c208100f9c26a8e10cb898c5aaf39ee635fbe4d5901ce804733b1e964b22aa1f58685404b29255e");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (sha512_base64)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_encode_sha512_base64 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 88);
    ch_assert_str_eq (b.buf, "z4PhNX7vuL3xVChQ1m2AB9Yg5AULVxXcg/SpIdNs6c5H0NE8XYXysP+DGNKHfuwvY7kxvUdBeoGlODJ6+SfaPg==");

    /* Text */
    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&a, "abc 123");
    ret = chula_buffer_encode_sha512_base64 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 88);
    ch_assert_str_eq (b.buf, "r1x+bgAovORmgU2Lj1ZR3dgkl7bwBr3YGBlOFJAxFKZUntTvXiSAWR8eE/O8C7iHnDuPd1A0mq7ND37hEzHAUg==");

    /* Binary */
    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&a, "\x00\x01\xfe\xff");
    ret = chula_buffer_encode_sha512_base64 (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 88);
    ch_assert_str_eq (b.buf, "S+g8yt+OrA6uWWUfPC60pgNCI17hvQg2bzsqQL0EXFkQ5pOAxTRUCQwkOWWxVx6rJ2BmPZxS8pH5viDYLk3EWg==");

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (encode_hex)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_encode_hex (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);
    ch_assert_str_eq (b.buf, "");

    /* Text */
    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&a, "abc 123");
    ret = chula_buffer_encode_hex (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 14);
    ch_assert_str_eq (b.buf, "61626320313233");

    /* Binary */
    chula_buffer_clean (&a);
    chula_buffer_clean (&b);

    chula_buffer_add_str (&a, "\x00\x01\xfe\xff");
    ret = chula_buffer_encode_hex (&a, &b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 8);
    ch_assert_str_eq (b.buf, "0001feff");

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (decode_hex)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_buffer_decode_hex (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    /* /\* Text *\/ */
    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "61626320313233");
    ret = chula_buffer_decode_hex (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 7);
    ch_assert_str_eq (b.buf, "abc 123");

    /* Binary */
    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "0001feff");
    ret = chula_buffer_decode_hex (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 4);
    ch_assert_str_eq (b.buf, "\x00\x01\xfe\xff");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (end_char)
{
    char           re;
    chula_buffer_t b   = CHULA_BUF_INIT;

    re = chula_buffer_end_char (&b);
    ch_assert (re == '\0');

    chula_buffer_add_str (&b, "ab\0cd");
    re = chula_buffer_end_char (&b);
    ch_assert (re == 'd');

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (replace_string)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_replace_string (&b, "hola", 4, "adios", 5);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&b, "hola caracola");

    ret = chula_buffer_replace_string (&b, NULL, 0, "adios", 5);
    ch_assert (ret == ret_deny);

    ret = chula_buffer_replace_string (&b, "hola", 4, NULL, 0);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, " caracola");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "hola caracola");
    ret = chula_buffer_replace_string (&b, "hola", 4, "adios", 5);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "adios caracola");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (substitute_string)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_substitute_string (&a, &b, "hola", 4, "adios", 5);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);
    ch_assert (a.len == 0);

    chula_buffer_add_str (&a, "hola caracola");

    ret = chula_buffer_substitute_string (&a, &b, NULL, 0, "adios", 5);
    ch_assert (ret == ret_deny);

    ret = chula_buffer_substitute_string (&a, &b, "hola", 4, NULL, 0);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, " caracola");

    ret = chula_buffer_substitute_string (&a, &b, "hola", 4, "adios", 5);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "adios caracola");

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (add_comma_marks)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_add_comma_marks (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&b, "99");
    ret = chula_buffer_add_comma_marks (&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "99");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "999");
    ret = chula_buffer_add_comma_marks (&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "999");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "9999");
    ret = chula_buffer_add_comma_marks (&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "9,999");

   chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "99999");
    ret = chula_buffer_add_comma_marks (&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "99,999");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "1234567890");
    ret = chula_buffer_add_comma_marks (&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "1,234,567,890");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (trim)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_trim (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, " ");
    ret = chula_buffer_trim (&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "a");
    ret = chula_buffer_trim (&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "a");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, " abc ");
    ret = chula_buffer_trim (&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "abc");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "\ttesting\r\n\r\n");
    ret = chula_buffer_trim (&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "testing");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (to_lowcase)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_to_lowcase (&b);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "123 456");
    ret = chula_buffer_to_lowcase (&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "123 456");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "AaBbCcDdEeFf");
    ret = chula_buffer_to_lowcase (&b);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "aabbccddeeff");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (insert)
{
    ret_t          ret;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_insert (&b, NULL, 0, 0);
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_clean (&b);

    chula_buffer_add_str (&b, "123");
    ret = chula_buffer_insert (&b, "ab", 2, 0);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "ab123");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "123");
    ret = chula_buffer_insert (&b, "ab", 2, 1);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "1ab23");

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "123");
    ret = chula_buffer_insert (&b, "ab", 2, 9999);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (b.buf, "123ab");

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (insert_buffer)
{
    ret_t          ret;
    chula_buffer_t a    = CHULA_BUF_INIT;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_insert_buffer (&a, &b, 0);
    ch_assert (ret == ret_ok);
    ch_assert (a.len == 0);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&b, "\0");
    ret = chula_buffer_insert_buffer (&a, &b, 0);
    ch_assert (ret == ret_ok);
    ch_assert (a.len == 1);

    chula_buffer_clean (&a);
    chula_buffer_clean (&b);
    chula_buffer_add_str (&a, "hola");
    chula_buffer_add_str (&b, "12");
    ret = chula_buffer_insert_buffer (&a, &b, 2);
    ch_assert (ret == ret_ok);
    ch_assert (a.len == 6);
    ch_assert_str_eq (a.buf, "ho12la");

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST


START_TEST (split_lines)
{
    ret_t          ret;
    int            num;
    chula_buffer_t b    = CHULA_BUF_INIT;

    ret = chula_buffer_split_lines (&b, 80,  " ");
    ch_assert (ret == ret_ok);
    ch_assert (b.len == 0);

    chula_buffer_add_str (&b, "1 2 3 4 5 6 7 8 9");
    ret = chula_buffer_split_lines (&b, 10,  "\t");
    num = 0;
    for (uint32_t i=0; i<b.len; i++) num += (b.buf[i]=='\t')?1:0;
    ch_assert (ret == ret_ok);
    ch_assert (num == 1);

    chula_buffer_clean (&b);
    chula_buffer_add_str (&b, "1 2 3 4 5 6 7 8 9 10 11 12");
    ret = chula_buffer_split_lines (&b, 10,  "*");
    num = 0;
    for (uint32_t i=0; i<b.len; i++) num += (b.buf[i]=='*')?1:0;
    ch_assert (ret == ret_ok);
    ch_assert (num == 2);

    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (swap_buffers)
{
    chula_buffer_t a = CHULA_BUF_INIT;
    chula_buffer_t b = CHULA_BUF_INIT;

    chula_buffer_add_str (&a, "a");
    chula_buffer_swap_buffers (&a, &b);
    ch_assert (a.len == 0);
    ch_assert (a.buf == NULL);
    ch_assert (b.len == 1);
    ch_assert (b.buf != NULL);

    chula_buffer_add_str (&a, "bb");
    chula_buffer_swap_buffers (&a, &b);
    ch_assert (a.len == 1);
    ch_assert (b.len == 2);

    chula_buffer_mrproper(&a);
    chula_buffer_mrproper(&b);
}
END_TEST

START_TEST (cnt_spn)
{
    size_t         re;
    chula_buffer_t buf = CHULA_BUF_INIT;

    re = chula_buffer_cnt_spn (&buf, 0, "aeiou");
    ch_assert (re == 0);

    chula_buffer_fake_str (&buf, "aaaaaiiiiizooo");
    re = chula_buffer_cnt_spn (&buf, 0, "aeiou");
    ch_assert (re == 10);

    chula_buffer_fake_str (&buf, "aaaaaiiiiizooo");
    re = chula_buffer_cnt_spn (&buf, 11, "aeiou");
    ch_assert (re == 3);

    chula_buffer_fake_str (&buf, "0aaaaaiiiiizooo");
    re = chula_buffer_cnt_spn (&buf, 0, "aeiou");
    ch_assert (re == 0);
}
END_TEST

START_TEST (cnt_cspn)
{
    size_t         re;
    chula_buffer_t buf = CHULA_BUF_INIT;

    re = chula_buffer_cnt_spn (&buf, 0, "aeiou");
    ch_assert (re == 0);

    chula_buffer_fake_str (&buf, "xxxxxyyyyyazzz");
    re = chula_buffer_cnt_cspn (&buf, 0, "aeiou");
    ch_assert (re == 10);

    chula_buffer_fake_str (&buf, "xxxxxyyyyyazzz");
    re = chula_buffer_cnt_cspn (&buf, 11, "aeiou");
    ch_assert (re == 3);

    chula_buffer_fake_str (&buf, "uxxxxxyyyyyazzz");
    re = chula_buffer_cnt_cspn (&buf, 0, "aeiou");
    ch_assert (re == 0);
}
END_TEST

START_TEST (print_debug)
{
    chula_buffer_t buf = CHULA_BUF_INIT;

    chula_buffer_fake_str (&buf, "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz");
    chula_print_repr (chula, buffer, &buf);
}
END_TEST

START_TEST (print_cstr)
{
    ret_t          ret;
    chula_buffer_t buf  = CHULA_BUF_INIT;

    chula_buffer_fake_str (&buf, "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz");
    ret = chula_buffer_print_cstr (&buf);
    ch_assert (ret == ret_ok);
}
END_TEST

START_TEST (_import)
{
    ret_t           ret;
    int             len;
    chula_buffer_t  buf  = CHULA_BUF_INIT;
    char           *mem  = NULL;

    /* Allocate a chunk of memory */
    mem = malloc (1234);
    ch_assert (mem != NULL);

    /* Write on it */
    len = snprintf (mem, 1234, "hola");
    ch_assert (len == 4);

    /* Import it as a buffer */
    ret = chula_buffer_import (&buf, mem, len);
    ch_assert (ret == ret_ok);
    ch_assert ((uint8_t *)mem == buf.buf);

    /* Append something, for instance */
    ret = chula_buffer_add_str (&buf, " caracola");
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (buf.buf, "hola caracola");
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
    check_add (s1, add_uint16be);
    check_add (s1, add_uint32be);
    check_add (s1, add_va);
    check_add (s1, add_char);
    check_add (s1, add_char_n);
    check_add (s1, prepend);
    check_add (s1, is_ending);
    check_add (s1, move_to_begin);
    check_add (s1, ensure_size);
    check_add (s1, retract);
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
    check_add (s1, encode_sha512);
    check_add (s1, sha1_digest);
    check_add (s1, sha1_base64);
    check_add (s1, sha512_digest);
    check_add (s1, sha512_base64);
    check_add (s1, encode_hex);
    check_add (s1, decode_hex);
    check_add (s1, end_char);
    check_add (s1, replace_string);
    check_add (s1, substitute_string);
    check_add (s1, add_comma_marks);
    check_add (s1, trim);
    check_add (s1, to_lowcase);
    check_add (s1, insert);
    check_add (s1, insert_buffer);
    check_add (s1, split_lines);
    check_add (s1, swap_buffers);
    check_add (s1, cnt_spn);
    check_add (s1, cnt_cspn);
    check_add (s1, print_debug);
    check_add (s1, print_cstr);
    check_add (s1, _import);
    run_test (s1);
}
