/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/* All files in libhpack are Copyright (C) 2014 Alvaro Lopez Ortega.
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
#include "libhpack/integer.h"

/* All examples came from:
 * http://tools.ietf.org/html/draft-ietf-httpbis-header-compression-05
 */

#define check_add_tc(suit,testcase,func)		\
    do {                                        \
        suite_add_tcase (suit, testcase);       \
        tcase_add_test (testcase, func);        \
    } while(0);


START_TEST (int10_5bits)
{
    unsigned char len   = 0xFF;
    unsigned char tmp[] = {0,0};

    /* 4.1.1.1.  Example 1: Encoding 10 using a 5-bit prefix
     */
    integer_encode (5, 10, tmp, &len);

    /* Check output */
    ck_assert_int_eq (len, 1);
    ck_assert_int_eq (tmp[0], 10);
}
END_TEST

START_TEST (int1337_5bits)
{
    unsigned char len   = 0xFF;
    unsigned char tmp[] = {0,0,0,0,0};

    /* 4.1.1.2.  Example 2: Encoding 1337 using a 5-bit prefix
     */
    integer_encode (5, 1337, tmp, &len);

    /* Check output */
    ck_assert_int_eq (len, 3);
    ck_assert_int_eq (tmp[0], 31);
    ck_assert_int_eq (tmp[1], 154);
    ck_assert_int_eq (tmp[2], 10);
}
END_TEST

START_TEST (int42_8bits)
{
    unsigned char len   = 0xFF;
    unsigned char tmp[] = {0,0};

    /* 4.1.1.3.  Example 3: Encoding 42 starting at an octet-boundary
     */
    integer_encode (8, 42, tmp, &len);

    /* Check output */
    ck_assert_int_eq (len, 1);
    ck_assert_int_eq (tmp[0], 42);
}
END_TEST

START_TEST (int12_6bits)
{
    unsigned char len   = 0xFF;
    unsigned char tmp[] = {0xC0,0};

    integer_encode (6, 12, tmp, &len);

    /* Check output */
    ck_assert_int_eq (len, 1);
    ck_assert_int_eq (tmp[0], 0xC0 | 12);
}
END_TEST

START_TEST (int1338_5bits)
{
    unsigned char len   = 0xFF;
    unsigned char tmp[] = {0xA0,0,0,0,0};

    integer_encode (5, 1338, tmp, &len);

    /* Check output */
    ck_assert_int_eq (len, 3);
    ck_assert_int_eq (tmp[0], 0xA0 | 31);
    ck_assert_int_eq (tmp[1], 155);
    ck_assert_int_eq (tmp[2], 10);
}
END_TEST


int
main (void)
{
    Suite   *s1    = suite_create("Integer Encoding");
    SRunner *sr    = srunner_create(s1);
    TCase   *tc1_1 = tcase_create("5bits prefix");
    TCase   *tc1_2 = tcase_create("5bits prefix w/ extra");
    TCase   *tc1_3 = tcase_create("8bits prefix");
    TCase   *tc1_4 = tcase_create("6bits prefix and data");
    TCase   *tc1_5 = tcase_create("5bits prefix w/ extra and data");

    check_add_tc (s1, tc1_1, int10_5bits);
    check_add_tc (s1, tc1_2, int1337_5bits);
    check_add_tc (s1, tc1_3, int42_8bits);
    check_add_tc (s1, tc1_4, int12_6bits);
    check_add_tc (s1, tc1_5, int1338_5bits);

    srunner_run_all(sr, CK_VERBOSE);
    return srunner_ntests_failed(sr);
}
