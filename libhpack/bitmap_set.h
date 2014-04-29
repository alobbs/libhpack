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
 * @file      bitmap_set.h
 * @brief     Function prototipes for HPACK's Set implementation.
 *
 * This contains the prototypes for the bitmap Set implementation used in HPACK,
 * useless for any other use.
 *
.* Also contains any macros, constants, or global variables you will need.
 *
 * @author    Alvaro Lopez Ortega <alvaro@gnu.org>
 * @author    Gorka Eguileor <gorka@eguileor.com>
 * @date      April, 2014
 */

#ifndef LIBHPACK_BITMAP_SET_H
#define LIBHPACK_BITMAP_SET_H

#if !defined(HPACK_H_INSIDE) && !defined (HPACK_COMPILATION)
# error "Only <libhpack/libhpack.h> can be included directly."
#endif

#include <limits.h>
#include <stdint.h>

#include <libchula/libchula.h>
#include <libhpack/macros.h>


/**
 * @typedef hpack_set_entry_t
 *
 * Type definition for each of the entries in the bitmap set array.
 * Each bit corresponds to 1 index.
 * We align this with the word size of the machine, so it will be 32 or 64
 * indexes per word.
 */

/**
 * @def BITMAP_SET_SHIFT
 *
 * Defines how many bits are used for the relative position in each entry of the
 * bitmap set array and by exclusion how many are for array entry;
 *
 * The value depends on whether we have a 32 or 64 bits machine.
 *
 * E.g. For a 32 bits machine, and index 45, the first 5 bits (01101b) are the
 * position (13) and the remaining bits (1b) are the entry in the array (second
 *  entry).
 */
#if (64 == __WORDSIZE)
  typedef uint64_t hpack_set_entry_t;
# define BITMAP_SET_SHIFT 6
#else
  typedef uint32_t hpack_set_entry_t;
# define BITMAP_SET_SHIFT 5
#endif


/**
 * How many bytes are needed to hold all indexes.
 */
#define HPACK_SET_BYTES_NEEDED    (int)((HPACK_MAX_HEADER_TABLE_ENTRIES + 7) >> 3)


/**
 * Number of entries needed to bitmap all possible indexes from HPACK header
 * table.
 */
#define HPACK_SET_NUM_ENTRIES     (int)((HPACK_SET_BYTES_NEEDED + sizeof(hpack_set_entry_t) -1) / (sizeof(hpack_set_entry_t)))


/**
 * Representation of a set for the indexes of an HPACK context.
 * Each array entry holds 32 or 64 indexes, depending on the machine,
 */
typedef hpack_set_entry_t hpack_set_t[HPACK_SET_NUM_ENTRIES];


/** Iterator for a bitmap set
 *
 * The iterator type allows getting the elements of a bitmap set one at a time to
 * avoid having to check for the existence of each possible index or needing an
 * array to get all items.
 *
 * @see  hpack_set_iter_init()
 * @see  hpack_set_iter_set()
 * @see  hpack_set_iter_next()
 * @see  hpack_set_iter_reset()
 *
 */
typedef struct {
    hpack_set_entry_t *set;   /**< Pointer to the first entry of the Set we want to iterate */
    uint8_t            bit;   /**< Which bit we are currently checking */
    uint8_t            entry; /**< Which one of the entries are we checking */
} hpack_set_iterator_t;


void    hpack_set_init          (hpack_set_t b_set,  bool fill_set);
ret_t   hpack_set_add           (hpack_set_t b_set,  unsigned int idx);
ret_t   hpack_set_remove        (hpack_set_t b_set,  unsigned int idx);
bool    hpack_set_exists        (hpack_set_t b_set,  unsigned int idx);
void    hpack_set_union         (hpack_set_t b_set1, hpack_set_t b_set2);
void    hpack_set_relative_comp (hpack_set_t b_set1, hpack_set_t b_set2);
void    hpack_set_intersection  (hpack_set_t bset_1, hpack_set_t b_set2);
bool    hpack_set_equals        (hpack_set_t b_set1, hpack_set_t b_set2);
void    hpack_set_set           (hpack_set_t b_set1, hpack_set_t b_set2);
void    hpack_set_complement    (hpack_set_t b_set);
void    hpack_set_clear         (hpack_set_t b_set);
void    hpack_set_fill          (hpack_set_t b_set);
bool    hpack_set_is_empty      (hpack_set_t b_set);
bool    hpack_set_is_full       (hpack_set_t b_set);

ret_t   hpack_set_iter_init     (hpack_set_iterator_t *iter, hpack_set_t b_set);
int16_t hpack_set_iter_next     (hpack_set_iterator_t *iter);
void    hpack_set_iter_reset    (hpack_set_iterator_t *iter);

/** @brief Changes the working set for an iterator. */
#define  hpack_set_iter_set      hpack_set_iter_init

#endif /* LIBHPACK_BITMAP_SET_H */

/** @endcond */
