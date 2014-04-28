/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/* All files in libhpack are Copyright (C) 2014 Alvaro Lopez Ortega.
 *
 *   Authors:
 *     * Alvaro Lopez Ortega <alvaro@gnu.org>
 *     * Gorka Eguileor <gorka@eguileor.com>
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

/**
 * @file      header_parser.h
 * @brief     Header Parser implementation for HPACK.
 *
 * This contains the prototypes for the Header Field implementation used in HPACK
 * parser/decoder as specified in [HPACK - Header Compression for HTTP/2](http://http2.github.io/http2-spec/compression.html).
 *
.* Also contains any macros, constants, or global variables you will need.
 *
 * @author    Alvaro Lopez Ortega <alvaro@gnu.org>
 * @author    Gorka Eguileor <gorka@eguileor.com>
 * @date      April, 2014
 */

#ifndef LIBHPACK_HEADER_PARSER_H
#define LIBHPACK_HEADER_PARSER_H

#if !defined(HPACK_H_INSIDE) && !defined (HPACK_COMPILATION)
# error "Only <libhpack/libhpack.h> can be included directly."
#endif

#include <libchula/libchula.h>
#include <libhpack/header_field.h>
#include <libhpack/header_table.h>
#include <libhpack/header_store.h>
#include <libhpack/bitmap_set.h>


/**
 * Decoder context table structure.
 */
typedef struct {
    hpack_header_table_t table;            /**< Header Table. */
    hpack_set_t          reference_set;    /**< Reference Set for differential encoding. */
    hpack_set_t          ref_not_emitted;  /**< References from the reference set we haven't emmited yet. */
    hpack_set_iterator_t iter_not_emitted; /**< Iterator to emit remaining headers from the reference set. */
    bool                 finished;         /**< Marks when we will receive no more data to decode. */
} hpack_header_parser_context_t;

/**
 * Header Parser Structure.
 */
typedef struct {
    hpack_header_parser_context_t context;  /**< Decoding context. */
    hpack_header_store_t          *store;   /**< Storage to return decoded fields. */
} hpack_header_parser_t;


ret_t hpack_header_parser_new       (hpack_header_parser_t **parser);
ret_t hpack_header_parser_init      (hpack_header_parser_t  *parser);
ret_t hpack_header_parser_mrproper  (hpack_header_parser_t **parser);

ret_t hpack_header_parser_reg_store (hpack_header_parser_t  *parser,
                                     hpack_header_store_t   *store);

ret_t hpack_header_parser_field     (hpack_header_parser_t  *parser,
                                     chula_buffer_t         *buf,
                                     unsigned int            offset,
                                     hpack_header_field_t   *field,
                                     unsigned int           *consumed);

ret_t hpack_header_parser_all       (hpack_header_parser_t  *parser,
                                     chula_buffer_t         *buf,
                                     unsigned int            offset,
                                     unsigned int           *consumed);

#endif /* LIBHPACK_HEADER_PARSER_H */
