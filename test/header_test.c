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
#include "libhpack/header_parser.h"

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
    ret_t                 ret;
    chula_buffer_t        raw;
    hpack_header_parser_t parser;
    hpack_header_field_t  field;
    unsigned int          consumed = 0;

    hpack_header_parser_init (&parser);
    hpack_header_field_init  (&field);
    chula_buffer_fake_str (&raw, "\x00\x0a\x63\x75\x73\x74\x6f\x6d\x2d\x6b\x65\x79\x0d\x63\x75\x73\x74\x6f\x6d\x2d\x68\x65\x61\x64\x65\x72");

    ret = hpack_header_parser_field (&parser, &raw, 0, &field, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == raw.len);
    ck_assert_str_eq (field.name.buf, "custom-key");
    ck_assert_str_eq (field.value.buf, "custom-header");

    hpack_header_parser_mrproper (&parser);
}
END_TEST

START_TEST (literal_wo_index) {
    ret_t                 ret;
    chula_buffer_t        raw;
    hpack_header_parser_t parser;
    hpack_header_field_t  field;
    unsigned int          consumed = 0;
/*
   44                                      | == Literal not indexed ==
                                           |   Indexed name (idx = 4) :path
   0c                                      | Literal value (len = 12)
   2f73 616d 706c 652f 7061 7468           |   /sample/path
*/

    hpack_header_parser_init (&parser);
    hpack_header_field_init  (&field);
    chula_buffer_fake_str (&raw, "\x44\x0c\x2f\x73\x61\x6d\x70\x6c\x65\x2f\x70\x61\x74\x68");

    ret = hpack_header_parser_field (&parser, &raw, 0, &field, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == raw.len);
    ck_assert_str_eq (field.name.buf, ":path");
    ck_assert_str_eq (field.value.buf, "/sample/path");

    hpack_header_parser_mrproper (&parser);
}
END_TEST

START_TEST (indexed) {
    ret_t                 ret;
    chula_buffer_t        raw;
    hpack_header_parser_t parser;
    hpack_header_field_t  field;
    unsigned int          consumed = 0;

    hpack_header_parser_init (&parser);
    hpack_header_field_init (&field);
    chula_buffer_fake_str (&raw, "\x82");

    ret = hpack_header_parser_field (&parser, &raw, 0, &field, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == raw.len);
    ck_assert_str_eq (field.name.buf, ":method");
    ck_assert_str_eq (field.value.buf, "GET");

    hpack_header_parser_mrproper (&parser);
}
END_TEST

START_TEST (request1) {
    ret_t                 ret;
    chula_buffer_t        raw;
    hpack_header_parser_t parser;
    hpack_header_field_t  field;
    unsigned int          offset   = 0;
    unsigned int          consumed = 0;

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

    chula_buffer_fake_str (&raw, "\x82\x87\x86\x04\x0f\x77\x77\x77\x2e\x65\x78\x61\x6d\x70\x6c\x65\x2e\x63\x6f\x6d");
    hpack_header_field_init (&field);
    hpack_header_parser_init (&parser);

    chula_print_repr (chula, buffer, &raw);

    /* 82 - :method: GET */
    ret = hpack_header_parser_field (&parser, &raw, offset, &field, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == 1);
    ck_assert_str_eq (field.name.buf, ":method");
    ck_assert_str_eq (field.value.buf, "GET");
    chula_print_repr (hpack, header_field, &field);

    offset += consumed;
    ck_assert (offset == 1);
    memset (&field, 0, sizeof(field));

    /* 87 - :scheme: http */
    ret = hpack_header_parser_field (&parser, &raw, offset, &field, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == 1);
    ck_assert_str_eq (field.name.buf, ":scheme");
    ck_assert_str_eq (field.value.buf, "http");
    chula_print_repr (hpack, header_field, &field);

    offset += consumed;
    ck_assert (offset == 2);
    memset (&field, 0, sizeof(field));

    /* 86 - :path: / */
    ret = hpack_header_parser_field (&parser, &raw, offset, &field, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == 1);
    ck_assert_str_eq (field.name.buf, ":path");
    ck_assert_str_eq (field.value.buf, "/");
    chula_print_repr (hpack, header_field, &field);

    offset += consumed;
    ck_assert (offset == 3);
    memset (&field, 0, sizeof(field));

    /* 04 - :authority: www.example.com */
    ret = hpack_header_parser_field (&parser, &raw, offset, &field, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == 17);
    ck_assert_str_eq (field.name.buf, ":authority");
    ck_assert_str_eq (field.value.buf, "www.example.com");
    chula_print_repr (hpack, header_field, &field);

    hpack_header_parser_mrproper (&parser);
}
END_TEST


