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
#include "huffman_tables.h"

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
hpack_huffman_encode (chula_buffer_t *in,
                      chula_buffer_t *out)
{
    ret_t                       ret;
    const hpack_huffman_code_t *code;
    unsigned char               remaining_bits = 8;

    for (uint32_t n=0; n < in->len; n++) {
        /* Memory management */
        if (out->size - out->len < 8) {
            ret = chula_buffer_ensure_addlen (out, in->len);
            if (unlikely(ret != ret_ok)) return ret;
        }

        /* Encode */
        code = &hpack_huffman[(unsigned char)in->buf[n]];
        if (remaining_bits == 8) {
            out->buf[out->len] = 0;
        }
        remaining_bits = encode_symbol (code, remaining_bits, out);
    }

    if (remaining_bits < 8) {
        code = &hpack_huffman[256];
        out->buf[out->len] |= code->code >> (code->bits - remaining_bits);
        out->len++;
    }

    ret = chula_buffer_ensure_size (out, out->len+1);
    if (unlikely(ret != ret_ok)) return ret;

    out->buf[out->len] = '\0';
    return ret_ok;
}


/**  Huffman decoding
 *
 * Decodes a Huffman encoded buffer into another one. The memory
 * allocation of the output buffer is handled automatically by the
 * function, there is no need to pre-allocate memory for the
 * uncompressed output.
 *
 * @param      in           Buffer with the information to uncompress
 * @param[out] out          Buffer to uncompress the information to
 * @param[out] context      Context object for the Huffman decoding
 * @retval     ret_ok       Buffer successfully uncompressed
 */
ret_t
hpack_huffman_decode (chula_buffer_t                 *in,
                      chula_buffer_t                 *out,
                      hpack_huffman_decode_context_t *context)
{
    ret_t                         ret;
    uint8_t                       c;
    uint32_t                      n;
    const hpack_huffman_decode_t *t;

    for (n=0; n < in->len; n++) {
        /* Memory management */
        if (out->size - out->len < 8) {
            ret = chula_buffer_ensure_addlen (out, in->len * 2);
            if (unlikely(ret != ret_ok))
                return ret;
        }

        /* Decoding */
        c = ((uint8_t)in->buf[n]) >> 4;
        for (int halfbyte = 0; halfbyte < 2; ++halfbyte) {
            t = &decode_table[context->state][c];
            if(t->state == -1) {
                return ret_error;
            }
            if(t->flags & HPACK_HUFFMAN_SYMBOL) {
                ret = chula_buffer_add_char (out, t->sym);
                if (unlikely (ret != ret_ok)) return ret;
            }
            context->state  = t->state;
            context->accept = (t->flags & HPACK_HUFFMAN_ACCEPTED) != 0;
            c = in->buf[n] & 0xf;
        }
    }

    return ret_ok;
}
