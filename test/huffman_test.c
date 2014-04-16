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

#define encode_test(s1,s2) encode_string_test(s1, sizeof(s1)-1, s2, sizeof(s2)-1)
#define decode_test(s1,s2) decode_string_test(s1, sizeof(s1)-1, s2, sizeof(s2)-1)
#define endecode_test(s1)  endecode_string_test(s1, sizeof(s1)-1)

/* Requests */
#define HUFF_EXAMPLE_TEXT "www.example.com"
#define HUFF_EXAMPLE_HUFF "\xe7\xcf\x9b\xeb\xe8\x9b\x6f\xb1\x6f\xa9\xb6\xff"

#define HUFF_NOCACHE_TEXT "no-cache"
#define HUFF_NOCACHE_HUFF "\xb9\xb9\x94\x95\x56\xbf"

#define HUFF_CUSTOM_TEXT  "custom-value"
#define HUFF_CUSTOM_HUFF  "\x57\x1c\x5c\xdb\x73\x72\x4d\x9c\x57"

/* Responses */
#define HUFF_DATE1_TEXT  "Mon, 21 Oct 2013 20:13:21 GMT"
#define HUFF_DATE1_HUFF  "\xd6\xdb\xb2\x98\x84\xde\x2a\x71\x88\x05\x06\x20\x98\x51\x31\x09\xb5\x6b\xa3"

#define HUFF_DATE2_TEXT  "Mon, 21 Oct 2013 20:13:22 GMT"
#define HUFF_DATE2_HUFF  "\xd6\xdb\xb2\x98\x84\xde\x2a\x71\x88\x05\x06\x20\x98\x51\x31\x11\xb5\x6b\xa3"

#define HUFF_PRIV_TEXT   "private"
#define HUFF_PRIV_HUFF   "\xbf\x06\x72\x4b\x97"

#define HUFF_URL_TEXT    "https://www.example.com"
#define HUFF_URL_HUFF    "\xad\xce\xbf\x19\x8e\x7e\x7c\xf9\xbe\xbe\x89\xb6\xfb\x16\xfa\x9b\x6f"

#define HUFF_GZIP_TEXT   "gzip"
#define HUFF_GZIP_HUFF   "\xab\xdd\x97\xff"

#define HUFF_COOKIE_TEXT "foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; max-age=3600; version=1"
#define HUFF_COOKIE_HUFF                                                 \
    "\xe0\xd6\xcf\x9f\x6e\x8f\x9f\xd3\xe5\xf6\xfa\x76\xfe\xfd\x3c\x7e"  \
    "\xdf\x9e\xff\x1f\x2f\x0f\x3c\xfe\x9f\x6f\xcf\x7f\x8f\x87\x9f\x61"  \
    "\xad\x4f\x4c\xc9\xa9\x73\xa2\x20\x0e\xc3\x72\x5e\x18\xb1\xb7\x4e"  \
    "\x3f"

/* Texts */
#define TEXT1 "When your hammer is C++, everything begins to look like a thumb."
#define TEXT2 "A SQL query goes into a bar, walks up to two tables and asks, 'Can I join you?'"
#define TEXT3 "Two bytes meet. The first byte asks, 'Are you ill?'. The second byte replies: No, just feeling a bit off."


static void
encode_string_test (char *str, size_t str_len,
                    char *enc, size_t enc_len)
{
    ret_t          ret;
    chula_buffer_t A;
    chula_buffer_t B    = CHULA_BUF_INIT;

    chula_buffer_fake (&A, str, str_len);

    ret = hpack_huffman_encode (&A, &B);
    ch_assert (ret == ret_ok);
    ch_assert (B.len == enc_len);

    chula_print_repr (chula, buffer, &B);
    printf ("Source: %s\n\n", A.buf);

    for (uint64_t i=0; i<enc_len; i++)
        ch_assert ((uint8_t)enc[i] == B.buf[i]);

    chula_buffer_mrproper (&B);
}

static void
decode_string_test (char *str, size_t str_len,
                    char *dec, size_t dec_len)
{
    ret_t                          ret;
    chula_buffer_t                 A;
    chula_buffer_t                 B       = CHULA_BUF_INIT;
    hpack_huffman_decode_context_t context = HUFFMAN_DEC_CTX_INIT;

    chula_buffer_fake (&A, str, str_len);

    ret = hpack_huffman_decode (&A, &B, &context);

    chula_print_repr (chula, buffer, &A);
    printf ("Decoded: %s\n\n", B.buf);

    ch_assert (ret == ret_ok);
    ch_assert (B.len == dec_len);

    for (uint64_t i=0; i<dec_len; i++)
        ch_assert ((uint8_t)dec[i] == B.buf[i]);

    chula_buffer_mrproper (&B);
}

