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

#include <stdio.h>
#include <math.h>

void
integer_encode (int            N,        /* Prefix length in bits  */
                int            value,    /* Number to encode       */
                unsigned char *mem,      /* Memory to encode it to */
                unsigned char *mem_len)  /* Memory used            */
{
    int i = 0;

    /* N is always between 1 and 8 bits [4.1.1.]
     */
    const unsigned char limits[] = {0, 1, 3, 7, 15, 31, 63, 127, 255};
    const unsigned char limit    = limits[N];

    /* An integer is represented in two parts:
     * - A prefix that fills the current octet (N bits length)
     * - An optional list of octets
     */
    if (value < limit) {
        mem[i] = (mem[i] & ~limit) | (unsigned char)value;
        *mem_len = ++i;
        return;
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
}
