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

#include "libchula/testing_macros.h"
#include "libhpack/header_table.h"


START_TEST (_init) {
    ret_t                ret;
    hpack_header_block_t block;

    ret = hpack_header_block_init (&block);
    ch_assert (ret == ret_ok);
    ch_assert (block.len == 0);
    ch_assert (block.headers == NULL);

    ret = hpack_header_block_mrproper (&block);
    ch_assert (ret == ret_ok);
}
END_TEST

START_TEST (_set_max) {
    ret_t                ret;
    hpack_header_block_t block;

    hpack_header_block_init (&block);
    ret = hpack_header_block_set_max (&block, 5);
    ch_assert (ret == ret_ok);
    ch_assert (block.headers != NULL);

    ret = hpack_header_block_mrproper (&block);
    ch_assert (ret == ret_ok);
}
END_TEST

START_TEST (_add1) {
    ret_t                 ret;
    hpack_header_block_t  block;
    hpack_header_field_t  field[6];
    hpack_header_field_t *fi;

    hpack_header_block_init (&block);
    hpack_header_block_set_max (&block, 5);

    for (int i=0; i<6; i++) {
        hpack_header_field_init (&field[i]);
        chula_buffer_add_va  (&field[i].name, "%c", 'a'+i);
        chula_buffer_add_str (&field[i].value, "foobar");
    }

    /* Add field #2-5 */
    for (int i=0; i<6; i++) {
        ret = hpack_header_block_add (&block, &field[i]);
        ch_assert (ret == ret_ok);
    }
    ch_assert (block.len == 5);

    /*    |4  3  2  1  0| */
    /*    |             | */
    /* [a][b][c][d][e][f] */

    /* Get pos=0 */
    fi  = NULL;
    ret = hpack_header_block_get (&block, 0, &fi);
    ch_assert (ret == ret_ok);
    ch_assert (fi != NULL);
    ch_assert_str_eq (fi->name.buf, "f");

    /* Get pos=1 */
    fi = NULL;
    ret = hpack_header_block_get (&block, 1, &fi);
    ch_assert_str_eq (fi->name.buf, "e");

    /* Get pos=5 */
    ret = hpack_header_block_get (&block, 4, &fi);
    ch_assert_str_eq (fi->name.buf, "b");

    /* Clean up */
    ret = hpack_header_block_mrproper (&block);
    ch_assert (ret == ret_ok);
}
END_TEST

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
    chula_buffer_clean (&field.name);
    chula_buffer_clean (&field.value);
    chula_buffer_add_str (&field.name,  ":method");
    chula_buffer_add_str (&field.value, "GET");
    ret = hpack_header_field_get_size (&field, &len);
    ch_assert (ret == ret_ok);
    ch_assert (len == 42);
}
END_TEST


int
blocks (void)
{
    Suite *s1 = suite_create("Header Table Blocks");
    check_add (s1, _init);
    check_add (s1, _set_max);
    check_add (s1, _add1);
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
