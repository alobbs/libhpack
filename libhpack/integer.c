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

/* Current ref:
 * http://tools.ietf.org/html/draft-ietf-httpbis-header-compression-05
 */

#include "config.h"
#include "integer.h"
#include <stdio.h>
#include <math.h>

/** Limits by number of bits
 */
static const unsigned char limits[] = {0, 1, 3, 7, 15, 31, 63, 127, 255};

/**  Integer encoding
 *
 * Encodes an integer number into HPACK's int representation. A prefix
 * length (in bits) can be provided so the first byte can be partially
 * utilized even if a few bits have maybe used already.
 *
 * @param      N       Number of bits of the prefix
 * @param      value   Number to encode
 * @param[out] mem     Memory where the number has to be encoded to
 * @param[out] mem_len Total amount of memory (in bytes) used to encode the number
 * @retval ret_ok  number converted successfully
 */
ret_t
hpack_integer_encode (int            N,
                      unsigned int   value,
                      unsigned char *mem,
                      unsigned char *mem_len)
{
    int i = 0;

    /* N is always between 1 and 8 bits [4.1.1.]
     */
    const unsigned char limit = limits[N];

    /* An integer is represented in two parts:
     * - A prefix that fills the current octet (N bits length)
     * - An optional list of octets
     */
    if (value < limit) {
        mem[i] = (mem[i] & ~limit) | (unsigned char)value;
        *mem_len = ++i;
        return ret_ok;
    }

    /* the bits of the prefix are set to 1 */
    mem[i] = (mem[i] & ~limit) | (unsigned char)limit;

    /* allowed by an unsigned variable length integer */
    i++;
    value -= limit;
    while (value >= 128) {
        unsigned char t = (char)(128 + (value % 128));
        mem[i++] = t;
        value /= 128;
    }

    mem[i++] = (char)value;
    *mem_len = i;

    return ret_ok;
}


/** Integer decoding
 *
 * Decodes an integer number from a HPACK representation in memory. A
 * prefix length (in bits) is provided just in case there were bits in
 * the first byte that shouldn't be consumed.
 *
 * The function will stop reading memory when the number is completed.
 * The mem_len parameter is the maximum length of memory it could try
 * to read to parse the number, but it doesn't necesarily mean that it
 * will read it all.
 *
 * @param      N        Number of bits of the prefix
 * @param      mem      Pointer to the first byte of memory containing the number
 * @param      mem_len  Length (in bytes) of the memory holding the number
 * @param[out] ret      Pointer to an integer to store the decoded number
 * @param[out] consumed Memory processed to read the number (in bytes)
 * @retval ret_ok Number was read successfuly
 * @retval ret_error Incorrect format
 */
ret_t
hpack_integer_decode (int            N,
                      unsigned char *mem,
                      unsigned char  mem_len,
                      unsigned int  *ret,
                      unsigned int  *consumed)
{
    const unsigned char limit   = limits[N];
    int                 M       = 0;
    unsigned long       decoded = 0;

    /* Trivial 1 byte number
     */
    *ret = mem[0] & limit;

    if (*ret < limit) {
        *consumed = 1;
        return ret_ok;
    }

    /* Unsigned variable length integer
     */
    decoded = *ret;
    for (int i=1; i < MIN(mem_len,VLQ_MAX_LEN_INTEGER); i++) {
        decoded += (mem[i] & 127) * (1L << M);
        if (128 != (mem[i] & 128)) {
            if (decoded > UINT_MAX)
                return ret_error;
            *consumed = i + 1;
            *ret = (unsigned int) decoded;
            return ret_ok;
        }

        M += 7;
    }
    return ret_error;
}
