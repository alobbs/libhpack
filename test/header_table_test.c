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

#include <libhpack/libhpack.h>
#include <libchula-qa/libchula-qa.h>
#include <libchula-qa/testing_macros-internal.h>


void check_table_empty (hpack_header_table_t *table)
{
    ch_assert (table->num_headers == 0);
    ch_assert (table->used_data == 0);
    ch_assert (table->headers_offsets.head == table->headers_offsets.tail);
    ch_assert (table->headers_data.head == table->headers_data.tail);
}

START_TEST (_init) {
    ret_t                 ret;
    hpack_header_table_t *table;

    ret = hpack_header_table_new (&table);
    ch_assert (ret == ret_ok);

    ch_assert (table->max_data == SETTINGS_HEADER_TABLE_SIZE);
    check_table_empty (table);

    hpack_header_table_free (table);
}
END_TEST

START_TEST (_clear)
{
    ret_t                  ret;
    hpack_header_table_t *table;

    ret = hpack_header_table_new (&table);
    ch_assert (ret == ret_ok);

    ret += hpack_header_table_clear (table);
    ch_assert (ret == ret_ok);

    ch_assert (table->max_data == SETTINGS_HEADER_TABLE_SIZE);
    check_table_empty (table);

    hpack_header_table_free (table);
}
END_TEST

START_TEST (_set_max_no_table) {
    ret_t                 ret;
    hpack_header_table_t *table;
    hpack_set_t           evicted;

    ret = hpack_header_table_new (&table);
    ch_assert (ret == ret_ok);

    ret = hpack_header_table_set_max (table, 0, evicted);
    ch_assert (ret == ret_ok);

    ch_assert (hpack_set_is_empty (evicted));

    ch_assert (table->max_data == 0);
    check_table_empty (table);

    hpack_header_table_free (table);
}
END_TEST

START_TEST (_set_max_no_table_evict_all) {
    ret_t                 ret;
    hpack_header_table_t *table;
    hpack_header_field_t  field;
    hpack_set_t           evicted;

    ret = hpack_header_table_new (&table);
    ch_assert (ret == ret_ok);

    hpack_header_field_init (&field);

    chula_buffer_add_str (&field.name,  "custom-key");
    chula_buffer_add_str (&field.value, "custom-header");

    for (int i=0; i<10; ++i) {
        ret = hpack_header_table_add (table, &field, evicted);
        ch_assert (ret_ok == ret);

        ch_assert (hpack_set_is_empty(evicted));
    }

    ret = hpack_header_table_set_max (table, 0, evicted);
    ch_assert (ret == ret_ok);

    hpack_set_is_full(evicted);

    ch_assert (table->max_data == 0);
    check_table_empty (table);

    hpack_header_field_mrproper (&field);
    hpack_header_table_free (table);
}
END_TEST

START_TEST (_set_max_no_table_evict_some) {
    ret_t                 ret;
    hpack_header_table_t *table;
    hpack_header_field_t  field;
    hpack_set_t           evicted;
    uint64_t              size;
    uint16_t              used_bytes;
    uint16_t              max_size;
    hpack_set_iterator_t  iter;

    ret = hpack_header_table_new (&table);
    ch_assert (ret == ret_ok);

    hpack_header_field_init (&field);

    chula_buffer_add_str (&field.name,  "custom-key");
    chula_buffer_add_str (&field.value, "custom-header");

    for (int i=0; i<10; ++i) {
        ret = hpack_header_table_add (table, &field, evicted);
        ch_assert (ret_ok == ret);

        /* Confirm that there's been no evictions */
        ch_assert (hpack_set_is_empty (evicted));

        /* Confirm that the number of headers in the table is OK */
        ch_assert (i+1 == table->num_headers);

        /* Confirm that the "used data" is correct */
        ch_assert ((i+1) * (23+32) == table->used_data);

        /* Confirm that the offset head is in place */
        ch_assert (0 == table->headers_offsets.head);

        /* Confirm that the offsets tail is ok */
        ch_assert (i+1 == table->headers_offsets.tail);

        /* Confirm that the data head is in place */
        ch_assert (0 == table->headers_data.head);

        /* Confirm that the data tail is ok */
        ch_assert ((i+1) * (23+sizeof(hpack_header_table_field_info_t)) == table->headers_data.tail);

        /* Confirm that the offset reference is correct */
        ch_assert (table->headers_offsets.buffer[i] == i * (23+sizeof(hpack_header_table_field_info_t)));
    }

    /* We'll leave only space for 3 and 1/2 elements */
    ret = hpack_header_field_get_size (&field, &size);
    ch_assert (ret == ret_ok);

    used_bytes = (uint16_t) size * 3;
    max_size = (uint16_t) size * 3.5;

    ret = hpack_header_table_set_max (table, max_size, evicted);
    ch_assert (ret == ret_ok);

    hpack_header_table_iter_init (&iter, evicted);

    for (int i=0; i<8; ++i) {
        int e = hpack_header_table_iter_next (table, &iter);
        ch_assert (e == (i==7?-1:10-i));
    }

    ch_assert (table->max_data == max_size);
    ch_assert (table->num_headers == 3);

    ch_assert (table->used_data == used_bytes);
    ch_assert (table->headers_offsets.head == 7);
    ch_assert (table->headers_offsets.head == table->headers_offsets.tail - 3);

    /* Calculate the size of each field in table->headers_data */
    size -= HPACK_HEADER_ENTRY_OVERHEAD - sizeof(hpack_header_table_field_info_t);
    ch_assert (table->headers_data.head == size * 7);
    ch_assert (table->headers_data.head == table->headers_data.tail - (size * 3));

    hpack_header_field_mrproper (&field);
    hpack_header_table_free (table);
}
END_TEST


