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
#include "libhpack/header.h"

/* Literal Headers
 */

START_TEST (literal_w_index) {
/*
   00                                      | == Literal indexed ==
   0a                                      |   Literal name (len = 10)
   6375 7374 6f6d 2d6b 6579                | custom-key
   0d                                      |   Literal value (len = 13)
   6375 7374 6f6d 2d68 6561 6465 72        | custom-header
*/
    ret_t                ret;
    chula_buffer_t       header;
    hpack_header_table_t table;
    hpack_header_field_t field;
    unsigned int         consumed = 0;

    hpack_header_table_init (&table);
    hpack_header_field_init (&field);
    chula_buffer_fake_str (&header, "\x00\x0a\x63\x75\x73\x74\x6f\x6d\x2d\x6b\x65\x79\x0d\x63\x75\x73\x74\x6f\x6d\x2d\x68\x65\x61\x64\x65\x72");

    ret = hpack_header_field_parse (&header, 0, &table, &field, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == header.len);
    ck_assert_str_eq (field.name.buf, "custom-key");
    ck_assert_str_eq (field.value.buf, "custom-header");

    hpack_header_field_mrproper (&field);
    hpack_header_table_mrproper (&table);
}
END_TEST

START_TEST (literal_wo_index) {
    ret_t                ret;
    chula_buffer_t       header;
    hpack_header_table_t table;
    hpack_header_field_t field;
    unsigned int         consumed = 0;
/*
   44                                      | == Literal not indexed ==
                                           |   Indexed name (idx = 4) :path
   0c                                      | Literal value (len = 12)
   2f73 616d 706c 652f 7061 7468           |   /sample/path
*/

    hpack_header_table_init (&table);
    hpack_header_field_init (&field);
    chula_buffer_fake_str (&header, "\x44\x0c\x2f\x73\x61\x6d\x70\x6c\x65\x2f\x70\x61\x74\x68");

    ret = hpack_header_field_parse (&header, 0, &table, &field, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == header.len);
    ck_assert_str_eq (field.name.buf, ":path");
    ck_assert_str_eq (field.value.buf, "/sample/path");

    hpack_header_field_mrproper (&field);
    hpack_header_table_mrproper (&table);
}
END_TEST

START_TEST (indexed) {
    ret_t                ret;
    chula_buffer_t       header;
    hpack_header_table_t table;
    hpack_header_field_t field;
    unsigned int         consumed = 0;

    hpack_header_table_init (&table);
    hpack_header_field_init (&field);
    chula_buffer_fake_str (&header, "\x82");

    ret = hpack_header_field_parse (&header, 0, &table, &field, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == header.len);
    ck_assert_str_eq (field.name.buf, ":method");
    ck_assert_str_eq (field.value.buf, "GET");

    hpack_header_field_mrproper (&field);
    hpack_header_table_mrproper (&table);
}
END_TEST

START_TEST (request1) {
    ret_t                ret;
    chula_buffer_t       header;
    hpack_header_table_t table;
    hpack_header_field_t field;
    unsigned int         offset   = 0;
    unsigned int         consumed = 0;

/*
   82                                      | == Indexed - Add ==
                                           |   idx = 2
                                           | -> :method: GET
   87                                      | == Indexed - Add ==
                                           |   idx = 7
                                           | -> :scheme: http
   86                                      | == Indexed - Add ==
                                           |   idx = 6
                                           | -> :path: /
   04                                      | == Literal indexed ==
                                           |   Indexed name (idx = 4)
                                           |     :authority
   0f                                      |   Literal value (len = 15)
   7777 772e 6578 616d 706c 652e 636f 6d   | www.example.com
                                           | -> :authority: www.example.com
*/

    chula_buffer_fake_str (&header, "\x82\x87\x86\x04\x0f\x77\x77\x77\x2e\x65\x78\x61\x6d\x70\x6c\x65\x2e\x63\x6f\x6d");
    hpack_header_field_init (&field);
    hpack_header_table_init (&table);

    /* 82 - :method: GET */
    ret = hpack_header_field_parse (&header, offset, &table, &field, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == 1);
//    ck_assert_str_eq (field.name.buf, ":method");
//    ck_assert_str_eq (field.value.buf, "GET");

    offset += consumed;
    ck_assert (offset == 1);
    hpack_header_field_clean (&field);

    /* 87 - :scheme: http */
    ret = hpack_header_field_parse (&header, offset, &table, &field, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == 1);
//    ck_assert_str_eq (field.name.buf, ":scheme");
//    ck_assert_str_eq (field.value.buf, "http");

    offset += consumed;
    ck_assert (offset == 2);
    hpack_header_field_clean (&field);

    /* 86 - :path: / */
    ret = hpack_header_field_parse (&header, offset, &table, &field, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == 1);
//    ck_assert_str_eq (field.name.buf, ":path");
//    ck_assert_str_eq (field.value.buf, "/");

    offset += consumed;
    ck_assert (offset == 3);
    hpack_header_field_clean (&field);

    /* 04 - :authority: www.example.com */
    ret = hpack_header_field_parse (&header, offset, &table, &field, &consumed);
    ck_assert (ret == ret_ok);
//    ck_assert (consumed == 1);


    printf ("consumed %d\n", consumed);
//  ck_assert (consumed_now == 20);

    hpack_header_field_mrproper (&field);
    hpack_header_table_mrproper (&table);
}
END_TEST


int
literal_headers (void)
{
    Suite *s1 = suite_create("Literal Headers");
    check_add (s1, literal_w_index);
    check_add (s1, literal_wo_index);
    check_add (s1, indexed);
    check_add (s1, request1);
    run_test (s1);
}

int
header_tests (void)
{
    int ret;

    ret = literal_headers();

    return ret;
}
