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

/*
E.1.1.  Literal Header Field with Indexing

   The header field representation uses a literal name and a literal
   value.

   00                                      | == Literal indexed ==
   0a                                      |   Literal name (len = 10)
   6375 7374 6f6d 2d6b 6579                | custom-key
   0d                                      |   Literal value (len = 13)
   6375 7374 6f6d 2d68 6561 6465 72        | custom-header
                                           | -> custom-key: custom-header

   Header Table (after decoding):

   [  1] (s =  55) custom-key: custom-header
         Table size:  55
*/
#define LIT_W_IND  "\x00\x0a\x63\x75\x73\x74\x6f\x6d\x2d\x6b\x65\x79\x0d\x63\x75\x73\x74\x6f\x6d\x2d\x68\x65\x61\x64\x65\x72"
#define LIT_WO_IND "\x44\x0c\x2f\x73\x61\x6d\x70\x6c\x65\x2f\x70\x61\x74\x68"
#define INDEXED    "\x82"
#define REQUEST1   "\x82\x87\x86\x04\x0f\x77\x77\x77\x2e\x65\x78\x61\x6d\x70\x6c\x65\x2e\x63\x6f\x6d"

/* Literal Headers
 */

START_TEST (literal_w_index) {
    ret_t          ret;
    chula_buffer_t header;
    unsigned int   consumed = 0;

    chula_buffer_fake_str (&header, LIT_W_IND);
    ret = hpack_header_parse (&header, 0, &consumed);

    ck_assert (ret == ret_ok);
    ck_assert (consumed == sizeof(LIT_W_IND)-1);
}
END_TEST

START_TEST (literal_wo_index) {
    ret_t          ret;
    chula_buffer_t header;
    unsigned int   consumed = 0;

    chula_buffer_fake_str (&header, LIT_WO_IND);
    ret = hpack_header_parse (&header, 0, &consumed);

    ck_assert (ret == ret_ok);
    ck_assert (consumed == sizeof(LIT_WO_IND)-1);
}
END_TEST

START_TEST (indexed) {
    ret_t          ret;
    chula_buffer_t header;
    unsigned int   consumed = 0;

    chula_buffer_fake_str (&header, INDEXED);
    ret = hpack_header_parse (&header, 0, &consumed);

    ck_assert (ret == ret_ok);
    ck_assert (consumed == sizeof(INDEXED)-1);
}
END_TEST

START_TEST (request1) {
    ret_t          ret;
    chula_buffer_t header;
    unsigned int   consumed = 0;

    chula_buffer_fake_str (&header, REQUEST1);

    /* 82 - :method: GET */
    ret = hpack_header_parse (&header, consumed, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == 1);

    /* 87 - :scheme: http */
    ret = hpack_header_parse (&header, consumed, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == 2);

    /* 86 - :path: / */
    ret = hpack_header_parse (&header, consumed, &consumed);
    ck_assert (ret == ret_ok);
    ck_assert (consumed == 3);

    /* 04 - :authority: www.example.com */
    ret = hpack_header_parse (&header, consumed, &consumed);
    ck_assert (ret == ret_ok);
    printf ("consumed %d\n", consumed);
//  ck_assert (consumed == 20);
}
END_TEST


int
literal_headers (void)
{
    Suite *s1 = suite_create("Literal Headers");
//    check_add (s1, literal_wo_index);
//    check_add (s1, literal_w_index);
//    check_add (s1, indexed);
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
