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

#include <libhpack/libhpack.h>
#include <libchula-qa/libchula-qa.h>
#include <libchula-qa/testing_macros-internal.h>


#define assert_store_n_eq(parser,n,_name,_value)  do {                                         \
        ret_t                 ret;                                                             \
        hpack_header_field_t *field;                                                           \
                                                                                               \
        ret = hpack_header_store_get_n ((parser)->store, n, &field);                           \
        ch_assert (ret == ret_ok);                                                             \
        ch_assert_str_eq (field->name.buf, _name);                                             \
        ch_assert_str_eq (field->value.buf, _value);                                           \
    } while(false)

#define assert_header_table_n_eq(parser,n,_name,_value) do {                                   \
        ret_t                ret;                                                              \
        hpack_header_field_t field;                                                            \
        bool                 is_static;                                                        \
                                                                                               \
        hpack_header_field_init (&field);                                                      \
        ret = hpack_header_table_get (&(parser)->context.table, n, false, &field, &is_static); \
        ch_assert (ret == ret_ok);                                                             \
        ch_assert_str_eq (field.name.buf, _name);                                              \
        ch_assert_str_eq (field.value.buf, _value);                                            \
        hpack_header_field_mrproper (&field);                                                  \
    } while(false)


/* Literal Headers
 */

START_TEST (literal_w_index) {
/*
   http://http2.github.io/http2-spec/compression.html#rfc.section.D.2.1

   40                                      | == Literal indexed ==
   0a                                      |   Literal name (len = 10)
   6375 7374 6f6d 2d6b 6579                | custom-key
   0d                                      |   Literal value (len = 13)
   6375 7374 6f6d 2d68 6561 6465 72        | custom-header
                                           | -> custom-key: custom-header

   Header Table (after decoding):
     [  1] (s =  55) custom-key: custom-header
           Table size:  55

   Decoded header set:
     custom-key: custom-header
*/
    ret_t                  ret;
    chula_buffer_t         raw;
    hpack_header_parser_t *parser;
    hpack_header_field_t   field;
    unsigned int           consumed = 0;

    hpack_header_parser_new (&parser);
    hpack_header_field_init  (&field);
    chula_buffer_fake_str (&raw, "\x40\x0a\x63\x75\x73\x74\x6f\x6d\x2d\x6b\x65\x79\x0d\x63\x75\x73\x74\x6f\x6d\x2d\x68\x65\x61\x64\x65\x72");

    ret = hpack_header_parser_field (parser, &raw, 0, &field, &consumed);
    ch_assert (ret == ret_ok);
    ch_assert (consumed == raw.len);
    ch_assert_str_eq (field.name.buf, "custom-key");
    ch_assert_str_eq (field.value.buf, "custom-header");

    ret = hpack_header_parser_field (parser, &raw, consumed, &field, &consumed);
    ch_assert (ret_eof == ret);
    ch_assert (0 == consumed);
    ch_assert (hpack_header_field_is_empty (&field));

    hpack_header_field_mrproper (&field);
    hpack_header_parser_mrproper (&parser);
}
END_TEST


