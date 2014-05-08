/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/* All files in libhpack are Copyright (C) 2014 Alvaro Lopez Ortega.
 *
 *   Authors:
 *     * Alvaro Lopez Ortega <alvaro@gnu.org>
 *     * Gorka Eguileor <gorka@eguileor.com>
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

#include <string.h>

#include <libhpack/libhpack.h>
#include <libchula-qa/libchula-qa.h>
#include <libchula-qa/testing_macros-internal.h>


#define EVEN_SET(E) memset (&E, 0x55, sizeof(E));
#define ODD_SET(E)  memset (&E, 0xAA, sizeof(E));

START_TEST (_init_empty)
{
    hpack_set_t b_set;

    hpack_set_init (b_set, false);

    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i)
        ch_assert (0 == b_set[i]);
}
END_TEST

START_TEST (_init_full)
{
    hpack_set_t b_set;

    hpack_set_init (b_set, true);

    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i)
        ch_assert ((hpack_set_entry_t) ~0 == b_set[i]);
}
END_TEST

START_TEST (_is_empty)
{
    hpack_set_t b_set;

    hpack_set_init (b_set, false);
    ch_assert (hpack_set_is_empty (b_set));
}
END_TEST

START_TEST (_is_full)
{
    hpack_set_t b_set;

    hpack_set_init (b_set, true);
    ch_assert (hpack_set_is_full (b_set));
}
END_TEST

START_TEST (_add_all)
{
    ret_t              ret;
    hpack_set_t b_set;

    hpack_set_init (b_set, false);

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; ++i) {
        ret = hpack_set_add (b_set, i);
        ch_assert (ret == ret_ok);
    }

    ch_assert (hpack_set_is_full (b_set));
}
END_TEST

START_TEST (_add_even)
{
    ret_t             ret;
    hpack_set_t       b_set;
    hpack_set_entry_t e;

    hpack_set_init (b_set, false);

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; i+=2) {
        ret = hpack_set_add (b_set, i);
        ch_assert (ret == ret_ok);
    }

    EVEN_SET(e);

    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i)
        ch_assert (e == b_set[i]);
}
END_TEST

START_TEST (_add_odd)
{
    ret_t             ret;
    hpack_set_t       b_set;
    hpack_set_entry_t e;

    hpack_set_init (b_set, false);

    for (unsigned int i=1; i < HPACK_MAX_HEADER_TABLE_ENTRIES; i+=2) {
        ret = hpack_set_add (b_set, i);
        ch_assert (ret == ret_ok);
    }

    ODD_SET(e);

    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i)
        ch_assert (e == b_set[i]);
}
END_TEST

START_TEST (_add_fail)
{
    ret_t       ret;
    hpack_set_t b_set;

    hpack_set_init (b_set, false);

    ret = hpack_set_add (b_set, HPACK_MAX_HEADER_TABLE_ENTRIES);
    ch_assert (ret == ret_error);
}
END_TEST

START_TEST (_remove_all)
{
    ret_t       ret;
    hpack_set_t b_set;

    hpack_set_init (b_set, true);

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; ++i) {
        ret = hpack_set_remove (b_set, i);
        ch_assert (ret == ret_ok);
    }

    ch_assert (hpack_set_is_empty (b_set));
}
END_TEST

START_TEST (_remove_even)
{
    ret_t             ret;
    hpack_set_t       b_set;
    hpack_set_entry_t e;

    hpack_set_init (b_set, true);

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; i+=2) {
        ret = hpack_set_remove (b_set, i);
        ch_assert (ret == ret_ok);
    }

    ODD_SET(e);

    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i)
        ch_assert (e == b_set[i]);
}
END_TEST

START_TEST (_remove_odd)
{
    ret_t             ret;
    hpack_set_t       b_set;
    hpack_set_entry_t e;

    hpack_set_init (b_set, true);

    for (int i=1; i<HPACK_MAX_HEADER_TABLE_ENTRIES; i+=2) {
        ret = hpack_set_remove (b_set, i);
        ch_assert (ret == ret_ok);
    }

    EVEN_SET(e);

    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i)
        ch_assert (e == b_set[i]);
}
END_TEST

START_TEST (_remove_fail)
{
    ret_t       ret;
    hpack_set_t b_set;

    hpack_set_init (b_set, true);

    ret = hpack_set_remove (b_set, HPACK_MAX_HEADER_TABLE_ENTRIES);
    ch_assert (ret == ret_error);
}
END_TEST

START_TEST (_equals_each)
{
    ret_t       ret;
    hpack_set_t b_set1;
    hpack_set_t b_set2;

    hpack_set_init (b_set1, false);
    hpack_set_init (b_set2, false);

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; ++i) {
        ret = hpack_set_add (b_set1, i);
        ch_assert (ret == ret_ok);
        hpack_set_set (b_set2, b_set1);

        ch_assert (hpack_set_equals (b_set1, b_set2));
        hpack_set_clear (b_set1);
    }
}
END_TEST

START_TEST (_equals_all)
{
    ret_t       ret;
    hpack_set_t b_set1;
    hpack_set_t b_set2;

    hpack_set_init (b_set1, false);
    hpack_set_init (b_set2, false);

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; ++i) {
        ret = hpack_set_add (b_set1, i);
        ch_assert (ret == ret_ok);

        ret = hpack_set_add (b_set2, i);
        ch_assert (ret == ret_ok);

        ch_assert (hpack_set_equals (b_set1, b_set2));
    }
}
END_TEST

START_TEST (_equals_even)
{
    ret_t       ret;
    hpack_set_t b_set1;
    hpack_set_t b_set2;

    hpack_set_init (b_set1, false);
    hpack_set_init (b_set2, false);

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; i+=2) {
        ret = hpack_set_add (b_set1, i);
        ch_assert (ret == ret_ok);

        ret = hpack_set_add (b_set2, i);
        ch_assert (ret == ret_ok);

        ch_assert (hpack_set_equals (b_set1, b_set2));
    }
}
END_TEST

START_TEST (_equals_odd)
{
    ret_t       ret;
    hpack_set_t b_set1;
    hpack_set_t b_set2;

    hpack_set_init (b_set1, false);
    hpack_set_init (b_set2, false);

    for (int i=1; i < HPACK_MAX_HEADER_TABLE_ENTRIES; i+=2) {
        ret = hpack_set_add (b_set1, i);
        ch_assert (ret == ret_ok);

        ret = hpack_set_add (b_set2, i);
        ch_assert (ret == ret_ok);

        ch_assert (hpack_set_equals (b_set1, b_set2));
    }
}
END_TEST

START_TEST (_union)
{
    ret_t       ret;
    hpack_set_t b_set1;
    hpack_set_t b_set2;
    hpack_set_t b_set3;

    hpack_set_init (b_set1, false);
    hpack_set_init (b_set2, false);
    hpack_set_init (b_set3, false);

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; ++i) {
        ret = hpack_set_add (i&1?b_set1:b_set2, i);
        ck_assert (ret == ret_ok);
    }

    hpack_set_union (b_set3, b_set1);
    hpack_set_union (b_set3, b_set2);

    ck_assert (hpack_set_is_full (b_set3));
}
END_TEST

START_TEST (_remove_set)
{
    ret_t       ret;
    hpack_set_t b_set1;
    hpack_set_t b_set2;
    hpack_set_t b_set3;

    hpack_set_init (b_set1, false);
    hpack_set_init (b_set2, false);
    hpack_set_init (b_set3, true);

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; ++i) {
        ret = hpack_set_add (i&1?b_set1:b_set2, i);
        ck_assert (ret == ret_ok);
    }

    hpack_set_relative_comp (b_set3, b_set1);
    hpack_set_relative_comp (b_set3, b_set2);

    ck_assert (hpack_set_is_empty (b_set3));
}
END_TEST

START_TEST (_intersection1)
{
    ret_t       ret;
    hpack_set_t b_set1;
    hpack_set_t b_set2;

    hpack_set_init (b_set1, false);
    hpack_set_init (b_set2, false);

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; ++i) {
        ret = hpack_set_add (i&1?b_set1:b_set2, i);
        ck_assert (ret == ret_ok);
    }

    hpack_set_intersection (b_set1, b_set2);

    ck_assert (hpack_set_is_empty (b_set1));
}
END_TEST

START_TEST (_intersection2)
{
    ret_t       ret;
    hpack_set_t b_set1;
    hpack_set_t b_set2;

    hpack_set_init (b_set1, false);
    hpack_set_init (b_set2, false);

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; i+=2) {
        ret = hpack_set_add (b_set1, i);
        ck_assert (ret == ret_ok);

        ret = hpack_set_add (b_set2, i);
        ck_assert (ret == ret_ok);
    }

    hpack_set_intersection (b_set1, b_set2);

    ck_assert (hpack_set_equals (b_set1, b_set2));
}
END_TEST

START_TEST (_complement)
{
    hpack_set_t b_set1;

    hpack_set_init (b_set1, false);

    hpack_set_complement (b_set1);
    ck_assert (hpack_set_is_full (b_set1));

    hpack_set_complement (b_set1);
    ck_assert (hpack_set_is_empty (b_set1));
}
END_TEST

START_TEST (_exists)
{
    ret_t       ret;
    hpack_set_t b_set1;
    hpack_set_t b_set2;

    hpack_set_init (b_set1, false);
    hpack_set_init (b_set2, false);

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; ++i) {
        ret = hpack_set_add (i&1?b_set1:b_set2, i);
        ck_assert (ret == ret_ok);
    }

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; ++i) {
        ck_assert (hpack_set_exists (i&1?b_set1:b_set2, i));
        ck_assert (!hpack_set_exists (i&1?b_set2:b_set1, i));
    }
}
END_TEST

START_TEST (_exists_fail)
{
    hpack_set_t b_set;

    hpack_set_init (b_set, true);

    ck_assert (!hpack_set_exists (b_set, HPACK_MAX_HEADER_TABLE_ENTRIES));
}
END_TEST

START_TEST (_set)
{
    ret_t       ret;
    hpack_set_t b_set1;
    hpack_set_t b_set2;

    hpack_set_init (b_set1, false);
    hpack_set_init (b_set2, false);

    for (int i=0; i < HPACK_MAX_HEADER_TABLE_ENTRIES; ++i) {
        ret = hpack_set_add (i&1?b_set1:b_set2, i);
        ck_assert (ret == ret_ok);
    }

    hpack_set_set (b_set1, b_set2);

    ck_assert (hpack_set_equals (b_set1, b_set2));
}
END_TEST

START_TEST (_iter_all)
{
    hpack_set_t          b_set;
    hpack_set_iterator_t iter;
    int                  i;
    int                  i_next;

    hpack_set_init (b_set, true);

    i = 0;
    hpack_set_iter_init (&iter, b_set);

    do {
        i_next = hpack_set_iter_next (&iter);
        if (HPACK_MAX_HEADER_TABLE_ENTRIES == i) {
            ch_assert (-1 == i_next);
        } else {
            ch_assert (i_next == i);
        }
        ++i;
    } while (-1 != i_next);

    /* Confirm that the original set is unaltered */
    ch_assert (hpack_set_is_full (b_set));
}
END_TEST

