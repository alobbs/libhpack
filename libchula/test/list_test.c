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
#include "testing_macros.h"
#include "libchula/list.h"

typedef struct {
    chula_list_t base;
    int          value;
} test_entry_t;

START_TEST (empty)
{
    chula_list_t l = LIST_HEAD_INIT(l);
    ck_assert (chula_list_empty(&l));
}
END_TEST

START_TEST (add)
{
    chula_list_t l = LIST_HEAD_INIT(l);
    test_entry_t e1 = {.base = LIST_HEAD_INIT(e1), .value = 1};
    test_entry_t e2 = {.base = LIST_HEAD_INIT(e2), .value = 2};
    test_entry_t e3 = {.base = LIST_HEAD_INIT(e3), .value = 3};

    chula_list_add (&e1, &l);
    chula_list_add (&e2, &l);
    chula_list_add (&e3, &l);

    ck_assert (((test_entry_t *)(l.next))->value == 3);
    ck_assert (((test_entry_t *)(l.next->next))->value == 2);
    ck_assert (((test_entry_t *)(l.next->next->next))->value == 1);
    ck_assert (l.next->next->next->next == &l);
}
END_TEST

START_TEST (add_tail)
{
    chula_list_t l = LIST_HEAD_INIT(l);
    test_entry_t e1 = {.base = LIST_HEAD_INIT(e1), .value = 1};
    test_entry_t e2 = {.base = LIST_HEAD_INIT(e2), .value = 2};
    test_entry_t e3 = {.base = LIST_HEAD_INIT(e3), .value = 3};

    chula_list_add_tail (&e1, &l);
    chula_list_add_tail (&e2, &l);
    chula_list_add_tail (&e3, &l);

    ck_assert (((test_entry_t *)(l.next))->value == 1);
    ck_assert (((test_entry_t *)(l.next->next))->value == 2);
    ck_assert (((test_entry_t *)(l.next->next->next))->value == 3);
    ck_assert (l.next->next->next->next == &l);
}
END_TEST

int
list_tests (void)
{
    Suite *s1 = suite_create("List");
    check_add (s1, empty);
    check_add (s1, add);
    check_add (s1, add_tail);
    run_test (s1);
}
