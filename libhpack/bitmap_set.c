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
 * @file      bitmap_set.c
 * @brief     A bitmap Set implementation for HPACK.
 *
 * Given the specifics of the HPACK sets they have been implemented as as bitmap
 * set, which would be crazy were it any other domain not as restricted as the
 * one from HPACK Header Table.
 *
 * The maximum size of the HPACK Header Table si 4096, considering that the
 * overhead for each entry is 32 bytes, as per HPACK specifications, and if we
 * add 1 additional byte for the smallest possible header field (1 char for the
 * name and no value) we end up with a maximum number of 124.12 indexes.
 *
 * Which means we will have indexes ranging from 0 to 123, and with 128 bits (2
 * words on a 64 bits machine) we can map all possible indexes.
 *
 * Also all operations on a bitmap set are very fast and inexpensive, the only
 * exception is the retrieval of the sets elements, but with the use of an
 * iterator it isn't that expensive either.
 *
 * @author    Alvaro Lopez Ortega <alvaro@gnu.org>
 * @author    Gorka Eguileor <gorka@eguileor.com>
 * @date      April, 2014
 */

#include <libchula/libchula.h>
#include "bitmap_set.h"

/**
 * Mask to get the position in the entry.
 */
#define BITMAP_SET_MASK           ((8 * sizeof(hpack_set_entry_t))-1)

/**
 * How many bits are in each entry, useful for the iterator.
 */
#define HPACK_SET_BITS_IN_ENTRY   (sizeof(hpack_set_entry_t) * 8)


/** Set initializer
 *
 * Initializes a Set to an empty or full et. If the set already has content this
 * will be lost.
 *
 * @param[out] b_set    Set to initialize
 * @param[in]  fill_set @parblock
 *                      - true : The set will be initialized to a set with all the
 *                               indexes in it.
 *                      - false: The set will be initialized to an empty set.
 *                      @endparblock
 *
 */
void
hpack_set_init (hpack_set_t b_set,
                bool        fill_set)
{
    if (fill_set)
        hpack_set_fill(b_set);
    else
        hpack_set_clear(b_set);
}


/** Adds an index to the set
 *
 * Ensures that an index is included in a Set by adding it in if it isn't already
 * there.
 *
 * @pre @a b_set must have already been [initialized](@ref hpack_set_init) or
 *      [cloned](@ref hpack_set_set) from another Set.
 *
 * @param[in,out] b_set  Set to add the index to.
 * @param[in]     idx    Index to include. Must be a value in the range of
 *                       [0 - [HPACK_SET_NUM_ENTRIES-1](@ref HPACK_SET_NUM_ENTRIES)]
 *
 * @return Result of the operation.
 * @retval ret_error The index is out of range.
 * @retval ret_ok    The index was added successfully.
 */
ret_t
hpack_set_add (hpack_set_t  b_set,
               unsigned int idx)
{
    if (unlikely(HPACK_MAX_HEADER_TABLE_ENTRIES <= idx))
        return ret_error;

    b_set[idx >> BITMAP_SET_SHIFT] |= (hpack_set_entry_t) 1 << (idx & BITMAP_SET_MASK);

    return ret_ok;
}


/** Performs the union of the sets
 *
 * Performs the union of the supplied Sets and returns the value in the first
 * Set.
 * This could be considered as adding the indexes from the second Set to the
 * first one.
 *
 * @param[in,out] b_set1  First set for the operation and the Set were the result
 *                        of the operation will be returned in.
 * @param[in]     b_set2  Second set for the operation.
 *
 */
void
hpack_set_union (hpack_set_t b_set1,
                 hpack_set_t b_set2)
{
    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i)
        b_set1[i] |= b_set2[i];
}


/** Removes an index from the set
 *
 * Ensures that an index is not in a Set by removing it from it if it's in it.
 *
 * @param[in,out] b_set  Set to remove the index from.
 * @param[in]     idx    Index to include. Must be a value in the range of
 *                       [0 - [HPACK_SET_NUM_ENTRIES-1](@ref HPACK_SET_NUM_ENTRIES)
 *
 * @return Result of the operation.
 * @retval ret_error The index is out of range.
 * @retval ret_ok    The index was removed successfully.
 */
ret_t
hpack_set_remove (hpack_set_t  b_set,
                  unsigned int idx)
{
    if (unlikely(HPACK_MAX_HEADER_TABLE_ENTRIES <= idx))
        return ret_error;

    b_set[idx >> BITMAP_SET_SHIFT] &= ~((hpack_set_entry_t) 1 << (idx & BITMAP_SET_MASK));

    return ret_ok;
}