static void
endecode_string_test (char *str, size_t str_len)
{
    ret_t          ret;
    chula_buffer_t A;
    chula_buffer_t B    = CHULA_BUF_INIT;
    chula_buffer_t C    = CHULA_BUF_INIT;
    hpack_huffman_decode_context_t context = HUFFMAN_DEC_CTX_INIT;

    chula_buffer_fake (&A, str, str_len);

    ret = hpack_huffman_encode (&A, &B);
    ch_assert (ret == ret_ok);

    chula_print_repr (chula, buffer, &B);
    printf ("Source: %s\n\n", A.buf);

    ret = hpack_huffman_decode (&B, &C, &context);
    ch_assert (ret == ret_ok);

    ch_assert (A.len == C.len);
    for (uint64_t i=0; i<A.len; i++)
        ch_assert (A.buf[i] == C.buf[i]);

    chula_buffer_mrproper (&B);
    chula_buffer_mrproper (&C);
}



/* Encode
 */
START_TEST (request_www_example_com) {
    encode_test (HUFF_EXAMPLE_TEXT, HUFF_EXAMPLE_HUFF);
}
END_TEST
START_TEST (request_no_cache) {
    encode_test (HUFF_NOCACHE_TEXT, HUFF_NOCACHE_HUFF);
}
END_TEST
START_TEST (request_custom_value) {
    encode_test (HUFF_CUSTOM_TEXT, HUFF_CUSTOM_HUFF);
}
END_TEST
START_TEST (response_date1) {
    encode_test (HUFF_DATE1_TEXT, HUFF_DATE1_HUFF);
}
END_TEST
START_TEST (response_date2) {
    encode_test (HUFF_DATE2_TEXT, HUFF_DATE2_HUFF);
}
END_TEST
START_TEST (response_private) {
    encode_test (HUFF_PRIV_TEXT, HUFF_PRIV_HUFF);
}
END_TEST
START_TEST (response_url) {
    encode_test (HUFF_URL_TEXT, HUFF_URL_HUFF);
}
END_TEST
START_TEST (response_gzip) {
    encode_test (HUFF_GZIP_TEXT, HUFF_GZIP_HUFF);
}
END_TEST
START_TEST (response_cookie) {
    encode_test (HUFF_COOKIE_TEXT, HUFF_COOKIE_HUFF);
}
END_TEST


/* Decode
 */
START_TEST (decode_www_example_com) {
    decode_test (HUFF_EXAMPLE_HUFF, HUFF_EXAMPLE_TEXT);
}
END_TEST
START_TEST (decode_no_cache) {
    decode_test (HUFF_NOCACHE_HUFF, HUFF_NOCACHE_TEXT);
}
END_TEST
START_TEST (decode_custom) {
    decode_test (HUFF_CUSTOM_HUFF, HUFF_CUSTOM_TEXT);
}
END_TEST
START_TEST (decode_date1) {
    decode_test (HUFF_DATE1_HUFF, HUFF_DATE1_TEXT);
}
END_TEST
START_TEST (decode_date2) {
    decode_test (HUFF_DATE2_HUFF, HUFF_DATE2_TEXT);
}
END_TEST
START_TEST (decode_priv) {
    decode_test (HUFF_PRIV_HUFF, HUFF_PRIV_TEXT);
}
END_TEST
START_TEST (decode_url) {
    decode_test (HUFF_URL_HUFF, HUFF_URL_TEXT);
}
END_TEST
START_TEST (decode_gzip) {
    decode_test (HUFF_GZIP_HUFF, HUFF_GZIP_TEXT);
}
END_TEST
START_TEST (decode_cookie) {
    decode_test (HUFF_COOKIE_HUFF, HUFF_COOKIE_TEXT);
}
END_TEST

/* Encode-Decode
 */

/* Requests */
START_TEST (endecode_text1) {
    endecode_test (TEXT1);
}
END_TEST
START_TEST (endecode_text2) {
    endecode_test (TEXT2);
}
END_TEST
START_TEST (endecode_text3) {
    endecode_test (TEXT3);
}
END_TEST


int
encode (void)
{
    Suite *s1 = suite_create("Huffman Encoding");
    check_add (s1, request_www_example_com);
    check_add (s1, request_no_cache);
    check_add (s1, request_custom_value);
    check_add (s1, response_date1);
    check_add (s1, response_date2);
    check_add (s1, response_private);
    check_add (s1, response_url);
    check_add (s1, response_gzip);
    check_add (s1, response_cookie);
    run_test (s1);
}

int
decode (void)
{
    Suite *s1 = suite_create("Huffman Decoding");
    check_add (s1, decode_www_example_com);
    check_add (s1, decode_no_cache);
    check_add (s1, decode_custom);
    check_add (s1, decode_date1);
    check_add (s1, decode_date2);
    check_add (s1, decode_priv);
    check_add (s1, decode_url);
    check_add (s1, decode_gzip);
    check_add (s1, decode_cookie);
    run_test (s1);
}

int
endecode (void)
{
    Suite *s1 = suite_create("Huffman Encode+Decode");
    check_add (s1, endecode_text1);
    check_add (s1, endecode_text2);
    check_add (s1, endecode_text3);
    run_test (s1);
}

int
huffman_tests (void)
{
    int re;

    re = 0;
    re  = encode();
    re += decode();
    re += endecode();

    return re;
}