START_TEST (_iter_odd)
{
    hpack_set_t          b_set1;
    hpack_set_t          b_set2;
    hpack_set_iterator_t iter;
    int                  i;
    int                  i_next;

    hpack_set_init (b_set1, false);

    for (i=1; i < HPACK_MAX_HEADER_TABLE_ENTRIES; i += 2)
        hpack_set_add (b_set1, i);

    hpack_set_set (b_set2, b_set1);

    i = 1;
    hpack_set_iter_init (&iter, b_set1);

    do {
        i_next = hpack_set_iter_next (&iter);

        if (HPACK_MAX_HEADER_TABLE_ENTRIES < i) {
            ch_assert (-1 == i_next);
        } else {
            ch_assert (i_next == i);
        }
        i += 2;
    } while (-1 != i_next);

    /* Confirm that the original set is unaltered */
    ch_assert (hpack_set_equals (b_set1, b_set2));
}
END_TEST


int
sets (void)
{
    Suite *s1 = suite_create("Sets");
    check_add (s1, _init_empty);
    check_add (s1, _init_full);

    check_add (s1, _is_empty);
    check_add (s1, _is_full);

    check_add (s1, _add_all);
    check_add (s1, _add_even);
    check_add (s1, _add_odd);
    check_add (s1, _add_fail);

    check_add (s1, _remove_all);
    check_add (s1, _remove_even);
    check_add (s1, _remove_odd);
    check_add (s1, _remove_fail);

    check_add (s1, _equals_each);
    check_add (s1, _equals_all);
    check_add (s1, _equals_even);
    check_add (s1, _equals_odd);

    check_add (s1, _union);
    check_add (s1, _remove_set);
    check_add (s1, _intersection1);
    check_add (s1, _intersection2);
    check_add (s1, _complement);
    check_add (s1, _exists);
    check_add (s1, _set);
    check_add (s1, _exists_fail);

    check_add (s1, _iter_all);
    check_add (s1, _iter_odd);

    run_test (s1);
}

int
bitmap_set_tests (void)
{
    int re;

    re = sets();

    return re;
}
