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
#include "libhpack/header_parser.h"

#define assert_store_n_eq(parser,n,_name,_value)  do {                  \
        ret_t                 ret;                                      \
        hpack_header_field_t *field;                                    \
                                                                        \
        ret = hpack_header_store_get_n ((parser)->store, n, &field);    \
        ck_assert (ret == ret_ok);                                      \
        ck_assert_str_eq (field->name.buf, _name);                      \
        ck_assert_str_eq (field->value.buf, _value);                    \
    } while(false)

#define assert_dyn_table_n_eq(parser,n,_name,_value) do {               \
        ret_t                 ret;                                      \
        hpack_header_field_t *field;                                    \
                                                                        \
        ret = hpack_header_block_get (&(parser)->table.dynamic, n-1, &field); \
        ck_assert (ret == ret_ok);                                      \
        ck_assert_str_eq (field->name.buf, _name);                      \
        ck_assert_str_eq (field->value.buf, _value);                    \
    } while(false)


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

START_TEST (literal_w_index_false_len) {
/*
    This test tries to break the header field parsing by sending string
    literals with a string length field too big to be hold by a signed integer
    (works with 32bit and 64bit machines). Since it's too big, and given the
    algorith of the VLQ it gets transformed into a negative number if this has
    not been taken into account.

    As a negative number it would pass many of the test conditions for sizes.

    For a 32 bits machine:
   00                                      | == Literal indexed ==
   ffff ffff ff0a                          |   Literal name (len = 2952790270). Since it's bigger than INT_MAX (2147483647) it results in a negative number.
   6375 7374 6f6d 2d6b 6579                | custom-key
   0d                                      |   Literal value (len = 13)
   6375 7374 6f6d 2d68 6561 6465 72        | custom-header


    For a 64 bits machine:
   00                                      | == Literal indexed ==
   ffff ffff ffff ffff ffff                |   Literal name (len = 9223372036854776062). Since it's bigger than INT_MAX (9223372036854775807) it results in a negative number
   6375 7374 6f6d 2d6b 6579                | custom-key
   0d                                      |   Literal value (len = 13)
   6375 7374 6f6d 2d68 6561 6465 72        | custom-header

*/
    ret_t                 ret;
    chula_buffer_t        raw;
    hpack_header_parser_t parser;
    hpack_header_field_t  field;
    unsigned int          consumed = 0;

    const char *data64 = "\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x63\x75\x73\x74\x6f\x6d\x2d\x6b\x65\x79\x0d\x63\x75\x73\x74\x6f\x6d\x2d\x68\x65\x61\x64\x65\x72";
    const char *data32 = "\x00\xFF\xFF\xFF\xFF\xFF\x0A\x63\x75\x73\x74\x6f\x6d\x2d\x6b\x65\x79\0d\x63\x75\x73\x74\x6f\x6d\x2d\x68\x65\x61\x64\x65\x72";

    hpack_header_parser_init (&parser);
    hpack_header_field_init  (&field);
    chula_buffer_fake_str (&raw, sizeof(int)>32?data64:data32);

    ret = hpack_header_parser_field (&parser, &raw, 0, &field, &consumed);
    ck_assert (ret != ret_ok);
    ck_assert (consumed == 0);

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

START_TEST (indexed_big_value) {
/*
    This test tries to break the header field parsing by sending an index header
    field too big to be hold by a signed integer (works with 32bit and 64bit
    machines). Since it's too big, and given the algorith of the VLQ it gets
    transformed into a negative number if this has not been taken into account.

    As a negative number it would pass many of the test conditions for sizes.
*/
    ret_t                 ret;
    chula_buffer_t        raw;
    hpack_header_parser_t parser;
    hpack_header_field_t  field;
    unsigned int          consumed = 0;

    const char *data64 = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
    const char *data32 = "\xFF\xFF\xFF\xFF\xFF\x0A";

    hpack_header_parser_init (&parser);
    hpack_header_field_init (&field);
    chula_buffer_fake_str (&raw, sizeof(int)>32?data64:data32);

    ret = hpack_header_parser_field (&parser, &raw, 0, &field, &consumed);
    ck_assert (ret != ret_ok);
    ck_assert (consumed == 0);

    hpack_header_parser_mrproper (&parser);
}
END_TEST

START_TEST (indexed_many_zeroes) {
/*
    According to the draft a large number of zero values MUST be treated as a
    decoding error.

    This test sends a considerable number of zeroes as an index header field.
*/
    ret_t                 ret;
    chula_buffer_t        raw;
    hpack_header_parser_t parser;
    hpack_header_field_t  field;
    unsigned int          consumed = 0;

    char data[256];

    data[0] = 0xFF;
    memset(data+1, 0x80, sizeof(data)-1);
    data[sizeof(data)-1] |= 1;

    hpack_header_parser_init (&parser);
    hpack_header_field_init (&field);
    chula_buffer_fake_str (&raw, data);

    ret = hpack_header_parser_field (&parser, &raw, 0, &field, &consumed);
    ck_assert (ret != ret_ok);
    ck_assert (consumed == 0);

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
request1_full_TEST (hpack_header_parser_t *parser)
{
    assert_store_n_eq (parser, 1, ":method",    "GET");
    assert_store_n_eq (parser, 2, ":scheme",    "http");
    assert_store_n_eq (parser, 3, ":path",      "/");
    assert_store_n_eq (parser, 4, ":authority", "www.example.com");

    assert_dyn_table_n_eq (parser, 1, ":authority", "www.example.com");
    assert_dyn_table_n_eq (parser, 2, ":path",      "/");
    assert_dyn_table_n_eq (parser, 3, ":scheme",    "http");
    assert_dyn_table_n_eq (parser, 4, ":method",    "GET");
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
    request1_full_TEST (&parser);

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

    /* First Request
     */

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
    request1_full_TEST (&parser);


    /* Second Request
     */

    offset   = 0;
    consumed = 0;

    chula_buffer_fake_str (&raw, "\x1b\x86\x63\x65\x4a\x13\x98\xff");
    hpack_header_store_init (&store);

    /* Full header parse */
    ret = hpack_header_parser_all (&parser, &raw, offset, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == raw.len);

    chula_print_repr (chula, buffer, &raw);
    chula_print_repr (hpack, header_store, &store);
    chula_print_repr (hpack, header_table, &parser.table);

    assert_store_n_eq (&parser, 1, "cache-control", "no-cache");
    assert_store_n_eq (&parser, 2, ":authority", "www.example.com");
    assert_store_n_eq (&parser, 3, ":path", "/");
    assert_store_n_eq (&parser, 4, ":scheme", "http");
    assert_store_n_eq (&parser, 5, ":method", "GET");


    /* Third Request
     */

    offset   = 0;
    consumed = 0;

    chula_buffer_fake_str (&raw, "\x80\x80\x85\x8c\x8b\x84\x00\x88\x4e\xb0\x8b\x74\x97\x90\xfa\x7f\x89\x4e\xb0\x8b\x74\x97\x9a\x17\xa8\xff");
    hpack_header_store_init (&store);

    /* Full header parse */
    ret = hpack_header_parser_all (&parser, &raw, offset, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == raw.len);

    chula_print_repr (chula, buffer, &raw);
    chula_print_repr (hpack, header_store, &store);
    chula_print_repr (hpack, header_table, &parser.table);

    assert_store_n_eq (&parser, 1, ":method", "GET");
    assert_store_n_eq (&parser, 2, ":scheme", "https");
    assert_store_n_eq (&parser, 3, ":path", "/index.html");
    assert_store_n_eq (&parser, 4, ":authority", "www.example.com");
    assert_store_n_eq (&parser, 5, "custom-key", "custom-value");

    /* Clean up */
    hpack_header_store_mrproper (&store);
    hpack_header_parser_mrproper (&parser);
}
END_TEST


START_TEST (request2_full_huffman) {
    ret_t                 ret;
    chula_buffer_t        raw;
    hpack_header_store_t  store;
    hpack_header_parser_t parser;
    unsigned int          offset   = 0;
    unsigned int          consumed = 0;

/*
   08                                      | == Literal indexed ==
                                           |   Indexed name (idx = 8)
                                           |     :status
   82                                      |   Literal value (len = 3)
                                           |     Huffman encoded:
   98a7                                    | ..
                                           |     Decoded:
                                           | 302
                                           | -> :status: 302
   18                                      | == Literal indexed ==
                                           |   Indexed name (idx = 24)
                                           |     cache-control
   85                                      |   Literal value (len = 7)
                                           |     Huffman encoded:
   73d5 cd11 1f                            | s....
                                           |     Decoded:
                                           | private
                                           | -> cache-control: private
   22                                      | == Literal indexed ==
                                           |   Indexed name (idx = 34)
                                           |     date
   98                                      |   Literal value (len = 29)
                                           |     Huffman encoded:
   ef6b 3a7a 0e6e 8fa2 63d0 729a 6e83 97d8 | .k:z.n..c.r.n...
   69bd 8737 47bb bfc7                     | i..7G...
                                           |     Decoded:
                                           | Mon, 21 Oct 2013 20:13:21 GMT
                                           | -> date: Mon, 21 Oct 2013 20:13:21 GMT
   30                                      | == Literal indexed ==
                                           |   Indexed name (idx = 48)
                                           |     location
   90                                      |   Literal value (len = 23)
                                           |     Huffman encoded:
   ce31 743d 801b 6db1 07cd 1a39 6244 b74f | .1t=..m....9bD.O
                                           |     Decoded:
                                           | https://www.example.com
                                           | -> location: https://www.example.com
*/

    chula_buffer_fake_str (&raw, "\x08\x82\x98\xa7\x18\x85\x73\xd5\xcd\x11\x1f\x22\x98\xef\x6b\x3a\x7a\x0e\x6e\x8f\xa2\x63\xd0\x72\x9a\x6e\x83\x97\xd8\x69\xbd\x87\x37\x47\xbb\xbf\xc7\x30\x90\xce\x31\x74\x3d\x80\x1b\x6d\xb1\x07\xcd\x1a\x39\x62\x44\xb7\x4f");

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

    /* Checks */
    assert_store_n_eq (&parser, 1, ":status",       "302");
    assert_store_n_eq (&parser, 2, "cache-control", "private");
    assert_store_n_eq (&parser, 3, "date",          "Mon, 21 Oct 2013 20:13:21 GMT");
    assert_store_n_eq (&parser, 4, "location",      "https://www.example.com");

    assert_dyn_table_n_eq (&parser, 1, "location",      "https://www.example.com");
    assert_dyn_table_n_eq (&parser, 2, "date",          "Mon, 21 Oct 2013 20:13:21 GMT");
    assert_dyn_table_n_eq (&parser, 3, "cache-control", "private");
    assert_dyn_table_n_eq (&parser, 4, ":status",       "302");

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
    check_add (s1, literal_w_index_false_len);
    check_add (s1, literal_wo_index);
    check_add (s1, indexed);
    check_add (s1, indexed_big_value);
    check_add (s1, indexed_many_zeroes);
    check_add (s1, request1);
    run_test (s1);
}

int
header_full (void)
{
    Suite *s1 = suite_create("Full header parsing");
    check_add (s1, request1_full);
    check_add (s1, request1_full_huffman);
    check_add (s1, request2_full_huffman);
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
