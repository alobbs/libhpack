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
 * @file      header_field.h
 * @brief     Function prototipes for HPACK's Header Field implementation.
 *
 * This contains the prototypes for the Header Field implementation used in HPACK
 * parser,
 *
.* Also contains any macros, constants, or global variables you will need.
 *
 * @author    Alvaro Lopez Ortega <alvaro@gnu.org>
 * @author    Gorka Eguileor <gorka@eguileor.com>
 * @date      April, 2014
 */

#ifndef LIBHPACK_HEADER_FIELD_H
#define LIBHPACK_HEADER_FIELD_H

#if !defined(HPACK_H_INSIDE) && !defined (HPACK_COMPILATION)
# error "Only <libhpack/libhpack.h> can be included directly."
#endif

#include <libchula/libchula.h>

/**
 * Defines the overhead in bytes for each field as declared in HPACK's
 * specifications.
 *
 * This is used to calculate the used space of a field in the Header Table.
 *
 */
#define HPACK_HEADER_ENTRY_OVERHEAD      32


/**
 * Different type of fields we may encounter depending on how they are generated.
 * This may be different for the name and the value.
 */
typedef enum {
    is_indexed_ht     = 0, /**< Obtained from an indexed entry in the Header Table */
    is_indexed_static = 1, /**< Obtained from an indexed entry in the Static Table */
    is_new            = 2, /**< Obtained from an literal representation */
    is_new_huffman    = 3  /**< Obtained from an literal representation with huffman encoding */
} hpack_header_field_type_t;


/**
 * String representation of the different header field types we may encounter as
 * enumerated in [hpack_header_field_type_t](@ref hpack_header_field_type_t).
 */
chula_buffer_t hpack_header_field_field_type_repr[4];


/**
 * Describes what kind of Header Field we got from the encoder.
 */
typedef enum {
    rep_empty         = 0, /**< The header field is empty. */
    rep_user_supplied = 1, /**< We generated this field ourselves, this didn't come from the encoder. */
    rep_indexed       = 2, /**< This came encoded as an Indexed Header Field Representation. */
    rep_inc_indexed   = 3, /**< This came encoded as a Header Field with Incremental Indexing. */
    rep_wo_indexing   = 4, /**< This came encoded as a Header Field without Indexing. */
    rep_never_indx    = 5  /**< This came encoded as a Header Field never Indexed. */
} hpack_header_field_representation_t;


/**
 * String representation of the different header field representations we may
 * encounter as enumerated in [hpack_header_field_representation_t](@ref hpack_header_field_representation_t).
 */
chula_buffer_t hpack_header_field_representations_repr[6];

/**
 * Flags representing how this field was encoded.
 */
typedef struct {
    hpack_header_field_type_t           name  : 2; /**< How name was encoded. */
    hpack_header_field_type_t           value : 2; /**< How the value was encoded. */
    hpack_header_field_representation_t rep   : 3; /**< What kind of representation was used to encoded this field. */
} hpack_header_field_flags_t;


/**
 * Header Field contents once a header field has been decoded.
 */
typedef struct {
    hpack_header_field_flags_t flags; /**< How was this field encoded. */
    chula_buffer_t             name;  /**< Name part of the header. */
    chula_buffer_t             value; /**< Value part of the header. */
} hpack_header_field_t;

#define HPACK_HEADER_FIELD(f) ((hpack_header_field_t *)(f))


ret_t hpack_header_field_new      (hpack_header_field_t **header);
ret_t hpack_header_field_free     (hpack_header_field_t  *header);
ret_t hpack_header_field_init     (hpack_header_field_t  *header);
ret_t hpack_header_field_clean    (hpack_header_field_t  *header);
ret_t hpack_header_field_mrproper (hpack_header_field_t  *header);
bool  hpack_header_field_is_empty (hpack_header_field_t  *header);
ret_t hpack_header_field_copy     (hpack_header_field_t  *header, hpack_header_field_t *tocopy);
ret_t hpack_header_field_repr     (hpack_header_field_t  *header, chula_buffer_t       *output);
ret_t hpack_header_field_get_size (hpack_header_field_t  *header, uint64_t             *size);

#endif /* LIBHPACK_HEADER_FIELD_H */
