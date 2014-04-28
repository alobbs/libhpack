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

#ifndef LIBHPACK_HUFFMAN_H
#define LIBHPACK_HUFFMAN_H

#if !defined(HPACK_H_INSIDE) && !defined (HPACK_COMPILATION)
# error "Only <libhpack/libhpack.h> can be included directly."
#endif

#include <libchula/libchula.h>

/** Entry of the Huffman code table
 */
typedef struct {
    char     bits;
    uint32_t code;
} hpack_huffman_code_t;

/** Huffman decoding
 */
typedef enum {
    HPACK_HUFFMAN_ACCEPTED = 1,
    HPACK_HUFFMAN_SYMBOL   = 1 << 1,
} hpack_huffman_decode_flag;

typedef struct {
    uint8_t state;
    uint8_t accept;
} hpack_huffman_decode_context_t;

#define HUFFMAN_DEC_CTX_INIT {.state = 0, .accept = 1}

typedef struct {
    int16_t state;
    uint8_t flags;
    uint8_t sym;
} hpack_huffman_decode_t;

typedef hpack_huffman_decode_t hpack_huffman_decode_table_t[256][16];


ret_t hpack_huffman_encode (chula_buffer_t *in,
                            chula_buffer_t *out);

ret_t hpack_huffman_decode (chula_buffer_t                 *in,
                            chula_buffer_t                 *out,
                            hpack_huffman_decode_context_t *context);

#endif /* LIBHPACK_HUFFMAN_H */