START_TEST (literal_w_index_false_len) {
/*
   This test tries to break the header field parsing by sending string
   literals with a string length field too big to be hold by a signed integer
   (works with 32bit and 64bit machines). Since it's too big, and given the
   algorithm of the VLQ it gets transformed into a negative number if this has
   not been taken into account.

   As a negative number it would pass many of the test conditions for sizes.

   For a 32 bits machine:
   40                                      | == Literal indexed ==
   ffff ffff ff0a                          |   Literal name (len = 2952790270). Since it's bigger than INT_MAX (2147483647) it results in a negative number.
   6375 7374 6f6d 2d6b 6579                | custom-key
   0d                                      |   Literal value (len = 13)
   6375 7374 6f6d 2d68 6561 6465 72        | custom-header


   For a 64 bits machine:
   40                                      | == Literal indexed ==
   ffff ffff ffff ffff ffff                |   Literal name (len = 9223372036854776062). Since it's bigger than INT_MAX (9223372036854775807) it results in a negative number
   6375 7374 6f6d 2d6b 6579                | custom-key
   0d                                      |   Literal value (len = 13)
   6375 7374 6f6d 2d68 6561 6465 72        | custom-header
*/
    ret_t                  ret;
    chula_buffer_t         raw;
    hpack_header_parser_t *parser;
    hpack_header_field_t   field;
    unsigned int           consumed = 0;

    const char *data64 = "\x40\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x63\x75\x73\x74\x6f\x6d\x2d\x6b\x65\x79\x0d\x63\x75\x73\x74\x6f\x6d\x2d\x68\x65\x61\x64\x65\x72";
    const char *data32 = "\x40\xFF\xFF\xFF\xFF\xFF\x0A\x63\x75\x73\x74\x6f\x6d\x2d\x6b\x65\x79\0d\x63\x75\x73\x74\x6f\x6d\x2d\x68\x65\x61\x64\x65\x72";

    hpack_header_parser_new (&parser);
    hpack_header_field_init (&field);

    chula_buffer_fake_str (&raw, sizeof(int)>32?data64:data32);

    ret = hpack_header_parser_field (parser, &raw, 0, &field, &consumed);
    ch_assert (ret != ret_ok);
    ch_assert (consumed == 0);
    ch_assert (hpack_header_field_is_empty (&field));

    hpack_header_field_mrproper (&field);
    hpack_header_parser_mrproper (&parser);
    hpack_header_field_mrproper (&field);
}
END_TEST

START_TEST (literal_wo_index) {
/*
   http://http2.github.io/http2-spec/compression.html#rfc.section.D.2.2

   04                                      | == Literal not indexed ==
                                           |   Indexed name (idx = 4) :path
   0c                                      | Literal value (len = 12)
   2f73 616d 706c 652f 7061 7468           |   /sample/path

   Header table (after decoding): empty.

   Decoded header set:
     :path: /sample/path
*/
    ret_t                  ret;
    chula_buffer_t         raw;
    hpack_header_parser_t *parser;
    hpack_header_field_t   field;
    unsigned int           consumed = 0;

    hpack_header_parser_new (&parser);
    hpack_header_field_init (&field);
    chula_buffer_fake_str (&raw, "\x04\x0c\x2f\x73\x61\x6d\x70\x6c\x65\x2f\x70\x61\x74\x68");

    ret = hpack_header_parser_field (parser, &raw, 0, &field, &consumed);
    ch_assert (ret == ret_ok);
    ch_assert (consumed == raw.len);
    ch_assert_str_eq (field.name.buf, ":path");
    ch_assert_str_eq (field.value.buf, "/sample/path");

    ret = hpack_header_parser_field (parser, &raw, consumed, &field, &consumed);
    ch_assert (ret_eof == ret);
    ch_assert (0 == consumed);
    ch_assert (hpack_header_field_is_empty (&field));

    hpack_header_field_mrproper (&field);
    hpack_header_parser_mrproper (&parser);
    hpack_header_field_mrproper (&field);
}
END_TEST

START_TEST (indexed) {
/*
   http://http2.github.io/http2-spec/compression.html#rfc.section.D.2.3

   82                                      | == Indexed - Add ==
                                           |   idx = 2
                                           | -> :method: GET

   Header Table (after decoding):
     [  1] (s =  42) :method: GET
           Table size:  42

   Decoded header set:
     :method: GET
 */
    ret_t                  ret;
    chula_buffer_t         raw;
    hpack_header_parser_t *parser;
    hpack_header_field_t   field;
    unsigned int           consumed = 0;

    hpack_header_parser_new (&parser);
    hpack_header_field_init (&field);
    chula_buffer_fake_str (&raw, "\x82");

    ret = hpack_header_parser_field (parser, &raw, 0, &field, &consumed);
    ch_assert (ret == ret_ok);
    ch_assert (consumed == raw.len);
    ch_assert_str_eq (field.name.buf, ":method");
    ch_assert_str_eq (field.value.buf, "GET");

    ret = hpack_header_parser_field (parser, &raw, consumed, &field, &consumed);
    ch_assert (ret_eof == ret);
    ch_assert (0 == consumed);
    ch_assert (hpack_header_field_is_empty (&field));

    hpack_header_field_mrproper (&field);
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
    ret_t                  ret;
    chula_buffer_t         raw;
    hpack_header_parser_t *parser;
    hpack_header_field_t   field;
    unsigned int           consumed = 0;

    const char *data64 = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
    const char *data32 = "\xFF\xFF\xFF\xFF\xFF\x0A";

    hpack_header_parser_new (&parser);
    hpack_header_field_init (&field);
    chula_buffer_fake_str (&raw, sizeof(int)>32?data64:data32);

    ret = hpack_header_parser_field (parser, &raw, 0, &field, &consumed);
    ch_assert (ret != ret_ok);
    ch_assert (consumed == 0);
    ch_assert (hpack_header_field_is_empty (&field));

    hpack_header_field_mrproper (&field);
    hpack_header_parser_mrproper (&parser);
    hpack_header_field_mrproper (&field);
}
END_TEST

