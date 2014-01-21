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
#define decode_test(s1,s2,t1,t2) decode_string_test(s1, sizeof(s1)-1, s2, sizeof(s2)-1, t1, t2)
#define endecode_test(s1,t1,t2)  endecode_string_test(s1, sizeof(s1)-1, t1, t2)

/* Requests */
#define REQ_EXAMPLE_TEXT "www.example.com"
#define REQ_EXAMPLE_HUFF "\xdb\x6d\x88\x3e\x68\xd1\xcb\x12\x25\xba\x7f"
#define REQ_NOCACHE_TEXT "no-cache"
#define REQ_NOCACHE_HUFF "\x63\x65\x4a\x13\x98\xff"
#define REQ_CUSTOM_TEXT  "custom-value"
#define REQ_CUSTOM_HUFF  "\x4e\xb0\x8b\x74\x97\x9a\x17\xa8\xff"

/* Responses */
#define RES_DATE1_TEXT  "Mon, 21 Oct 2013 20:13:21 GMT"
#define RES_DATE1_HUFF  "\xa2\xfb\xa2\x03\x20\xf2\xab\x30\x31\x24\x01\x8b\x49\x0d\x32\x09\xe8\x77"
#define RES_DATE2_TEXT  "Mon, 21 Oct 2013 20:13:22 GMT"
#define RES_DATE2_HUFF  "\xa2\xfb\xa2\x03\x20\xf2\xab\x30\x31\x24\x01\x8b\x49\x0d\x33\x09\xe8\x77"
#define RES_PRIV_TEXT   "private"
#define RES_PRIV_HUFF   "\xc3\x1b\x39\xbf\x38\x7f"
#define RES_URL_TEXT    "https://www.example.com"
#define RES_URL_HUFF    "\xe3\x9e\x78\x64\xdd\x7a\xfd\x3d\x3d\x24\x87\x47\xdb\x87\x28\x49\x55\xf6\xff"
#define RES_GZIP_TEXT   "gzip"
#define RES_GZIP_HUFF   "\xe1\xfb\xb3\x0f"
#define RES_COOKIE_TEXT "foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; max-age=3600; version=1"
#define RES_COOKIE_HUFF \
    "\xdf\x7d\xfb\x36\xd3\xd9\xe1\xfc\xfc\x3f\xaf\xe7\xab\xfc\xfe\xfc"  \
    "\xbf\xaf\x3e\xdf\x2f\x97\x7f\xd3\x6f\xf7\xfd\x79\xf6\xf9\x77\xfd"  \
    "\x3d\xe1\x6b\xfa\x46\xfe\x10\xd8\x89\x44\x7d\xe1\xce\x18\xe5\x65"  \
    "\xf7\x6c\x2f"

/* Texts */
#define TEXT1 "When your hammer is C++, everything begins to look like a thumb."
#define TEXT2 "A SQL query goes into a bar, walks up to two tables and asks, 'Can I join you?'"
#define TEXT3 "Two bytes meet. The first byte asks, 'Are you ill?'. The second byte replies: No, just feeling a bit off."


static void
encode_string_test (char *str, size_t str_len,
                    char *enc, size_t enc_len,
                    const hpack_huffman_code_t *huffman_table)
{
    ret_t          ret;
    chula_buffer_t A;
    chula_buffer_t B    = CHULA_BUF_INIT;

    chula_buffer_fake (&A, str, str_len);

    ret = hpack_huffman_encode (&A, &B, huffman_table);
    ck_assert (ret == ret_ok);
    ck_assert (B.len == enc_len);

    for (int i=0; i<enc_len; i++)
        ck_assert (enc[i] == B.buf[i]);

    chula_buffer_mrproper (&B);
}

static void
decode_string_test (char *str, size_t str_len,
                    char *dec, size_t dec_len,
                    const hpack_huffman_code_t         *huffman_table,
                    const hpack_huffman_decode_table_t  table_decode)
{
    ret_t          ret;
    chula_buffer_t A;
    chula_buffer_t B    = CHULA_BUF_INIT;

    chula_buffer_fake (&A, str, str_len);

    ret = hpack_huffman_decode (&A, &B, huffman_table, table_decode);
    ck_assert (ret == ret_ok);
    ck_assert (B.len == dec_len);

    for (int i=0; i<dec_len; i++)
        ck_assert (dec[i] == B.buf[i]);

    chula_buffer_mrproper (&B);
}

static void
endecode_string_test (char *str, size_t str_len,
                      const hpack_huffman_code_t         *huffman_table,
                      const hpack_huffman_decode_table_t  table_decode)
{
    ret_t          ret;
    chula_buffer_t A;
    chula_buffer_t B    = CHULA_BUF_INIT;
    chula_buffer_t C    = CHULA_BUF_INIT;

    chula_buffer_fake (&A, str, str_len);

    ret = hpack_huffman_encode (&A, &B, huffman_table);
    ck_assert (ret == ret_ok);

    ret = hpack_huffman_decode (&B, &C, huffman_table, table_decode);
    ck_assert (ret == ret_ok);

    ck_assert (A.len == C.len);
    for (int i=0; i<A.len; i++)
        ck_assert (A.buf[i] == C.buf[i]);

    chula_buffer_mrproper (&B);
    chula_buffer_mrproper (&C);
}



/* Encode
 */

