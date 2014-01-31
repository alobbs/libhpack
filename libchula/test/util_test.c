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

#include "libchula/testing_macros.h"
#include "libchula/util.h"


START_TEST (_strerror_r)
{
    char *s;
    int   re;
    char  tmp[256];

    /* 13: Permission denied */
    memset (tmp, 0, sizeof(tmp));
    s = chula_strerror_r (13, tmp, sizeof(tmp));
    ck_assert (s != NULL);

    re = memcmp (s, "Permission denied", strlen(s));
    ck_assert (re == 0);

    /* 98765 */
    memset (tmp, 0, sizeof(tmp));
    s = chula_strerror_r (98765, tmp, sizeof(tmp));
    ck_assert (s != NULL);

    re = memcmp (s, "Unknown error", 13);
    ck_assert (re == 0);
}
END_TEST

START_TEST (min_str)
{
    char *A = "a";
    char *B = "b";

    ck_assert (chula_min_str(NULL,NULL) == NULL);
    ck_assert (chula_min_str(A,NULL) == A);
    ck_assert (chula_min_str(NULL,A) == A);
    ck_assert (chula_min_str(A,B) != NULL);
}
END_TEST

START_TEST (max_str)
{
    char *A = "a";
    char *B = "b";

    ck_assert (chula_max_str(NULL,NULL) == NULL);
    ck_assert (chula_max_str(A,NULL) == A);
    ck_assert (chula_max_str(NULL,A) == A);
    ck_assert (chula_max_str(A,B) != NULL);
}
END_TEST

START_TEST (_atoi)
{
    int   re;
    ret_t ret;

    ret = chula_atoi (NULL, &re);
    ck_assert (ret == ret_error);

    ret = chula_atoi ("", &re);
    ck_assert (ret == ret_error);

    ret = chula_atoi ("123", &re);
    ck_assert (re == 123);
    ck_assert (ret == ret_ok);

    ret = chula_atoi ("-321", &re);
    ck_assert (re == -321);
    ck_assert (ret == ret_ok);
}
END_TEST

START_TEST (_atob)
{
    bool  re;
    ret_t ret;

    ret = chula_atoi (NULL, &re);
    ck_assert (ret == ret_error);

    ret = chula_atoi ("", &re);
    ck_assert (ret == ret_error);

    ret = chula_atoi ("123", &re);
    ck_assert (re == true);
    ck_assert (ret == ret_ok);

    ret = chula_atoi ("0", &re);
    ck_assert (re == false);
    ck_assert (ret == ret_ok);

    ret = chula_atoi ("-0", &re);
    ck_assert (re == false);
    ck_assert (ret == ret_ok);

    ret = chula_atoi ("-321", &re);
    ck_assert (re == true);
    ck_assert (ret == ret_ok);
}
END_TEST

int
util_tests (void)
{
    Suite *s1 = suite_create("AVL");

    check_add (s1, _strerror_r);
    check_add (s1, min_str);
    check_add (s1, max_str);
    check_add (s1, _atoi);
    check_add (s1, _atob);
    run_test (s1);
}
