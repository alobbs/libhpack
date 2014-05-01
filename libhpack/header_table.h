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
 * @cond      INTERNAL
 * @file      header_table.h
 * @brief     Header Table implementation for HPACK.
 *
 * This contains the prototypes for the Header Field implementation used in HPACK
 * parser,
 *
.* Also contains any macros, constants, or global variables you will need.
 *
 * Since the indexes used by HPACK compression and the indexes we use internally
 * are different we have a macro that switches between them both called
 * [INDEX_SWITCH_HT_HPACK](@ref INDEX_SWITCH_HT_HPACK) (it works both ways) and
 * also functions to work with the sets since they are always used as internal
 * indexes.
 *
 * @author    Alvaro Lopez Ortega <alvaro@gnu.org>
 * @author    Gorka Eguileor <gorka@eguileor.com>
 * @date      April, 2014
 */

#ifndef LIBHPACK_HEADER_TABLE_H
#define LIBHPACK_HEADER_TABLE_H

#if !defined(HPACK_H_INSIDE) && !defined (HPACK_COMPILATION)
# error "Only <libhpack/libhpack.h> can be included directly."
#endif

#include <libchula/libchula.h>
#include <libhpack/macros.h>
#include <libhpack/header_field.h>
#include <libhpack/bitmap_set.h>


/**
 * Structure for the Info stored in the header's data array before the field data
 * (name, value).
 */
typedef struct {
    uint16_t                   name_length;   /**< Octects used for the name. */
    uint16_t                   value_length;  /**< Octects used for the value. */
    hpack_header_field_flags_t flags;         /**< Flags for the header. */
} hpack_header_table_field_info_t;


/**
 * Structure for the Circular buffer used to store the position (offset) of each
 * header entry in the header's data array.
 */
typedef struct {
    uint16_t buffer[HPACK_MAX_HEADER_TABLE_ENTRIES];  /**< Array of offsets for the Header Entries. */
    uint16_t head;                                    /**< Head of the Circular Buffer. */
    uint16_t tail;                                    /**< Tail of the Circular Buffer. */
} hpack_headers_offs_cb_t;


/**
 * Structure for the Circular Buffer used to store the data of the Header Fields.
 */
typedef struct {
    char     buffer[HPACK_CB_HEADER_DATA_SIZE];  /**< Header Fields information. */
    uint16_t head;                               /**< Head of the Circular Buffer. */
    uint16_t tail;                               /**< Tail of the Circular Buffer. */
} hpack_headers_data_cb_t;


/**
 * Structure for the whole Header Table.
 */
typedef struct {
    hpack_headers_offs_cb_t headers_offsets;  /**< Positions of the headers in the headers_data field. */
    hpack_headers_data_cb_t headers_data;     /**< Header Field data. */
    uint16_t                num_headers;      /**< How many headers we currently have in the table. */
    uint16_t                used_data;        /**< How many octects we have used from the Header Table (this
                                             *   is regarding the Maximum Table Size and not the actual
                                             *   bytes used). */
    uint16_t                max_data;         /**< Maximum Table Size as specified in HPACK */
} hpack_header_table_t;


/**
 * The number of entries in the static table.
 */
#define STATIC_ENTRIES 61u

ret_t hpack_header_table_new         (hpack_header_table_t **table);
ret_t hpack_header_table_free        (hpack_header_table_t  *table);

ret_t hpack_header_table_init        (hpack_header_table_t  *table);
ret_t hpack_header_table_mrproper    (hpack_header_table_t  *table);
ret_t hpack_header_table_clear       (hpack_header_table_t  *table);
ret_t hpack_header_table_set_max     (hpack_header_table_t  *table, uint16_t max, hpack_set_t evicted_set);
ret_t hpack_header_table_add         (hpack_header_table_t  *table, hpack_header_field_t *field, hpack_set_t evicted_set);
ret_t hpack_header_table_get         (hpack_header_table_t  *table, uint16_t n, bool only_name, hpack_header_field_t *f, bool *is_static);
ret_t hpack_header_table_get_set_idx (hpack_header_table_t  *table, uint16_t n, bool only_name, hpack_header_field_t *f);
void  hpack_header_table_repr        (hpack_header_table_t  *table, chula_buffer_t *output);

/** Get the current size of the Header Table. */
#define  hpack_header_table_get_size(table_ptr) ((table_ptr)->used_data)

/** Check if the Header Table is empty. */
#define  hpack_header_table_is_empty(table_ptr) (0 == ((table_ptr)->used_data))

/** Convert an HPACK index to an index from our internal representation and the other way array */
#define INDEX_SWITCH_HT_HPACK(T,I) ((uint16_t) (((T)->headers_offsets.tail - (I) + HPACK_MAX_HEADER_TABLE_ENTRIES) & HPACK_CB_HEADER_OFFSETS_MASK))



/** Set initializer */
#define hpack_header_table_set_init                  hpack_set_init

/** Adds an HPACK index to the set. */
#define hpack_header_table_set_add(T,S,I)            hpack_set_add(S, INDEX_SWITCH_HT_HPACK(T,I))

/** Removes an HPACK index from the set. */
#define hpack_header_table_set_remove(T,S,I)         hpack_set_remove(S, INDEX_SWITCH_HT_HPACK(T,I))

/** Checks if an HPACK index is in a set. */
#define hpack_header_table_set_exists(T,S,I)         hpack_set_exists(S, INDEX_SWITCH_HT_HPACK(T,I))

/** Performs the union of the sets. */
#define hpack_header_table_set_union(S1,S2)          hpack_set_union(S1,S2)

/** Performs the relative complement of the sets. */
#define hpack_header_table_set_relative_comp(S1,S2)  hpack_set_relative_comp(S1,S2)

/** Performs the intersection of the sets. */
#define hpack_header_table_set_intersection(S1,S2)   hpack_set_intersection(S1,S2)

/** Compares two sets to see if they have the same indexes. */
#define hpack_header_table_set_equals(S1,S2)         hpack_set_equals(S1,S2)

/** Clones a set. */
#define hpack_header_table_set_set(S1,S2)            hpack_set_set(S1,S2)

/** Turns a set into its complement. */
#define hpack_header_table_set_complement(S)         hpack_set_complement(S)

/** Clears a set from all its indexes. */
#define hpack_header_table_set_clear(S)              hpack_set_clear(S)

/** Fills a set with all possible indexes. */
#define hpack_header_table_set_fill(S)               hpack_set_fill(S)

/** Checks if a set is empty. */
#define hpack_header_table_set_is_empty(S)           hpack_set_is_empty(S)

/** Checks if a set if complete. */
#define hpack_header_table_set_is_full(S)            hpack_set_is_full(S)


/** Initializes an iterator for a set. */
#define hpack_header_table_iter_init(I,S)            hpack_set_iter_init(I,S)

/** Changes the working set for an iterator. */
#define hpack_header_table_iter_set(I,S)             hpack_set_iter_set(I,S)

int     hpack_header_table_iter_next                 (hpack_header_table_t *table, hpack_set_iterator_t *iterA);

/** Restarts the iterator without changing the refererd set. */
#define hpack_header_table_iter_reset(I)             hpack_set_iter_reset(I)

#endif /* LIBHPACK_HEADER_TABLE_H */

/** @endcond */