START_TEST (indexed_many_zeroes) {
/*
    According to the draft a large number of zero values MUST be treated as a
    decoding error.

    This test sends a considerable number of zeroes as an index header field.
*/
    ret_t                  ret;
    chula_buffer_t         raw;
    hpack_header_parser_t *parser;
    hpack_header_field_t   field;
    unsigned int           consumed = 0;

    char data[256];

    data[0] = 0xFF;
    memset(data+1, 0x80, sizeof(data)-1);
    data[sizeof(data)-1] |= 1;

    hpack_header_parser_new (&parser);
    hpack_header_field_init (&field);
    chula_buffer_fake_str (&raw, data);

    ret = hpack_header_parser_field (parser, &raw, 0, &field, &consumed);
    ch_assert (ret != ret_ok);
    ch_assert (consumed == 0);
    ch_assert (hpack_header_field_is_empty (&field));

    hpack_header_field_mrproper (&field);
    hpack_header_parser_mrproper (&parser);
    hpack_header_field_mrproper (&field);
}
END_TEST

START_TEST (request1) {
/*
   http://http2.github.io/http2-spec/compression.html#rfc.section.D.3.1

   82                                      | == Indexed - Add ==
                                           |   idx = 2
                                           | -> :method: GET
   87                                      | == Indexed - Add ==
                                           |   idx = 7
                                           | -> :scheme: http
   86                                      | == Indexed - Add ==
                                           |   idx = 6
                                           | -> :path: /
   44                                      | == Literal indexed ==
                                           |   Indexed name (idx = 4)
                                           |     :authority
   0f                                      |   Literal value (len = 15)
   7777 772e 6578 616d 706c 652e 636f 6d   | www.example.com
                                           | -> :authority: www.example.com
   Header Table (after decoding):
     [  1] (s =  57) :authority: www.example.com
     [  2] (s =  38) :path: /
     [  3] (s =  43) :scheme: http
     [  4] (s =  42) :method: GET
           Table size: 180

   Decoded header set:
     :method: GET
     :scheme: http
     :path: /
     :authority: www.example.com
*/
    ret_t                  ret;
    chula_buffer_t         raw;
    hpack_header_parser_t *parser;
    hpack_header_field_t   field;
    unsigned int           offset   = 0;
    unsigned int           consumed = 0;

    chula_buffer_fake_str (&raw, "\x82\x87\x86\x44\x0f\x77\x77\x77\x2e\x65\x78\x61\x6d\x70\x6c\x65\x2e\x63\x6f\x6d");
    hpack_header_field_init (&field);
    hpack_header_parser_new (&parser);


    chula_print_repr (chula, buffer, &raw);

    /* 82 - :method: GET */
    ret = hpack_header_parser_field (parser, &raw, offset, &field, &consumed);
    ch_assert (ret == ret_ok);
    ch_assert (consumed == 1);
    ch_assert_str_eq (field.name.buf, ":method");
    ch_assert_str_eq (field.value.buf, "GET");
    chula_print_repr (hpack, header_field, &field);

    offset += consumed;
    ch_assert (offset == 1);

    /* 87 - :scheme: http */
    ret = hpack_header_parser_field (parser, &raw, offset, &field, &consumed);
    ch_assert (ret == ret_ok);
    ch_assert (consumed == 1);
    ch_assert_str_eq (field.name.buf, ":scheme");
    ch_assert_str_eq (field.value.buf, "http");
    chula_print_repr (hpack, header_field, &field);

    offset += consumed;
    ch_assert (offset == 2);

    /* 86 - :path: / */
    ret = hpack_header_parser_field (parser, &raw, offset, &field, &consumed);
    ch_assert (ret == ret_ok);
    ch_assert (consumed == 1);
    ch_assert_str_eq (field.name.buf, ":path");
    ch_assert_str_eq (field.value.buf, "/");
    chula_print_repr (hpack, header_field, &field);

    offset += consumed;
    ch_assert (offset == 3);

    /* 04 - :authority: www.example.com */
    ret = hpack_header_parser_field (parser, &raw, offset, &field, &consumed);
    ch_assert (ret == ret_ok);
    ch_assert (consumed == 17);
    ch_assert_str_eq (field.name.buf, ":authority");
    ch_assert_str_eq (field.value.buf, "www.example.com");
    chula_print_repr (hpack, header_field, &field);

    offset += consumed;
    ret = hpack_header_parser_field (parser, &raw, offset, &field, &consumed);
    ch_assert (ret_eof == ret);
    ch_assert (consumed == 0);
    ch_assert (hpack_header_field_is_empty (&field));

    ch_assert (180 == hpack_header_table_get_size (&parser->context.table));

    hpack_header_field_mrproper (&field);
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

    assert_header_table_n_eq (parser, 1, ":authority", "www.example.com");
    assert_header_table_n_eq (parser, 2, ":path",      "/");
    assert_header_table_n_eq (parser, 3, ":scheme",    "http");
    assert_header_table_n_eq (parser, 4, ":method",    "GET");

    /* Check size */
    ch_assert (180 == hpack_header_table_get_size (&parser->context.table));
}

