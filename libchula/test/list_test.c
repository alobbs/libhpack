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

typedef struct {
    chula_list_t base;
    int          value;
} test_entry_t;


static int
entry_cmp (chula_list_t *a, chula_list_t *b)
{
    int a_ = ((test_entry_t *)a)->value;
    int b_ = ((test_entry_t *)b)->value;
    return (a_ > b_) - (a_ < b_);
}

START_TEST (empty)
{
    chula_list_t l = LIST_HEAD_INIT(l);
    ch_assert (chula_list_empty(&l));
}
END_TEST

START_TEST (add)
{
    chula_list_t l = LIST_HEAD_INIT(l);
    test_entry_t e1 = {.base = LIST_HEAD_INIT(e1.base), .value = 1};
    test_entry_t e2 = {.base = LIST_HEAD_INIT(e2.base), .value = 2};
    test_entry_t e3 = {.base = LIST_HEAD_INIT(e3.base), .value = 3};

    chula_list_add (LIST(&e1), &l);
    chula_list_add (LIST(&e2), &l);
    chula_list_add (LIST(&e3), &l);

    ch_assert (((test_entry_t *)(l.next))->value == 3);
    ch_assert (((test_entry_t *)(l.next->next))->value == 2);
    ch_assert (((test_entry_t *)(l.next->next->next))->value == 1);
    ch_assert (l.next->next->next->next == &l);
}
END_TEST

START_TEST (add_tail)
{
    chula_list_t l = LIST_HEAD_INIT(l);
    test_entry_t e1 = {.base = LIST_HEAD_INIT(e1.base), .value = 1};
    test_entry_t e2 = {.base = LIST_HEAD_INIT(e2.base), .value = 2};
    test_entry_t e3 = {.base = LIST_HEAD_INIT(e3.base), .value = 3};

    chula_list_add_tail (LIST(&e1), &l);
    chula_list_add_tail (LIST(&e2), &l);
    chula_list_add_tail (LIST(&e3), &l);

    ch_assert (((test_entry_t *)(l.next))->value == 1);
    ch_assert (((test_entry_t *)(l.next->next))->value == 2);
    ch_assert (((test_entry_t *)(l.next->next->next))->value == 3);
    ch_assert (l.next->next->next->next == &l);
}
END_TEST

START_TEST (del)
{
    chula_list_t l = LIST_HEAD_INIT(l);
    test_entry_t e1 = {.base = LIST_HEAD_INIT(e1.base), .value = 1};
    test_entry_t e2 = {.base = LIST_HEAD_INIT(e2.base), .value = 2};
    test_entry_t e3 = {.base = LIST_HEAD_INIT(e3.base), .value = 3};

    chula_list_add_tail (LIST(&e1), &l);
    chula_list_add_tail (LIST(&e2), &l);
    chula_list_add_tail (LIST(&e3), &l);

    chula_list_del (LIST(&e2));

    ch_assert (((test_entry_t *)(l.next))->value == 1);
    ch_assert (((test_entry_t *)(l.next->next))->value == 3);
}
END_TEST

START_TEST (reparent)
{
    chula_list_t *i, *tmp;
    chula_list_t l1 = LIST_HEAD_INIT(l1);
    chula_list_t l2 = LIST_HEAD_INIT(l2);
    test_entry_t e1 = {.base = LIST_HEAD_INIT(e1.base), .value = 1};
    test_entry_t e2 = {.base = LIST_HEAD_INIT(e2.base), .value = 2};
    test_entry_t e3 = {.base = LIST_HEAD_INIT(e3.base), .value = 3};

    chula_list_add_tail (LIST(&e1), &l1);
    chula_list_add_tail (LIST(&e2), &l1);
    chula_list_add_tail (LIST(&e3), &l1);

    list_for_each_safe (i, tmp, &l1) {
        chula_list_add_tail (i, &l2);
    }

    chula_list_reparent (&l2, &l1);

    ch_assert (((test_entry_t *)(l2.next))->value == 1);
    ch_assert (((test_entry_t *)(l2.next->next))->value == 2);
    ch_assert (((test_entry_t *)(l2.next->next->next))->value == 3);
}
END_TEST

START_TEST (sort)
{
    chula_list_t l  = LIST_HEAD_INIT(l);
    test_entry_t e1 = {.base = LIST_HEAD_INIT(e1.base), .value = 3};
    test_entry_t e2 = {.base = LIST_HEAD_INIT(e2.base), .value = 2};
    test_entry_t e3 = {.base = LIST_HEAD_INIT(e3.base), .value = 1};
    test_entry_t e4 = {.base = LIST_HEAD_INIT(e4.base), .value = 8};

    /* Empty */
    ch_assert (chula_list_empty(&l));
    chula_list_sort (&l, entry_cmp);
    ch_assert (chula_list_empty(&l));

    /* Sort a list */
    chula_list_add_tail (LIST(&e1), &l);
    chula_list_add_tail (LIST(&e2), &l);
    chula_list_add_tail (LIST(&e3), &l);
    chula_list_add_tail (LIST(&e4), &l);

    chula_list_sort (&l, entry_cmp);

    ch_assert (((test_entry_t *)(l.next))->value == 1);
    ch_assert (((test_entry_t *)(l.next->next))->value == 2);
    ch_assert (((test_entry_t *)(l.next->next->next))->value == 3);
    ch_assert (((test_entry_t *)(l.next->next->next->next))->value == 8);
}
END_TEST