START_TEST (_add_fits) {
    ret_t                  ret;
    hpack_header_table_t  *table;
    hpack_set_t            evicted_set;

    hpack_header_field_t   fields[6];
    hpack_header_field_t   field;

    uint64_t               size;
    uint16_t               real_size;

    ret = hpack_header_table_new (&table);
    ch_assert (ret == ret_ok);

    for (int i=0; i<6; i++) {
        hpack_header_field_init (&fields[i]);
        chula_buffer_add_va  (&fields[i].name, "%c-%d", 'a'+i, i);
        chula_buffer_add_va (&fields[i].value, "foobar-%d", i);
    }

    ret = hpack_header_field_get_size (&fields[0], &size);
    ch_assert (ret == ret_ok);
    real_size = fields[0].name.len + fields[0].value.len + sizeof(hpack_header_table_field_info_t);

    for (int i=0; i<6; i++) {
        ret = hpack_header_table_add (table, &fields[i], evicted_set);
        ch_assert (ret_ok == ret);

        /* Confirm that there's been no evictions */
        ch_assert (hpack_set_is_empty (evicted_set));

        /* Confirm that the number of headers in the table is OK */
        ch_assert (i+1 == table->num_headers);

        /* Confirm that the "used data" is correct */
        ch_assert ((i+1) * size == table->used_data);

        /* Confirm that the offset head is in place */
        ch_assert (0 == table->headers_offsets.head);

        /* Confirm that the offsets tail is ok */
        ch_assert (i+1 == table->headers_offsets.tail);

        /* Confirm that the data head is in place */
        ch_assert (0 == table->headers_data.head);

        /* Confirm that the data tail is ok */
        ch_assert ((i+1) * real_size == table->headers_data.tail);

        /* Confirm that the offset reference is correct */
        ch_assert (table->headers_offsets.buffer[i] == i * real_size);
    }

    ch_assert (table->num_headers == 6);

    /* Check that returned fields are OK */
    hpack_header_field_init (&field);
    for (int i=1; i<7; ++i) {
        bool is_static;
        ret = hpack_header_table_get (table, i, false, &field, &is_static);
        ch_assert (ret == ret_ok);

        ch_assert (field.flags.rep == fields[6-i].flags.rep);
        ch_assert (field.flags.name == fields[6-i].flags.name);
        ch_assert (field.flags.value == fields[6-i].flags.value);
        ch_assert_str_eq (field.name.buf, fields[6-i].name.buf);
        ch_assert_str_eq (field.value.buf, fields[6-i].value.buf);

        hpack_header_field_clean (&field);
    }

    /* Check that table's contents are OK*/
    ret = hpack_header_field_get_size (&fields[0], &size);
    ch_assert (ret == ret_ok);

    ch_assert (table->used_data == 6 * size);
    ch_assert (table->headers_offsets.head == 0);
    ch_assert (table->headers_offsets.head + 6 == table->headers_offsets.tail);

    /* Calculate the size of each field in table->headers_data */
    size -= HPACK_HEADER_ENTRY_OVERHEAD - sizeof(hpack_header_table_field_info_t);
    ch_assert (table->headers_data.head == 0);
    ch_assert (table->headers_data.head == table->headers_data.tail - (size *6));

    /* Clean up */
    hpack_header_field_mrproper (&field);
    for (int i=0; i<6; ++i) {
        hpack_header_field_mrproper (&fields[i]);
    }
    hpack_header_table_free (table);
}
END_TEST