START_TEST (request1_full) {
/*
   http://http2.github.io/http2-spec/compression.html#rfc.section.D.3.1

   82                                      | == Indexed - Add ==
                                           |   idx = 2
                                           | -> :method: GET
   87                                      | == Indexed - Add ==
                                           |   idx = 7
                                           | -> :scheme: http
   86                                      | == Indexed - Add ==
                                           |   idx = 6
                                           | -> :path: /
   44                                      | == Literal indexed ==
                                           |   Indexed name (idx = 4)
                                           |     :authority
   0f                                      |   Literal value (len = 15)
   7777 772e 6578 616d 706c 652e 636f 6d   | www.example.com
                                           | -> :authority: www.example.com
   Header Table (after decoding):
     [  1] (s =  57) :authority: www.example.com
     [  2] (s =  38) :path: /
     [  3] (s =  43) :scheme: http
     [  4] (s =  42) :method: GET
           Table size: 180

   Decoded header set:
     :method: GET
     :scheme: http
     :path: /
     :authority: www.example.com
*/
    ret_t                  ret;
    chula_buffer_t         raw;
    hpack_header_store_t   store;
    hpack_header_parser_t *parser;
    unsigned int           offset   = 0;
    unsigned int           consumed = 0;

    chula_buffer_fake_str (&raw, "\x82\x87\x86\x44\x0f\x77\x77\x77\x2e\x65\x78\x61\x6d\x70\x6c\x65\x2e\x63\x6f\x6d");

    hpack_header_store_init (&store);
    hpack_header_parser_new (&parser);
    hpack_header_parser_reg_store (parser, &store);

    /* Full header parse */
    ret = hpack_header_parser_all (parser, &raw, offset, &consumed);
    ch_assert (ret == ret_ok);
    printf ("Consumed:%d == Len=%d\n", consumed, raw.len);
    ch_assert (consumed == raw.len);

    chula_print_repr (chula, buffer, &raw);
    chula_print_repr (hpack, header_store, &store);
    chula_print_repr (hpack, header_table, &parser->context.table);

    /* Check headers */
    request1_full_TEST (parser);

    /* Clean up */
    hpack_header_store_mrproper (&store);
    hpack_header_parser_mrproper (&parser);
}
END_TEST

