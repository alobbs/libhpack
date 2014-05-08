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

#include <time.h>

#include <libhpack/libhpack.h>
#include <libchula-qa/libchula-qa.h>
#include <libchula-qa/testing_macros-internal.h>


/* All examples came from:
 * http://tools.ietf.org/html/draft-ietf-httpbis-header-compression-05
 */

START_TEST (encode_10_5bits)
{
    unsigned char len   = 0xFF;
    unsigned char tmp[] = {0,0};

    /* 4.1.1.1.  Example 1: Encoding 10 using a 5-bit prefix
     */
    hpack_integer_encode (5, 10, tmp, &len);

    /* Check output */
    ch_assert (len == 1);
    ch_assert (tmp[0] == 10);
}
END_TEST

START_TEST (encode_1337_5bits)
{
    unsigned char len   = 0xFF;
    unsigned char tmp[] = {0,0,0,0,0};

    /* 4.1.1.2.  Example 2: Encoding 1337 using a 5-bit prefix
     */
    hpack_integer_encode (5, 1337, tmp, &len);

    /* Check output */
    ch_assert (len == 3);
    ch_assert (tmp[0] == 31);
    ch_assert (tmp[1] == 154);
    ch_assert (tmp[2] == 10);
}
END_TEST

START_TEST (encode_42_8bits)
{
    unsigned char len   = 0xFF;
    unsigned char tmp[] = {0,0};

    /* 4.1.1.3.  Example 3: Encoding 42 starting at an octet-boundary
     */
    hpack_integer_encode (8, 42, tmp, &len);

    /* Check output */
    ch_assert (len == 1);
    ch_assert (tmp[0] == 42);
}
END_TEST

START_TEST (encode_12_6bits)
{
    unsigned char len   = 0xFF;
    unsigned char tmp[] = {0xC0,0};

    hpack_integer_encode (6, 12, tmp, &len);

    /* Check output */
    ch_assert (len == 1);
    ch_assert (tmp[0] == (0xC0 | 12));
}
END_TEST

START_TEST (encode_1338_5bits)
{
    unsigned char len   = 0xFF;
    unsigned char tmp[] = {0xA0,0,0,0,0};

    hpack_integer_encode (5, 1338, tmp, &len);

    /* Check output */
    ch_assert (len == 3);
    ch_assert (tmp[0] == (0xA0 | 31));
    ch_assert (tmp[1] == 155);
    ch_assert (tmp[2] == 10);
}
END_TEST

START_TEST (decode_19_6bits)
{
    ret_t         ret;
    uint32_t      num   = 0;
    unsigned int  con   = 0;
    unsigned char tmp[] = {0x80 | 19};

    ret = hpack_integer_decode (6, tmp, 1, &num, &con);
    ch_assert (ret == ret_ok);
    ch_assert (con == 1);
    ch_assert (num == 19);
}
END_TEST

START_TEST (decode_34_6bits)
{
    ret_t         ret;
    uint32_t      num   = 0;
    unsigned int  con   = 0;
    unsigned char tmp[] = {34,1};

    /* 34 = 00[100010]
     *
     * This test checks that even if the first bit of the significant
     * no subsequent bytes are tried to be read (for that, that part
     * should be filled with 1s).
     */
    ret = hpack_integer_decode (6, tmp, 1, &num, &con);
    ch_assert (ret == ret_ok);
    ch_assert (con == 1);
    ch_assert (num == 34);
}
END_TEST

START_TEST (decode_1337_5bits)
{
    ret_t         ret;
    uint32_t      num   = 0;
    unsigned int  con   = 0;
    unsigned char tmp[] = {31,154,10};

    ret = hpack_integer_decode (5, tmp, 3, &num, &con);

    ch_assert (ret == ret_ok);
    ch_assert (num == 1337);
    ch_assert (con == 3);
}
END_TEST


START_TEST (en_decode_2147483647_5bits)
{
    unsigned char tmp[6];
    unsigned char tmp_len  = 0;
    int           err      = 0;
    uint32_t      num      = 0;
    unsigned int  con      = 0;

    hpack_integer_encode (5, 2147483647, tmp, &tmp_len);
    ch_assert (tmp_len > 0);

    err = hpack_integer_decode (5, tmp, tmp_len, &num, &con);
    ch_assert (err == 0);
    ch_assert (con == 6);
    ch_assert (num == 2147483647);
}
END_TEST

START_TEST (decode_too_big_for_int)
{
    ret_t         ret;
    unsigned int  num    = 0;
    unsigned int  con    = 0;
    const char   *data64 = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
    const char   *data32 = "\xFF\xFF\xFF\xFF\xFF\x0A";
    const char   *data;

    /* ffff ffff ff0a = 2952790270 [for 32 bits int]
     * ffff ffff ffff ffff ffff = 9223372036854776062 [for 64 bits int]
     *
     * This test tries to decode a number too big for the signed integer type of
     * the machine, which would be an error if we were working with signed int.
     */
    data = sizeof(int) > 32? data64 : data32;

    ret = hpack_integer_decode (8, (unsigned char *)data, strlen(data), &num, &con);
    ch_assert (ret == ret_ok);
    ch_assert (num >= 0);
    ch_assert (num == (sizeof(int) > 32? 9223372036854776062u : 2952790270u));
    ch_assert (con == strlen(data));
}
END_TEST

