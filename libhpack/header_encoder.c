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

#include "header_encoder.h"
#include "header_field.h"
#include "integer.h"
#include "huffman.h"

ret_t
hpack_header_encoder_init (hpack_header_encoder_t *enc)
{
    ret_t ret;

    ret = hpack_header_store_init (&enc->store);
    if (ret != ret_ok) return ret;

    return ret_ok;
}

ret_t
hpack_header_encoder_mrproper (hpack_header_encoder_t *enc)
{
    ret_t ret;

    ret = hpack_header_store_mrproper (&enc->store);
    if (ret != ret_ok) return ret;

    return ret_ok;
}

ret_t
hpack_header_encoder_add (hpack_header_encoder_t *enc,
                          chula_buffer_t         *name,
                          chula_buffer_t         *value)
{
    ret_t                 ret;
    hpack_header_field_t *field = HPACK_NEW_OBJ(header_field);

    /* Build new header field */
    chula_buffer_add_buffer_RET (&field->name, name);
    chula_buffer_add_buffer_RET (&field->value, value);

    /* Add the field */
    ret = hpack_header_encoder_add_field (enc, field);
    if (unlikely (ret != ret_ok)) return ret;

    return ret_ok;
}


ret_t
hpack_header_encoder_add_field (hpack_header_encoder_t *enc,
                                hpack_header_field_t   *field)
{
    return hpack_header_store_emit (&enc->store, field);
}

static ret_t
add_string (chula_buffer_t *output,
            chula_buffer_t *in,
            bool            huffman)
{
    ret_t   ret;
    uint8_t mem_len = 16;
    uint8_t mem[16] = {[0 ... 15] =  0};

    /* TODO */
    UNUSED(huffman);

    /* Length */
    ret = hpack_integer_encode (7, in->len, mem, &mem_len);
    if (unlikely (ret != ret_ok)) return ret;

    /* Content */
    chula_buffer_add_RET (output, (const char *)mem, mem_len);
    chula_buffer_add_buffer_RET (output, in);

    return ret_ok;
}

static ret_t
field_encode_plain (hpack_header_encoder_t *enc,
                    hpack_header_field_t   *field,
                    chula_buffer_t         *output)
{
    ret_t   ret;

    UNUSED (enc);

    /* Literal Header Field without Indexing - New Name
     *
     *     0   1   2   3   4   5   6   7
     *   +---+---+---+---+---+---+---+---+
     *   | 0 | 0 | 0 | 0 |       0       |
     *   +---+---+-----------------------+
     *   | H |     Name Length (7+)      |
     *   +---+---------------------------+
     *   |  Name String (Length octets)  |
     *   +---+---------------------------+
     *   | H |     Value Length (7+)     |
     *   +---+---------------------------+
     *   | Value String (Length octets)  |
     *   +-------------------------------+
     */
    chula_buffer_add_char_RET (output, '\0');

    /* Name */
    ret = add_string (output, &field->name, false);
    if (unlikely (ret != ret_ok)) return ret;

    ret = add_string (output, &field->value, false);
    if (unlikely (ret != ret_ok)) return ret;

    return ret_ok;
}

ret_t
hpack_header_encoder_render (hpack_header_encoder_t *enc,
                             chula_buffer_t         *output)
{
    ret_t                       ret;
    hpack_header_store_entry_t *i;

    hpack_header_store_foreach (i, &enc->store) {
        hpack_header_field_t *field = HPACK_HEADER_FIELD(i);

        ret = field_encode_plain (enc, field, output);
        if (unlikely (ret != ret_ok)) return ret;
    }

    return ret_ok;
}