START_TEST (request1_full_huffman) {
    ret_t                  ret;
    chula_buffer_t         raw;
    hpack_header_store_t   store;
    hpack_header_parser_t *parser;
    unsigned int           offset   = 0;
    unsigned int           consumed = 0;

    /* First Request
     */
    chula_buffer_fake_str (&raw, "\x82\x87\x86\x44\x8c\xf1\xe3\xc2\xe5\xf2\x3a\x6b\xa0\xab\x90\xf4\xff");

    hpack_header_store_init (&store);
    hpack_header_parser_new (&parser);
    hpack_header_parser_reg_store (parser, &store);

    /* Full header parse */
    ret = hpack_header_parser_all (parser, &raw, offset, &consumed);
    ch_assert (ret == ret_ok);
    ch_assert (consumed == raw.len);

    chula_print_repr (chula, buffer, &raw);
    chula_print_repr (hpack, header_store, &store);
    chula_print_repr (hpack, header_table, &parser->context.table);

    /* Check headers */
    request1_full_TEST (parser);

    hpack_header_store_mrproper (&store);

    /* Second Request
     */

    offset   = 0;
    consumed = 0;

    chula_buffer_fake_str (&raw, "\x5c\x86\xa8\xeb\x10\x64\x9c\xbf");

    hpack_header_store_init (&store);

    /* Full header parse */
    ret = hpack_header_parser_all (parser, &raw, offset, &consumed);
    ch_assert (ret == ret_ok);
    ch_assert (consumed == raw.len);

    chula_print_repr (chula, buffer, &raw);
    chula_print_repr (hpack, header_store, &store);
    chula_print_repr (hpack, header_table, &parser->context.table);

    assert_store_n_eq (parser, 1, "cache-control", "no-cache");
    assert_store_n_eq (parser, 2, ":method", "GET");
    assert_store_n_eq (parser, 3, ":scheme", "http");
    assert_store_n_eq (parser, 4, ":path", "/");
    assert_store_n_eq (parser, 5, ":authority", "www.example.com");

    assert_header_table_n_eq (parser, 1, "cache-control", "no-cache");
    assert_header_table_n_eq (parser, 2, ":authority", "www.example.com");
    assert_header_table_n_eq (parser, 3, ":path", "/");
    assert_header_table_n_eq (parser, 4, ":scheme", "http");
    assert_header_table_n_eq (parser, 5, ":method", "GET");

    ch_assert (233 == hpack_header_table_get_size (&parser->context.table));

    hpack_header_store_mrproper (&store);

    /* Third Request
     */

    offset   = 0;
    consumed = 0;

    chula_buffer_fake_str (&raw, "\x30\x85\x8c\x8b\x84\x40\x88\x25\xa8\x49\xe9\x5b\xa9\x7d\x7f\x89\x25\xa8\x49\xe9\x5b\xb8\xe8\xb4\xbf");

    hpack_header_store_init (&store);

    /* Full header parse */
    ret = hpack_header_parser_all (parser, &raw, offset, &consumed);
    ch_assert (ret == ret_ok);
    ch_assert (consumed == raw.len);

    chula_print_repr (chula, buffer, &raw);
    chula_print_repr (hpack, header_store, &store);
    chula_print_repr (hpack, header_table, &parser->context.table);

    assert_store_n_eq (parser, 1, ":method", "GET");
    assert_store_n_eq (parser, 2, ":scheme", "https");
    assert_store_n_eq (parser, 3, ":path", "/index.html");
    assert_store_n_eq (parser, 4, ":authority", "www.example.com");
    assert_store_n_eq (parser, 5, "custom-key", "custom-value");

    assert_header_table_n_eq (parser, 1, "custom-key", "custom-value");
    assert_header_table_n_eq (parser, 2, ":path", "/index.html");
    assert_header_table_n_eq (parser, 3, ":scheme", "https");
    assert_header_table_n_eq (parser, 4, "cache-control", "no-cache");
    assert_header_table_n_eq (parser, 5, ":authority", "www.example.com");
    assert_header_table_n_eq (parser, 6, ":path", "/");
    assert_header_table_n_eq (parser, 7, ":scheme", "http");
    assert_header_table_n_eq (parser, 8, ":method", "GET");

    ch_assert (379 == hpack_header_table_get_size (&parser->context.table));

    /* Clean up */
    hpack_header_store_mrproper (&store);
    hpack_header_parser_mrproper (&parser);
}
END_TEST


