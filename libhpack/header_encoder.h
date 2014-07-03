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

#ifndef LIBHPACK_HEADER_ENCODER_H
#define LIBHPACK_HEADER_ENCODER_H

#if !defined(HPACK_H_INSIDE) && !defined (HPACK_COMPILATION)
# error "Only <libhpack/libhpack.h> can be included directly."
#endif

#include <libchula/libchula.h>
#include <libhpack/header_field.h>
#include <libhpack/header_table.h>
#include <libhpack/header_store.h>
#include <libhpack/bitmap_set.h>

/**
 * Header Parser Structure.
 */
typedef struct {
    hpack_header_store_t store;
    chula_buffer_t       tmp;
} hpack_header_encoder_t;

ret_t hpack_header_encoder_init     (hpack_header_encoder_t *enc);
ret_t hpack_header_encoder_mrproper (hpack_header_encoder_t *enc);

ret_t hpack_header_encoder_add       (hpack_header_encoder_t *enc,
                                      chula_buffer_t         *name,
                                      chula_buffer_t         *value);
ret_t hpack_header_encoder_add_field (hpack_header_encoder_t *enc,
                                      hpack_header_field_t   *field);
ret_t hpack_header_encoder_render    (hpack_header_encoder_t *enc,
                                      chula_buffer_t         *output);

#endif /* LIBHPACK_HEADER_ENCODER_H */
