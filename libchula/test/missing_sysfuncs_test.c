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

#include "config.h"
#include "libchula/testing_macros.h"
#include "libchula/missing_sysfuncs.h"

START_TEST (_strsep)
{
    char *string;
    char *tofree;
    char *token;

    string = strdup ("Hello  there,stranger");
    ck_assert (string != NULL);
    tofree = string;

    token = strsep (&string, " ,");
    ck_assert (token != NULL);
    ck_assert_str_eq (token, "Hello");

    token = strsep (&string, " ,");
    ck_assert (token != NULL);
    ck_assert_str_eq (token, "");

    token = strsep (&string, " ,");
    ck_assert (token != NULL);
    ck_assert_str_eq (token, "there");

    token = strsep (&string, " ,");
    ck_assert (token != NULL);
    ck_assert_str_eq (token, "stranger");

    token = strsep (&string, " ,");
    ck_assert (token == NULL);

    free (tofree);
}
END_TEST

START_TEST (_strnstr)
{
    char *p;
    char *string = "This is a testing string";

    p = strnstr (string, "is a", strlen(string));
    ck_assert (p != NULL);

    p = strnstr (string, string, strlen(string));
    ck_assert (p != NULL);
    ck_assert (p == string);

    p = strnstr (string, "testing", 9);
    ck_assert (p == NULL);

    p = strnstr (string, "foobar", strlen(string));
    ck_assert (p == NULL);
}
END_TEST

START_TEST (_strcasestr)
{
    char *p;
    char *string = "This is a testing string";

    p = strcasestr (string, "iS A");
    ck_assert (p != NULL);

    p = strcasestr (string, "tHIS IS A TESTING STRiNG");
    ck_assert (p != NULL);
    ck_assert (p == string);

    p = strcasestr (string, string);
    ck_assert (p != NULL);
    ck_assert (p == string);

    p = strcasestr (string, "TeStInG");
    ck_assert (p != NULL);

    p = strcasestr (string, "foobar");
    ck_assert (p == NULL);
}
END_TEST

START_TEST (_strncasestrn)
{
    char *p;
    char *string    = "This is a testing string";
    int   stringlen = strlen(string);

    p = strncasestrn (string, stringlen, "iS A", 4);
    ck_assert (p != NULL);

    p = strncasestrn (string, stringlen, "tHIS IS A TESTING STRiNG", stringlen);
    ck_assert (p != NULL);
    ck_assert (p == string);

    p = strncasestrn (string, stringlen, string, stringlen);
    ck_assert (p != NULL);
    ck_assert (p == string);

    p = strncasestrn (string, stringlen, "TeStInG", 7);
    ck_assert (p != NULL);

    p = strncasestrn (string, stringlen, "is a trick that I'm playing", 4);
    ck_assert (p != NULL);

    p = strncasestrn (string, stringlen, "foobar", 6);
    ck_assert (p == NULL);
}
END_TEST

START_TEST (_strncasestr)
{
    char *p;
    char *string    = "This is a testing string";
    int   stringlen = strlen(string);

    p = strncasestr (string, "iS A", stringlen);
    ck_assert (p != NULL);
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


    len = strlcat (s1, s2, sizeof(s1));
    ck_assert_str_eq (s1, "hi there!");
    ck_assert (len == strlen("hi there!"));

    /* Too long */
    strncpy (s2, "12345678901234567890", sizeof(s2));
    len = strlcat (s1, s2, sizeof(s1));
    ck_assert (len > sizeof(s1));
}
END_TEST


int
missing_sysfuncs_tests (void)
{
    Suite *s1 = suite_create("Missing System Functions");
    check_add (s1, _strsep);
    check_add (s1, _strnstr);
    check_add (s1, _strcasestr);
    check_add (s1, _strncasestrn);
    check_add (s1, _strncasestr);
    check_add (s1, _strlcat);
    run_test (s1);
}