START_TEST (_add_some_evacs) {
    ret_t                  ret;
    hpack_header_table_t  *table;
    hpack_set_t            evicted_set;

    hpack_header_field_t   fields[6];
    hpack_header_field_t   field;

    uint64_t               size;
    uint16_t               max_size;

    ret = hpack_header_table_new (&table);
    ch_assert (ret == ret_ok);

    for (int i=0; i<6; i++) {
        hpack_header_field_init (&fields[i]);
        chula_buffer_add_va  (&fields[i].name, "%c-%d", 'a'+i, i);
        chula_buffer_add_va (&fields[i].value, "foobar-%d", i);
    }

    ret = hpack_header_field_get_size (&fields[0], &size);
    ch_assert (ret == ret_ok);

    max_size = (uint16_t) size * 4.5;

    ret = hpack_header_table_set_max (table, max_size, evicted_set);
    ch_assert (ret == ret_ok);

    for (int i=0; i<6; i++) {
        ret = hpack_header_table_add (table, &fields[i], evicted_set);
        ch_assert (ret_ok == ret);

        if (i < 4) {
            ch_assert (hpack_set_is_empty (evicted_set));

        } else {
            /* Since only 4 items fit, whenever we add more we lose the oldest one, that is the fifth element */
            ch_assert (hpack_header_table_set_exists (table, evicted_set, 5));
        }
    }

    ch_assert (table->num_headers == 4);

    /* Make sure relevant contents are in there */
    hpack_header_field_init (&field);
    for (int i=1; i<5; ++i) {
        bool is_static;

        ret = hpack_header_table_get (table, i, false, &field, &is_static);
        ch_assert (ret == ret_ok);
        ch_assert_str_eq (field.name.buf, fields[6-i].name.buf);
        ch_assert_str_eq (field.value.buf, fields[6-i].value.buf);
        ch_assert (field.flags.rep == fields[6-i].flags.rep);
        ch_assert (field.flags.name == fields[6-i].flags.name);
        ch_assert (field.flags.value == fields[6-i].flags.value);
        hpack_header_field_clean (&field);
    }

    /* Check that table's contents are OK*/
    ch_assert (table->used_data == size * 4);
    ch_assert (table->headers_offsets.head == 2);
    ch_assert (table->headers_offsets.head == table->headers_offsets.tail - 4);

    /* Calculate the size of each field in table->headers_data */
    size -= HPACK_HEADER_ENTRY_OVERHEAD - sizeof(hpack_header_table_field_info_t);
    ch_assert (table->headers_data.head == size * 2);
    ch_assert (table->headers_data.head == table->headers_data.tail - (size * 4));

    /* Clean up */
    hpack_header_field_mrproper (&field);
    for (int i=0; i<6; ++i) {
        hpack_header_field_mrproper (&fields[i]);
    }

    hpack_header_table_free (table);
}
END_TEST


START_TEST (_add_doesnt_fit) {
    ret_t                  ret;
    hpack_header_table_t  *table;
    hpack_set_t            evicted_set;
    hpack_header_field_t   field;

    ret = hpack_header_table_new (&table);
    ch_assert (ret == ret_ok);

    hpack_header_field_init (&field);
    ret = chula_buffer_ensure_size (&field.name, (size_t) (table->max_data/2));
    ch_assert (ret_ok == ret);

    ret = chula_buffer_ensure_size (&field.value, (size_t) (table->max_data/2));
    ch_assert (ret_ok == ret);

    for (unsigned int i=0; i<field.name.len; ++i) {
        ret = chula_buffer_add_char (&field.name, '0' + (i%10));
        ch_assert (ret_ok == ret);
        ret = chula_buffer_add_char (&field.value, '9' - (i%10));
        ch_assert (ret_ok == ret);
    }

    ret = hpack_header_table_add (table, &field, evicted_set);
    ch_assert (ret_ok == ret);

    hpack_set_is_full (evicted_set);

    /* Clean up */
    hpack_header_field_mrproper (&field);
    hpack_header_table_free (table);
}
END_TEST

//
//START_TEST (_add_multi_evac) {
//END_TEST
//

START_TEST (field_get_len) {
    ret_t                ret;
    uint64_t             len;
    hpack_header_field_t field;

    ret = hpack_header_field_init (&field);
    ch_assert (ret == ret_ok);

    len = 1234;
    ret = hpack_header_field_get_size (&field, &len);
    ch_assert (ret == ret_ok);
    ch_assert (len == 0);

    /* (s =  55) custom-key: custom-header */
    chula_buffer_add_str (&field.name,  "custom-key");
    chula_buffer_add_str (&field.value, "custom-header");
    ret = hpack_header_field_get_size (&field, &len);
    ch_assert (ret == ret_ok);
    ch_assert (len == 55);

    /* (s =  42) :method: GET */
    hpack_header_field_clean (&field);
    chula_buffer_add_str (&field.name,  ":method");
    chula_buffer_add_str (&field.value, "GET");
    ret = hpack_header_field_get_size (&field, &len);
    ch_assert (ret == ret_ok);
    ch_assert (len == 42);

    ret = hpack_header_field_mrproper (&field);
    ch_assert (ret == ret_ok);
}
END_TEST


int
blocks (void)
{
    Suite *s1 = suite_create("Header Table Blocks");

    check_add (s1, _init);
    check_add (s1, _clear);
    check_add (s1, _set_max_no_table);
    check_add (s1, _set_max_no_table_evict_all);
    check_add (s1, _set_max_no_table_evict_some);
    check_add (s1, _add_fits);
    check_add (s1, _add_doesnt_fit);
    check_add (s1, _add_some_evacs);
    run_test (s1);
}

int
fields (void)
{
    Suite *s1 = suite_create("Header Fields");
    check_add (s1, field_get_len);
    run_test (s1);
}


int
header_table_tests (void)
{
    int re;

    re  = blocks();
    re += fields();

    return re;
}