START_TEST (decode_too_big_for_uint)
{
    ret_t         ret;
    unsigned int  num    = 0;
    unsigned int  con    = 0;
    const char   *data64 = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x02";
    const char   *data32 = "\xFF\xFF\xFF\xFF\xFF\x14";
    const char   *data;

    /* ffff ffff ff14 = 5637144830 [for 32 bits unsigned int]
     * ffff ffff ffff ffff ffff 02 = 9223372036854776062 [for 64 bits unsigned int]
     *
     * This test tries to decode a number too big for the unsigned integer type of
     * the machine, which should be an error.
     */
    data = sizeof(int) > 32? data64 : data32;
    ret = hpack_integer_decode (8, (unsigned char *)data, strlen(data), &num, &con);
    ch_assert (ret != ret_ok);
    ch_assert (con == 0);
}
END_TEST

START_TEST (decode_too_big_for_ulong)
{
    ret_t         ret;
    unsigned int  num    = 0;
    unsigned int  con    = 0;
    const char   *data64 = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01";
    const char   *data32 = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x02";
    const char   *data;

    /* ffff ffff ffff ffff ffff 02 = 9223372036854776062 [for 32 bits unsigned long]
     * ffff ffff ffff ffff ffff ffff ffff ffff ffff ff01 = 680564652712219169506953685007735980286 [for 64 bits unsigned long]
     *
     * This test tries to decode a number too big for the unsigned long type of
     * the machine, which should be an error.
     */
    data = sizeof(int) > 32? data64 : data32;
    ret = hpack_integer_decode (8, (unsigned char *)data, strlen(data), &num, &con);
    ch_assert (ret != ret_ok);
    ch_assert (con == 0);
}
END_TEST

START_TEST (decode_too_many_zeros)
{
    ret_t        ret;
    char         data[256];
    unsigned int num         = 0;
    unsigned int con         = 0;

    /* According to the draft a large number of zero values MUST be treated as a
     * decoding error.
     */
    data[0] = 0xFF;
    memset (data+1, 0x80, sizeof(data)-1);
    data[sizeof(data)-1] |= 1;

    ret = hpack_integer_decode (8, (unsigned char *)data, (uint8_t)sizeof(data), &num, &con);
    ch_assert (ret != ret_ok);
    ch_assert (con == 0);
}
END_TEST

START_TEST (en_decode_max_uint_5bits)
{
    unsigned char tmp[15];
    unsigned char tmp_len  = 0;
    int           err      = 0;
    uint32_t      num      = 0;
    unsigned int  con      = 0;

    /* This test encodes and decodes the maximum unsigned int to confirm that
     * it also works in that end.
     */
    hpack_integer_encode (5, UINT_MAX, tmp, &tmp_len);
    ch_assert (tmp_len > 0);

    err = hpack_integer_decode (5, tmp, tmp_len, &num, &con);
    ch_assert (err == 0);
    ch_assert (con == tmp_len);
    ch_assert (num == UINT_MAX);
}
END_TEST

START_TEST (en_decode_2nd_byte_0)
{
    unsigned char tmp[2];
    unsigned char tmp_len  = 0;
    int           err      = 0;
    uint32_t      num      = 0;
    unsigned int  con      = 0;

    /* This test encodes and decodes a number that will use the limit of the
     * first byte and have the second byte as zero.
     */
    hpack_integer_encode (8, 255, tmp, &tmp_len);
    ch_assert (tmp_len > 0);

    err = hpack_integer_decode (8, tmp, tmp_len, &num, &con);
    ch_assert (err == 0);
    ch_assert (con == tmp_len);
    ch_assert (num == 255);
}
END_TEST

START_TEST (range_0_65535)
{
    ret_t    ret;
    uint8_t  tmp[64];
    uint64_t total    = 0;
    time_t   starting = time(NULL);

    for (uint8_t bitsn=1; bitsn <= 8; bitsn++) {
        for (uint32_t num=0; num < 65535; num++) {
            uint8_t  len1 = 0;
            uint32_t len2 = 0;
            uint32_t val  = 0;

            /* Clear memory */
            memset (tmp, 0, sizeof(tmp));

            /* Encode */
            ret = hpack_integer_encode (bitsn, num, tmp, &len1);
            ch_assert (ret == ret_ok);

            /* Decode */
            ret = hpack_integer_decode (bitsn, tmp, sizeof(tmp), &val, &len2);
            ch_assert (ret == ret_ok);

            /* Tests */
            ch_assert (len1 == len2);
            ch_assert (val == num);

            total++;
        }
    }

    uint32_t secs = MAX(1, time(NULL) - starting);
    printf ("%llu encoding+decodings in %d secs (%llu per sec)\n", total, secs, total/secs);
}
END_TEST



int
encode_tests (void)
{
    Suite *s1 = suite_create("Encoding");

    check_add (s1, encode_10_5bits);
    check_add (s1, encode_1337_5bits);
    check_add (s1, encode_42_8bits);
    check_add (s1, encode_12_6bits);
    check_add (s1, encode_1338_5bits);

    run_test (s1);
}

int
decode_tests (void)
{
    Suite *s1 = suite_create("Decoding");

    check_add (s1, decode_19_6bits);
    check_add (s1, decode_34_6bits);
    check_add (s1, decode_1337_5bits);
    check_add (s1, en_decode_2147483647_5bits);
    check_add (s1, decode_too_big_for_int);
    check_add (s1, decode_too_big_for_uint);
    check_add (s1, decode_too_big_for_ulong);
    check_add (s1, en_decode_max_uint_5bits);
    check_add (s1, en_decode_2nd_byte_0);
    check_add (s1, decode_too_many_zeros);

    run_test (s1);
}

int
range_tests (void)
{
    Suite *s1 = suite_create("Range en/decoding");

    check_add (s1, range_0_65535);

    run_test (s1);
}

int
integer_tests (void)
{
    int ret;

    ret  = encode_tests();
    ret += decode_tests();
    ret += range_tests();

    return ret;
}