/** Performs the relative complement of the sets
 *
 * Performs the relative complement of the supplied Sets and returns the value in
 * the first Set.
 * Doing this removes from the first Set all indexes that are in common between
 * the two Sets.
 *
 * @param[in,out] b_set1  First set for the operation and the Set were the result
 *                        of the operation will be returned in.
 * @param[in]     b_set2  Second set for the operation.
 *
 */
void
hpack_set_relative_comp (hpack_set_t b_set1,
                         hpack_set_t b_set2)
{
    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i)
        b_set1[i] &= ~b_set2[i];
}


/** Performs the intersection of the sets
 *
 * Performs the intersection of the supplied Sets and returns the value in the
 * first Set.
 * Doing this leaves in the first Set only those indexes that are in common
 * between the two Sets.
 *
 * @param[in,out] b_set1  First set for the operation and the Set were the result
 *                        of the operation will be returned in.
 * @param[in]     b_set2  Second set for the operation.
 *
 */
void
hpack_set_intersection (hpack_set_t b_set1,
                        hpack_set_t b_set2)
{
    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i)
        b_set1[i] &= b_set2[i];
}


/** Turns a set into its complement
 *
 * Performs the complement of a Set and returns the result of the operation in
 * itself.
 *
 * @param[in,out] b_set  Set used for the operation and also the Set were the
 *                       result of the operation will be returned in.
 *
 */
void
hpack_set_complement (hpack_set_t b_set)
{
    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i)
        b_set[i] = ~b_set[i];
}


/** Clears a set from all its indexes
 *
 * Removes all indexes from a Set and returns an empty set in its place.
 *
 * @param[out] b_set  Set to be emptied.
 *
 */
void
hpack_set_clear (hpack_set_t b_set)
{
    /* This is actually faster than `memset (b_set, 0, sizeof(hpack_set_t));` */
    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i)
        b_set[i] = 0;
}


/** Fills a set with all possible indexes
 *
 * Fills  the set will all the indexes from the domain.
 * This means that you will have indexes ranging from 0 to
 * [HPACK_SET_NUM_ENTRIES-1](@ref HPACK_SET_NUM_ENTRIES) in the set.
 *
 * @param[out] b_set  Set to be filled.
 *
 */
void
hpack_set_fill (hpack_set_t b_set)
{
    /* This is faster than `memset (b_set, 0xFF, sizeof(hpack_set_t));` */
    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i)
        b_set[i] = (hpack_set_entry_t) ~0;
}


/** Checks if an index is in a set
 *
 * @param[in] b_set  Set to look into.
 * @param[in] idx    Index to look for.
 *
 * @return Whether it was found or not.
 * @retval true  The item was found in the Set:
 * @retval false The item was not in the set or the index was not in the range of
 *               possible indexes [0-[HPACK_SET_NUM_ENTRIES-1](@ref HPACK_SET_NUM_ENTRIES)].
 *
 */
bool
hpack_set_exists (hpack_set_t  b_set,
                  unsigned int idx)
{
    if (unlikely(HPACK_MAX_HEADER_TABLE_ENTRIES <= idx))
        return false;

    return (b_set[idx >> BITMAP_SET_SHIFT] & ((hpack_set_entry_t) 1 << (idx & BITMAP_SET_MASK)));
}


/** Checks if a set is empty
 *
 * @param[in] b_set  Set to check.
 *
 * @return Whether the set was empty or not.
 * @retval true  The set was empty.
 * @retval false The set had at least one index.
 *
 */
bool
hpack_set_is_empty (hpack_set_t b_set)
{
    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i) {
        if (b_set[i])
            return false;
    }

    return true;
}


/** Checks if a set if complete
 *
 * @param[in] b_set  Set to check.
 *
 * @return Whether the set was full or not.
 * @retval true  The set had all possible indexes.
 * @retval false The set was missing at least one index.
 *
 */
bool
hpack_set_is_full (hpack_set_t b_set)
{
    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i) {
        if (b_set[i] != ~(hpack_set_entry_t) 0)
            return false;
    }

    return true;
}


/** Compares two sets to see if they have the same indexes
 *
 * @param[in] b_set1  Set to compare.
 * @param[in] b_set2  Set to compare to.
 *
 * @return Whether the Sets had the same indexes.
 * @retval true  The set had the same indexes.
 * @retval false There was at least one missing or additional index.
 *
 */
bool
hpack_set_equals (hpack_set_t b_set1,
                  hpack_set_t b_set2)
{
    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i) {
        if (b_set1[i] ^ b_set2[i])
            return false;
    }

    return true;
}


/** Clones a set
 *
 * @param[out] b_set1  Set to clone to.
 * @param[in]  b_set2  Set to clone from.
 *
 */
