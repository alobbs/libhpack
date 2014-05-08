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
#include "config.h"


START_TEST (_strsep)
{
    char *string;
    char *tofree;
    char *token;

    string = strdup ("Hello  there,stranger");
    ch_assert (string != NULL);
    tofree = string;

    token = chula_strsep (&string, " ,");
    ch_assert (token != NULL);
    ch_assert_str_eq (token, "Hello");

    token = chula_strsep (&string, " ,");
    ch_assert (token != NULL);
    ch_assert_str_eq (token, "");

    token = chula_strsep (&string, " ,");
    ch_assert (token != NULL);
    ch_assert_str_eq (token, "there");

    token = chula_strsep (&string, " ,");
    ch_assert (token != NULL);
    ch_assert_str_eq (token, "stranger");

    token = chula_strsep (&string, " ,");
    ch_assert (token == NULL);

    free (tofree);
}
END_TEST

START_TEST (_strnstr)
{
    char *p;
    char *string = "This is a testing string";

    p = chula_strnstr (string, "is a", strlen(string));
    ch_assert (p != NULL);

    p = chula_strnstr (string, string, strlen(string));
    ch_assert (p != NULL);
    ch_assert (p == string);

    p = chula_strnstr (string, "testing", 9);
    ch_assert (p == NULL);

    p = chula_strnstr (string, "foobar", strlen(string));
    ch_assert (p == NULL);
}
END_TEST

START_TEST (_strcasestr)
{
    char *p;
    char *string = "This is a testing string";

    p = chula_strcasestr (string, "iS A");
    ch_assert (p != NULL);

    p = chula_strcasestr (string, "tHIS IS A TESTING STRiNG");
    ch_assert (p != NULL);
    ch_assert (p == string);

    p = chula_strcasestr (string, string);
    ch_assert (p != NULL);
    ch_assert (p == string);

    p = chula_strcasestr (string, "TeStInG");
    ch_assert (p != NULL);

    p = chula_strcasestr (string, "foobar");
    ch_assert (p == NULL);
}
END_TEST

START_TEST (_strncasestrn)
{
    char *p;
    char *string    = "This is a testing string";
    int   stringlen = strlen(string);

    p = chula_strncasestrn (string, stringlen, "iS A", 4);
    ch_assert (p != NULL);

    p = chula_strncasestrn (string, stringlen, "tHIS IS A TESTING STRiNG", stringlen);
    ch_assert (p != NULL);
    ch_assert (p == string);

    p = chula_strncasestrn (string, stringlen, string, stringlen);
    ch_assert (p != NULL);
    ch_assert (p == string);

    p = chula_strncasestrn (string, stringlen, "TeStInG", 7);
    ch_assert (p != NULL);

    p = chula_strncasestrn (string, stringlen, "is a trick that I'm playing", 4);
    ch_assert (p != NULL);

    p = chula_strncasestrn (string, stringlen, "foobar", 6);
    ch_assert (p == NULL);
}
END_TEST

START_TEST (_strncasestr)
{
    char *p;
    char *string    = "This is a testing string";
    int   stringlen = strlen(string);

    p = chula_strncasestr (string, "iS A", stringlen);
    ch_assert (p != NULL);
}
END_TEST

START_TEST (_strlcat)
{
    size_t len;
    char   s1[12];
    char   s2[24];

    /* Concat */
    strncpy (s1, "hi ", sizeof(s1));
    strncpy (s2, "there!", sizeof(s2));

    len = chula_strlcat (s1, s2, sizeof(s1));
    ch_assert_str_eq (s1, "hi there!");
    ch_assert (len == strlen("hi there!"));

    /* Too long */
    strncpy (s2, "12345678901234567890", sizeof(s2));
    len = chula_strlcat (s1, s2, sizeof(s1));
    ch_assert (len > sizeof(s1));
}
END_TEST


int
cstrings_tests (void)
{
    Suite *s1 = suite_create("C-Strings Functions");
    check_add (s1, _strsep);
    check_add (s1, _strlcat);
    check_add (s1, _strnstr);
    check_add (s1, _strcasestr);
    check_add (s1, _strncasestr);
    check_add (s1, _strncasestrn);
    run_test (s1);
}
