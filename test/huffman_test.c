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
#include "libhpack/huffman.h"
#include "libhpack/huffman_tables.h"

#define encode_test(s1,s2,table) encode_string_test(s1, sizeof(s1)-1, s2, sizeof(s2)-1, table)

static void
encode_string_test (char *str, int str_len,
                    char *enc, int enc_len,
                    const hpack_huffman_code_t *huffman_table)
{
    ret_t          ret;
    chula_buffer_t A    = CHULA_BUF_INIT;
    chula_buffer_t B    = CHULA_BUF_INIT;

    chula_buffer_add (&A, str, str_len);
    chula_buffer_ensure_size (&B, 10*A.len);

    char tmp1 = B.buf[0];
    char tmp2 = B.buf[enc_len-1];

    ret = hpack_huffman_encode (&A, &B, huffman_table);
    ck_assert (ret == ret_ok);
    ck_assert (B.len == enc_len);

    int cmp = memcmp (B.buf, enc, enc_len);
    ck_assert (cmp == 0);

    chula_buffer_mrproper (&A);
    chula_buffer_mrproper (&B);
}


/* Requests
 */
START_TEST (request_www_example_com) {
    encode_test ("www.example.com", "\xdb\x6d\x88\x3e\x68\xd1\xcb\x12\x25\xba\x7f", hpack_huffman_request);
}
END_TEST
START_TEST (request_no_cache) {
    encode_test ("no-cache", "\x63\x65\x4a\x13\x98\xff", hpack_huffman_request);
}
END_TEST
START_TEST (request_custom_value) {
    encode_test ("custom-value", "\x4e\xb0\x8b\x74\x97\x9a\x17\xa8\xff", hpack_huffman_request);
}
END_TEST


/* Responses
 */
START_TEST (response_date1) {
    encode_test ("Mon, 21 Oct 2013 20:13:21 GMT", "\xa2\xfb\xa2\x03\x20\xf2\xab\x30\x31\x24\x01\x8b\x49\x0d\x32\x09\xe8\x77", hpack_huffman_response);
}
END_TEST
START_TEST (response_date2) {
    encode_test ("Mon, 21 Oct 2013 20:13:22 GMT", "\xa2\xfb\xa2\x03\x20\xf2\xab\x30\x31\x24\x01\x8b\x49\x0d\x33\x09\xe8\x77", hpack_huffman_response);
}
END_TEST
START_TEST (response_private) {
    encode_test ("private", "\xc3\x1b\x39\xbf\x38\x7f", hpack_huffman_response);
}
END_TEST
START_TEST (response_url) {
    encode_test ("https://www.example.com", "\xe3\x9e\x78\x64\xdd\x7a\xfd\x3d\x3d\x24\x87\x47\xdb\x87\x28\x49\x55\xf6\xff", hpack_huffman_response);
}
END_TEST
START_TEST (response_gzip) {
    encode_test ("gzip", "\xe1\xfb\xb3\x0f", hpack_huffman_response);
}
END_TEST
START_TEST (response_cookie) {
    encode_test ("foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; max-age=3600; version=1",
                 "\xdf\x7d\xfb\x36\xd3\xd9\xe1\xfc\xfc\x3f\xaf\xe7\xab\xfc\xfe\xfc\xbf\xaf\x3e\xdf\x2f\x97\x7f\xd3\x6f\xf7\xfd\x79\xf6\xf9\x77\xfd\x3d\xe1\x6b\xfa\x46\xfe\x10\xd8\x89\x44\x7d\xe1\xce\x18\xe5\x65\xf7\x6c\x2f",
                 hpack_huffman_response);
}
END_TEST


int
encode_requests (void)
{
    Suite *s1 = suite_create("Request Encoding");
    check_add (s1, request_www_example_com);
    check_add (s1, request_no_cache);
    check_add (s1, request_custom_value);
    run_test (s1);
}

int
encode_responses (void)
{
    Suite *s1 = suite_create("Response Encoding");
    check_add (s1, response_date1);
    check_add (s1, response_date2);
    check_add (s1, response_private);
    check_add (s1, response_url);
    check_add (s1, response_gzip);
    check_add (s1, response_cookie);
    run_test (s1);
}

int
huffman_tests (void)
{
    int ret;

    ret  = encode_requests();
    ret += encode_responses();

    return ret;
}