/* Requests */
START_TEST (request_www_example_com) {
    encode_test (REQ_EXAMPLE_TEXT, REQ_EXAMPLE_HUFF, hpack_huffman_request);
}
END_TEST
START_TEST (request_no_cache) {
    encode_test (REQ_NOCACHE_TEXT, REQ_NOCACHE_HUFF, hpack_huffman_request);
}
END_TEST
START_TEST (request_custom_value) {
    encode_test (REQ_CUSTOM_TEXT, REQ_CUSTOM_HUFF, hpack_huffman_request);
}
END_TEST

/* Responses */
START_TEST (response_date1) {
    encode_test (RES_DATE1_TEXT, RES_DATE1_HUFF, hpack_huffman_response);
}
END_TEST
START_TEST (response_date2) {
    encode_test (RES_DATE2_TEXT, RES_DATE2_HUFF, hpack_huffman_response);
}
END_TEST
START_TEST (response_private) {
    encode_test (RES_PRIV_TEXT, RES_PRIV_HUFF, hpack_huffman_response);
}
END_TEST
START_TEST (response_url) {
    encode_test (RES_URL_TEXT, RES_URL_HUFF, hpack_huffman_response);
}
END_TEST
START_TEST (response_gzip) {
    encode_test (RES_GZIP_TEXT, RES_GZIP_HUFF, hpack_huffman_response);
}
END_TEST
START_TEST (response_cookie) {
    encode_test (RES_COOKIE_TEXT, RES_COOKIE_HUFF, hpack_huffman_response);
}
END_TEST


/* Decode
 */

/* Requests */
START_TEST (decode_request_www_example_com) {
    decode_test (REQ_EXAMPLE_HUFF, REQ_EXAMPLE_TEXT, hpack_huffman_request, decode_req_table);
}
END_TEST
START_TEST (decode_request_no_cache) {
    decode_test (REQ_NOCACHE_HUFF, REQ_NOCACHE_TEXT, hpack_huffman_request, decode_req_table);
}
END_TEST
START_TEST (decode_request_custom) {
    decode_test (REQ_CUSTOM_HUFF, REQ_CUSTOM_TEXT, hpack_huffman_request, decode_req_table);
}
END_TEST

/* Responses */
START_TEST (decode_response_date1) {
    decode_test (RES_DATE1_HUFF, RES_DATE1_TEXT, hpack_huffman_response, decode_res_table);
}
END_TEST
START_TEST (decode_response_date2) {
    decode_test (RES_DATE2_HUFF, RES_DATE2_TEXT, hpack_huffman_response, decode_res_table);
}
END_TEST
START_TEST (decode_response_priv) {
    decode_test (RES_PRIV_HUFF, RES_PRIV_TEXT, hpack_huffman_response, decode_res_table);
}
END_TEST
START_TEST (decode_response_url) {
    decode_test (RES_URL_HUFF, RES_URL_TEXT, hpack_huffman_response, decode_res_table);
}
END_TEST
START_TEST (decode_response_gzip) {
    decode_test (RES_GZIP_HUFF, RES_GZIP_TEXT, hpack_huffman_response, decode_res_table);
}
END_TEST
START_TEST (decode_response_cookie) {
    decode_test (RES_COOKIE_HUFF, RES_COOKIE_TEXT, hpack_huffman_response, decode_res_table);
}
END_TEST

/* Encode-Decode
 */

/* Requests */
START_TEST (endecode_req_text1) {
    endecode_test (TEXT1, hpack_huffman_request, decode_req_table);
}
END_TEST
START_TEST (endecode_req_text2) {
    endecode_test (TEXT2, hpack_huffman_request, decode_req_table);
}
END_TEST
START_TEST (endecode_req_text3) {
    endecode_test (TEXT3, hpack_huffman_request, decode_req_table);
}
END_TEST

/* Responses */
START_TEST (endecode_res_text1) {
    endecode_test (TEXT1, hpack_huffman_response, decode_res_table);
}
END_TEST
START_TEST (endecode_res_text2) {
    endecode_test (TEXT2, hpack_huffman_response, decode_res_table);
}
END_TEST
START_TEST (endecode_res_text3) {
    endecode_test (TEXT3, hpack_huffman_response, decode_res_table);
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
decode_requests (void)
{
    Suite *s1 = suite_create("Request Decoding");
    check_add (s1, decode_request_www_example_com);
    check_add (s1, decode_request_no_cache);
    check_add (s1, decode_request_custom);
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
decode_responses (void)
{
    Suite *s1 = suite_create("Response Decoding");
    check_add (s1, decode_response_date1);
    check_add (s1, decode_response_date2);
    check_add (s1, decode_response_priv);
    check_add (s1, decode_response_url);
    check_add (s1, decode_response_gzip);
    check_add (s1, decode_response_cookie);
    run_test (s1);
}

int
endecode_request (void)
{
    Suite *s1 = suite_create("Request Enc+Decode");
    check_add (s1, endecode_req_text1);
    check_add (s1, endecode_req_text2);
    check_add (s1, endecode_req_text3);
    run_test (s1);
}

int
endecode_responses (void)
{
    Suite *s1 = suite_create("Response Enc+Decode");
    check_add (s1, endecode_res_text1);
    check_add (s1, endecode_res_text2);
    check_add (s1, endecode_res_text3);
    run_test (s1);
}

int
huffman_tests (void)
{
    int ret;

    ret  = encode_requests();
    ret += encode_responses();
    ret += decode_requests();
    ret += decode_responses();
    ret += endecode_request();
    ret += endecode_responses();

    return ret;
}
