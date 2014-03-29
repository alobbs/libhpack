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
#define HUFF_EXAMPLE_HUFF "\xdb\x6d\x88\x3e\x68\xd1\xcb\x12\x25\xba\x7f"

#define HUFF_NOCACHE_TEXT "no-cache"
#define HUFF_NOCACHE_HUFF "\x63\x65\x4a\x13\x98\xff"

#define HUFF_CUSTOM_TEXT  "custom-value"
#define HUFF_CUSTOM_HUFF  "\x4e\xb0\x8b\x74\x97\x9a\x17\xa8\xff"

/* Responses */
#define HUFF_DATE1_TEXT  "Mon, 21 Oct 2013 20:13:21 GMT"
#define HUFF_DATE1_HUFF  "\xef\x6b\x3a\x7a\x0e\x6e\x8f\xa2\x63\xd0\x72\x9a\x6e\x83\x97\xd8\x69\xbd\x87\x37\x47\xbb\xbf\xc7"

#define HUFF_DATE2_TEXT  "Mon, 21 Oct 2013 20:13:22 GMT"
#define HUFF_DATE2_HUFF  "\xef\x6b\x3a\x7a\x0e\x6e\x8f\xa2\x63\xd0\x72\x9a\x6e\x83\x97\xd8\x69\xbd\x87\x3f\x47\xbb\xbf\xc7"

#define HUFF_PRIV_TEXT   "private"
#define HUFF_PRIV_HUFF   "\x73\xd5\xcd\x11\x1f"

#define HUFF_URL_TEXT    "https://www.example.com"
#define HUFF_URL_HUFF    "\xce\x31\x74\x3d\x80\x1b\x6d\xb1\x07\xcd\x1a\x39\x62\x44\xb7\x4f"

#define HUFF_GZIP_TEXT   "gzip"
#define HUFF_GZIP_HUFF   "\xcb\xd5\x4e"

#define HUFF_COOKIE_TEXT "foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; max-age=3600; version=1"
#define HUFF_COOKIE_HUFF                                                 \
    "\xc5\xad\xb7\x7f\x87\x6f\xc7\xfb\xf7\xfd\xbf\xbe\xbf\xf3\xf7\xf4"  \
    "\xfb\x7e\xbb\xbe\x9f\x5f\x87\xe3\x7f\xef\xed\xfa\xee\xfa\x7c\x3f"  \
    "\x1d\x5d\x1a\x23\xce\x54\x64\x36\xcd\x49\x4b\xd5\xd1\xcc\x5f\x05"  \
    "\x35\x96\x9b"

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