void
hpack_set_set (hpack_set_t b_set1,
               hpack_set_t b_set2)
{
    /* This is faster than `memcpy (b_set1, b_set2, sizeof(hpack_set_t));`*/
    for (int i=0; i < HPACK_SET_NUM_ENTRIES; ++i)
        b_set1[i] = b_set2[i];
}


/** Initializes an iterator for a set
 *
 * Given a Set it initializes an interator to sequentially retrieve all items
 * from the set.
 *
 * You can initialize an iterator even before the Set has been initialized, it
 * doesn't matter, as long as it has been initialized before you do the actual
 * iteration.
 *
 * @code
 * hpack_set_t          b_set;
 * hpack_set_iterator_t iter;
 * int16_t              idx;
 *
 * hpack_set_init (b_set, true);
 * hpack_set_iter_init (&iter, b_set);
 * do {
 *     idx = hpack_set_iter_next (&iter);
 *     if (-1 != idx) {
 *       printf ("Index %d found\n", idx);
 *     }
 * } while (-1 != idx);
 * @endcode
 *
 * @param[out] iter   Iterator to initialize.
 * @param[in]  b_set  Set we want to iterate.
 *
 * @see hpack_set_iter_reset()
 * @see hpack_set_iter_next()
 */
ret_t
hpack_set_iter_init (hpack_set_iterator_t *iter,
                     hpack_set_t           b_set)
{
    if (unlikely (NULL == iter))
        return ret_error;

    iter->set = b_set;
    iter->bit = 0;
    iter->entry = 0;

    return ret_ok;
}


/** Restarts the iterator without changing the refererd set
 *
 * @pre @a iter must have been previously initialized with [hpack_set_iter_init](@ref hpack_set_iter_init)
 *
 * @param[in,out] iter   Iterator to reset.
 *
 * @see hpack_set_iter_init()
 * @see hpack_set_iter_next()
 */
void
hpack_set_iter_reset (hpack_set_iterator_t *iter)
{
    iter->bit = 0;
    iter->entry = 0;
}


/** Returns the next existing element in the set
 *
 * Gets the next index from the Set continuing where it left of in the previous
 * call.
 *
 * Items will be returned in strictly increasing order.
 *
 * The function is oblivious to changes to the Set. So you will only catch
 * changes to the Set if they are from an index with a higher value than the last
 * returned value.
 *
 * If you modify the Set or if you finish retrieving all items from a Set you
 * must [reinitialize](@ref hpack_set_iter_init) or [reset](@ref hpack_set_iter_reset)
 * the iterator before you can retrieve any more indexes with the iterator.
 *
 * @code
 * hpack_set_t          b_set;
 * hpack_set_iterator_t iter;
 * int16_t              idx;
 *
 * hpack_set_init (b_set, true);
 * hpack_set_iter_init (&iter, b_set);
 * do {
 *     idx = hpack_set_iter_next (&iter);
 *     if (-1 != idx) {
 *       printf ("Index %d found\n", idx);
 *     }
 * } while (-1 != idx);
 * @endcode
 *
 * @pre @a iter must have been previously initialized with [hpack_set_iter_init](@ref hpack_set_iter_init)
 *
 * @param[in,out] iter   Iterator to use.
 *
 * @return The next index in the set.
 * @retval   -1 No more indexes to return.
 * @retval !=-1 The index found in the set.
 *
 * @see hpack_set_iter_reset()
 * @see hpack_set_iter_next()
 */
int16_t
hpack_set_iter_next (hpack_set_iterator_t *iter)
{
    int16_t result = -1;

    if (unlikely (iter == NULL))
        return -1;

    while ((HPACK_SET_NUM_ENTRIES > iter->entry) && (-1 == result)) {

        /* We only check the bits if we know there's something in the remaining bits of this entry */
        if (iter->set[iter->entry] & ~((hpack_set_entry_t) (1 << iter->bit)-1)) {

            /* Check remaining bits from this entry */
            while ((iter->bit < HPACK_SET_BITS_IN_ENTRY) && (-1 == result)) {
                /* If we have found our next index we store it but we still have to advance the position for the next call. */
                if (iter->set[iter->entry] & ((hpack_set_entry_t) 1 << iter->bit))
                    result = iter->bit + (iter->entry * HPACK_SET_BITS_IN_ENTRY);

                ++iter->bit;
            }

            /* Finished with all bits from this entry, advance to the next. */
            if (HPACK_SET_BITS_IN_ENTRY == iter->bit) {
                ++iter->entry;
                iter->bit = 0;
            }

        /* If there are no indexes in this entry we advance to the next entry. */
        } else {
            ++iter->entry;
            iter->bit = 0;
        }
    }

    return (result);
}

/** @endcond */
