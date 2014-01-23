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

#include "huffman.h"

static unsigned char
encode_symbol (const hpack_huffman_code_t *code,
               unsigned char               remaining_bits,
               chula_buffer_t             *out)
{
    unsigned char nbits = code->bits;

    while (true) {
        if (remaining_bits > nbits) {
            out->buf[out->len] |= code->code << (remaining_bits - nbits);
            remaining_bits -= nbits;
            break;
        }

        out->buf[out->len] |= code->code >> (nbits - remaining_bits);
        out->len++;

        nbits -= remaining_bits;
        remaining_bits = 8;
        if (nbits == 0)
            break;

        out->buf[out->len] = 0;
    }

    return remaining_bits;
}

/**  Huffman encoding
 *
 * Encodes a buffer into another one using the provided Huffman
 * table. The memory allocation of the output buffer is handled
 * automatically by the function, there is no need to pre-allocate
 * memory for the compressed buffer.
 *
 * @param      in      Buffer with the information to compress
 * @param[out] out     Buffer to compress the information to
 * @param      table   Huffman table to use for the compression
 * @retval     ret_ok  Buffer successfully compressed
 */
ret_t
hpack_huffman_encode (chula_buffer_t             *in,
                      chula_buffer_t             *out,
                      const hpack_huffman_code_t *table)
{
    ret_t                       ret;
    const hpack_huffman_code_t *code;
    unsigned char               remaining_bits = 8;

    for (int n=0; n < in->len; n++) {
        /* Memory management */
        if (out->size - out->len < 8) {
            ret = chula_buffer_ensure_addlen (out, in->len);
            if (unlikely(ret != ret_ok))
                return ret;
        }

        /* Encode */
        code = &table[(unsigned char)in->buf[n]];
        if (remaining_bits == 8) {
            out->buf[out->len] = 0;
        }
        remaining_bits = encode_symbol (code, remaining_bits, out);
    }

    if (remaining_bits < 8) {
        code = &table[256];
        out->buf[out->len] |= code->code >> (code->bits - remaining_bits);
        out->len++;
    }

    chula_buffer_ensure_size (out, out->len+1);
    out->buf[out->len] = '\0';
    return ret_ok;
}

static inline int
check_last_byte (chula_buffer_t *src,
                 cuint_t         n,
                 cuint_t         bit_offset)
{
    uint8_t mask = (1 << (8 - bit_offset)) - 1;

    return ((bit_offset > 0) &&
            (src->len == n+1) &&
            (((uint8_t)(src->buf[n]) & mask) == mask));
}

static inline uint8_t
get_prefix_byte (chula_buffer_t *in, int offset, unsigned char bit_offset)
{
    uint8_t c = in->buf[offset];

    if (bit_offset == 0) {
        return c;
    }

    if (in->len <= offset) {
        return c << bit_offset;
    }

    return (c << bit_offset) | (((uint8_t)in->buf[offset+1]) >> (8 - bit_offset));
}


static int
decode (chula_buffer_t                     *in,
        unsigned int                        offset,
        unsigned char                       bit_offset,
        const hpack_huffman_code_t         *table_codes,
        const hpack_huffman_decode_table_t  table_decode)
{
    uint8_t prefix;
    int16_t re       = 0;
    int     len_orig = in->len - offset;

    if (offset >= in->len)
        return -1;

    while (true) {
        prefix = get_prefix_byte(in, offset, bit_offset);
        re = table_decode[re][prefix];
        if (re >= 0) {
            break;
        }

        re = -re;
        offset++;
        if (offset >= in->len) {
            return -1;
        }
    }

    if (bit_offset + table_codes[re].bits > len_orig * 8) {
        return -1;
    }

    return re;
}

/**  Huffman decoding
 *
 * Decodes a buffer into another one using the provided Huffman code
 * and decoding tables. The memory allocation of the output buffer is
 * handled automatically by the function, there is no need to
 * pre-allocate memory for the uncompressed output.
 *
 * @param      in           Buffer with the information to uncompress
 * @param[out] out          Buffer to uncompress the information to
 * @param      table_codes  Huffman code table
 * @param      table_decode Huffman decoding table
 * @retval     ret_ok       Buffer successfully uncompressed
 */
ret_t
hpack_huffman_decode (chula_buffer_t                     *in,
                      chula_buffer_t                     *out,
                      const hpack_huffman_code_t         *table_codes,
                      const hpack_huffman_decode_table_t  table_decode)
{
    ret_t        ret;
    unsigned int n;
    unsigned int re;
    unsigned int bit_offset = 0;

    for (n=0; n < in->len;) {
        /* Memory management */
        if (out->size - out->len < 8) {
            ret = chula_buffer_ensure_addlen (out, in->len * 2);
            if (unlikely(ret != ret_ok))
                return ret;
        }

        /* Decode */
        re = decode (in, n, bit_offset, table_codes, table_decode);
        if (re == -1) {
            if (check_last_byte (in, n, bit_offset)) {
                break;
            }
            return ret_error;
        }
        else if (re == 256) {
            return ret_error;
        }

        /* Current char */
        out->buf[out->len] = re;
        out->len++;

        /* Offset for next iteration */
        bit_offset += table_codes[re].bits;
        n += bit_offset / 8;
        bit_offset &= 0x7;
    }

    chula_buffer_ensure_size (out, out->len+1);
    out->buf[out->len] = '\0';
    return ret_ok;
}