START_TEST (get_len)
{
    size_t       len = 99;
    chula_list_t l   = LIST_HEAD_INIT(l);
    test_entry_t e1  = {.base = LIST_HEAD_INIT(e1.base), .value = 3};
    test_entry_t e2  = {.base = LIST_HEAD_INIT(e2.base), .value = 2};

    ch_assert (chula_list_empty(&l));
    chula_list_get_len (&l, &len);
    ch_assert (len == 0);

    chula_list_add_tail (LIST(&e1), &l);
    chula_list_get_len (&l, &len);
    ch_assert (len == 1);

    chula_list_add_tail (LIST(&e2), &l);
    chula_list_get_len (&l, &len);
    ch_assert (len == 2);
}
END_TEST

START_TEST (invert)
{
    chula_list_t l  = LIST_HEAD_INIT(l);
    test_entry_t e1 = {.base = LIST_HEAD_INIT(e1.base), .value = 1};
    test_entry_t e2 = {.base = LIST_HEAD_INIT(e2.base), .value = 2};
    test_entry_t e3 = {.base = LIST_HEAD_INIT(e3.base), .value = 3};
    test_entry_t e4 = {.base = LIST_HEAD_INIT(e4.base), .value = 4};

    /* Sort a list */
    chula_list_add_tail (LIST(&e1), &l);
    chula_list_add_tail (LIST(&e2), &l);
    chula_list_add_tail (LIST(&e3), &l);
    chula_list_add_tail (LIST(&e4), &l);

    chula_list_invert (&l);

    ch_assert (((test_entry_t *)(l.next))->value == 4);
    ch_assert (((test_entry_t *)(l.next->next))->value == 3);
    ch_assert (((test_entry_t *)(l.next->next->next))->value == 2);
    ch_assert (((test_entry_t *)(l.next->next->next->next))->value == 1);
}
END_TEST

START_TEST (content_add)
{
    chula_list_t l = LIST_HEAD_INIT(l);

    chula_list_content_add (&l, INT_TO_POINTER(NULL));
    chula_list_content_add (&l, INT_TO_POINTER(1));
    chula_list_content_add (&l, INT_TO_POINTER(2));
    chula_list_content_add (&l, INT_TO_POINTER(3));

    ch_assert (POINTER_TO_INT (LIST_ITEM_INFO(l.next)) == 3);
    ch_assert (POINTER_TO_INT (LIST_ITEM_INFO(l.next->next)) == 2);
    ch_assert (POINTER_TO_INT (LIST_ITEM_INFO(l.next->next->next)) == 1);
    ch_assert (LIST_ITEM_INFO(l.next->next->next->next) == NULL);

    chula_list_content_free (&l, NULL);
}
END_TEST

START_TEST (content_add_tail)
{
    chula_list_t l = LIST_HEAD_INIT(l);

    chula_list_content_add_tail (&l, INT_TO_POINTER(NULL));
    chula_list_content_add_tail (&l, INT_TO_POINTER(1));
    chula_list_content_add_tail (&l, INT_TO_POINTER(2));
    chula_list_content_add_tail (&l, INT_TO_POINTER(3));

    ch_assert (LIST_ITEM_INFO(l.next) == NULL);
    ch_assert (POINTER_TO_INT (LIST_ITEM_INFO(l.next->next)) == 1);
    ch_assert (POINTER_TO_INT (LIST_ITEM_INFO(l.next->next->next)) == 2);
    ch_assert (POINTER_TO_INT (LIST_ITEM_INFO(l.next->next->next->next)) == 3);

    chula_list_content_free (&l, NULL);
}
END_TEST

START_TEST (content_free)
{
    size_t       len = 99;
    chula_list_t l   = LIST_HEAD_INIT(l);

    chula_list_content_add_tail (&l, strndup("uno", 3));
    chula_list_content_add_tail (&l, strndup("dos", 3));
    chula_list_content_add_tail (&l, strndup("tres", 4));
    chula_list_content_add_tail (&l, strndup("cuatro", 5));

    chula_list_content_free (&l, free);

    chula_list_get_len (&l, &len);
    ch_assert (len == 0);

    chula_list_content_free (&l, NULL);
}
END_TEST

START_TEST (content_free_item_simple)
{
    chula_list_t *i, *tmp;
    size_t       len = 99;
    chula_list_t l   = LIST_HEAD_INIT(l);

    chula_list_content_add_tail (&l, strndup("uno", 3));
    chula_list_content_add_tail (&l, strndup("dos", 3));
    chula_list_content_add_tail (&l, strndup("tres", 4));
    chula_list_content_add_tail (&l, strndup("cuatro", 5));

    list_for_each_safe (i, tmp, &l) {
        chula_list_content_free_item_simple (i);
    }

    chula_list_get_len (&l, &len);
    ch_assert (len == 0);

    chula_list_content_free (&l, NULL);
}
END_TEST


int
list_tests (void)
{
    Suite *s1 = suite_create("List");
    check_add (s1, empty);
    check_add (s1, add);
    check_add (s1, add_tail);
    check_add (s1, del);
    check_add (s1, reparent);
    check_add (s1, sort);
    check_add (s1, get_len);
    check_add (s1, invert);
    check_add (s1, content_add);
    check_add (s1, content_add_tail);
    check_add (s1, content_free);
    check_add (s1, content_free_item_simple);
    run_test (s1);
}