static void
request1_full_TEST (hpack_header_store_t *store)
{
    ret_t                  ret;
    hpack_header_field_t  *field;

    ret = hpack_header_store_get_n (store, 1, &field);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (field->name.buf, ":method");
    ck_assert_str_eq (field->value.buf, "GET");

    ret = hpack_header_store_get_n (store, 2, &field);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (field->name.buf, ":scheme");
    ck_assert_str_eq (field->value.buf, "http");

    ret = hpack_header_store_get_n (store, 3, &field);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (field->name.buf, ":path");
    ck_assert_str_eq (field->value.buf, "/");

    ret = hpack_header_store_get_n (store, 4, &field);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (field->name.buf, ":authority");
    ck_assert_str_eq (field->value.buf, "www.example.com");
}

START_TEST (request1_full) {
    ret_t                 ret;
    chula_buffer_t        raw;
    hpack_header_store_t  store;
    hpack_header_parser_t parser;
    unsigned int          offset   = 0;
    unsigned int          consumed = 0;

    chula_buffer_fake_str (&raw, "\x82\x87\x86\x04\x0f\x77\x77\x77\x2e\x65\x78\x61\x6d\x70\x6c\x65\x2e\x63\x6f\x6d");

    hpack_header_store_init (&store);
    hpack_header_parser_init (&parser);
    hpack_header_parser_reg_store (&parser, &store);

    /* Full header parse */
    ret = hpack_header_parser_all (&parser, &raw, offset, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == raw.len);

    chula_print_repr (chula, buffer, &raw);
    chula_print_repr (hpack, header_store, &store);
    chula_print_repr (hpack, header_table, &parser.table);

    /* Check headers */
    request1_full_TEST (&store);

    /* Clean up */
    hpack_header_store_mrproper (&store);
    hpack_header_parser_mrproper (&parser);
}
END_TEST

START_TEST (request1_full_huffman) {
    ret_t                 ret;
    chula_buffer_t        raw;
    hpack_header_store_t  store;
    hpack_header_parser_t parser;
    unsigned int          offset   = 0;
    unsigned int          consumed = 0;

    chula_buffer_fake_str (&raw, "\x82\x87\x86\x04\x8b\xdb\x6d\x88\x3e\x68\xd1\xcb\x12\x25\xba\x7f");

    hpack_header_store_init (&store);
    hpack_header_parser_init (&parser);
    hpack_header_parser_reg_store (&parser, &store);

    /* Full header parse */
    ret = hpack_header_parser_all (&parser, &raw, offset, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == raw.len);

    chula_print_repr (chula, buffer, &raw);
    chula_print_repr (hpack, header_store, &store);
    chula_print_repr (hpack, header_table, &parser.table);

    /* Check headers */
    request1_full_TEST (&store);

    /* Clean up */
    hpack_header_store_mrproper (&store);
    hpack_header_parser_mrproper (&parser);
}
END_TEST


int
header_fields (void)
{
    Suite *s1 = suite_create("Header fields parsing");
    check_add (s1, literal_w_index);
    check_add (s1, literal_wo_index);
    check_add (s1, indexed);
    check_add (s1, request1);
    run_test (s1);
}

int
header_full (void)
{
    Suite *s1 = suite_create("Header fields parsing");
    check_add (s1, request1_full);
    check_add (s1, request1_full_huffman);
    run_test (s1);
}


int
header_tests (void)
{
    int re;

    re  = header_fields();
    re += header_full();

    return re;
}