START_TEST (request2_full_huffman) {
/*
   http://http2.github.io/http2-spec/compression.html#rfc.section.D.6.1

   Decoding process:

   48                                      | == Literal indexed ==
                                           |   Indexed name (idx = 8)
                                           |     :status
   82                                      |   Literal value (len = 3)
                                           |     Huffman encoded:
   4017                                    | @.
                                           |     Decoded:
                                           | 302
                                           | -> :status: 302
   59                                      | == Literal indexed ==
                                           |   Indexed name (idx = 25)
                                           |     cache-control
   85                                      |   Literal value (len = 7)
                                           |     Huffman encoded:
   bf06 724b 97                            | ..rK.
                                           |     Decoded:
                                           | private
                                           | -> cache-control: private
   63                                      | == Literal indexed ==
                                           |   Indexed name (idx = 35)
                                           |     date
   93                                      |   Literal value (len = 29)
                                           |     Huffman encoded:
   d6db b298 84de 2a71 8805 0620 9851 3109 | ......*q... .Q1.
   b56b a3                                 | .k.
                                           |     Decoded:
                                           | Mon, 21 Oct 2013 20:13:21 \
                                           | GMT
                                           | -> date: Mon, 21 Oct 2013 \
                                           |   20:13:21 GMT
   71                                      | == Literal indexed ==
                                           |   Indexed name (idx = 49)
                                           |     location
   91                                      |   Literal value (len = 23)
                                           |     Huffman encoded:
   adce bf19 8e7e 7cf9 bebe 89b6 fb16 fa9b | ......|.........
   6f                                      | o
                                           |     Decoded:
                                           | https://www.example.com
                                           | -> location: https://www.e\
                                           |   xample.com

   Header Table (after decoding):
     [  1] (s =  63) location: https://www.example.com
     [  2] (s =  65) date: Mon, 21 Oct 2013 20:13:21 GMT
     [  3] (s =  52) cache-control: private
     [  4] (s =  42) :status: 302
           Table size: 222

   Decoded header set:
     :status: 302
     cache-control: private
     date: Mon, 21 Oct 2013 20:13:21 GMT
     location: https://www.example.com
*/
    ret_t                  ret;
    chula_buffer_t         raw;
    hpack_header_store_t   store;
    hpack_header_parser_t *parser;
    unsigned int           offset   = 0;
    unsigned int           consumed = 0;

    chula_buffer_fake_str (&raw, "\x48\x82\x64\x02\x59\x85\xae\xc3\x77\x1a\x4b\x63\x96\xd0\x7a\xbe\x94\x10\x54\xd4\x44\xa8\x20\x05\x95\x04\x0b\x81\x66\xe0\x82\xa6\x2d\x1b\xff\x71\x91\x9d\x29\xad\x17\x18\x63\xc7\x8f\x0b\x97\xc8\xe9\xae\x82\xae\x43\xd3");

    hpack_header_store_init (&store);
    hpack_header_parser_new (&parser);
    hpack_header_parser_reg_store (parser, &store);

    /* Full header parse */
    ret = hpack_header_parser_all (parser, &raw, offset, &consumed);
    ch_assert (ret == ret_ok);
    ch_assert (consumed == raw.len);

    chula_print_repr (chula, buffer, &raw);
    chula_print_repr (hpack, header_store, &store);
    chula_print_repr (hpack, header_table, &parser->context.table);

    /* Checks */
    assert_store_n_eq (parser, 1, ":status",       "302");
    assert_store_n_eq (parser, 2, "cache-control", "private");
    assert_store_n_eq (parser, 3, "date",          "Mon, 21 Oct 2013 20:13:21 GMT");
    assert_store_n_eq (parser, 4, "location",      "https://www.example.com");

    assert_header_table_n_eq (parser, 1, "location",      "https://www.example.com");
    assert_header_table_n_eq (parser, 2, "date",          "Mon, 21 Oct 2013 20:13:21 GMT");
    assert_header_table_n_eq (parser, 3, "cache-control", "private");
    assert_header_table_n_eq (parser, 4, ":status",       "302");

    ch_assert (222 == hpack_header_table_get_size (&parser->context.table));

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
