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
 * @file      header_field.c
 * @brief     HPACK's Header Field Implementation
 *
 * This Header Field Implementation is used by the Header Parser (decoder) to
 * return fields from a Header Block once they've been decoded and to add and get
 * fields to/from the Header Table.
 *
 * It's also used by the Static Header implementation in the Header Table, but
 * not for the actual Header Table.
 *
 * @author    Alvaro Lopez Ortega <alvaro@gnu.org>
 * @author    Gorka Eguileor <gorka@eguileor.com>
 * @date      April, 2014
 */

#include <libchula/libchula.h>
#include "macros.h"
#include "header_field.h"

/**
 * String representation of the different header field representations we may
 * encounter as enumerated in hpack_header_field_representation_t.
 */
chula_buffer_t hpack_header_field_representations_repr[6] = {
    CHULA_BUF_INIT_FAKE("Empty"),
    CHULA_BUF_INIT_FAKE("User supplied"),
    CHULA_BUF_INIT_FAKE("Indexed"),
    CHULA_BUF_INIT_FAKE("Incremental indexing"),
    CHULA_BUF_INIT_FAKE("Without indexing"),
    CHULA_BUF_INIT_FAKE("Never indexed")
};

/**
 * String representation of the different header field types we may encounter as
 * enumerated in [hpack_header_field_type_t](@ref hpack_header_field_type_t).
 */
chula_buffer_t hpack_header_field_field_type_repr[4] = {
    CHULA_BUF_INIT_FAKE("Indexed Header Table"),
    CHULA_BUF_INIT_FAKE("Indexed Static Table"),
    CHULA_BUF_INIT_FAKE("New literal"),
    CHULA_BUF_INIT_FAKE("New Huffman literal")
};


/** Header Field initializer
 *
 * Initializes a Header Field to an empty header: No name or value and flags set
 * to empty representation and indexed from Header Table both name and value.
 *
 * @param[out] header Header Field to initialize
 *
 * @return Result of the initialization
 * @retval ret_ok    Currently the only value this function will return
 *
 */
ret_t
hpack_header_field_init (hpack_header_field_t *header)
{
    ret_t ret;

    header->flags.rep = rep_empty;
    header->flags.name = is_indexed_ht;
    header->flags.value = is_indexed_ht;

    ret  = chula_buffer_init (&header->name);
    ret += chula_buffer_init (&header->value);

    return ret;
}


/** Empties a Header Field
 *
 * Cleans a Header Field leaving it empty: No name or value and flag set to empty
 * representation.
 *
 * Does not free any memory used by the name and value buffers.
 *
 * @pre @a header needs to have at the very least been [initialized](@ref hpack_header_field_init)
 *
 * @param[out] header Header Field to empty.
 *
 * @return     Result of the operation.
 * @retval     ret_ok    Currently the only possible result of the operation.
 *
 */
ret_t
hpack_header_field_clean (hpack_header_field_t *header)
{
    header->flags.rep = rep_empty;

    chula_buffer_clean (&header->name);
    chula_buffer_clean (&header->value);

    /* No need to set position 0 to '\0' because chula's clean already does */

    return ret_ok;
}

HPACK_ADD_FUNC_NEW(header_field);
HPACK_ADD_FUNC_FREE(header_field);


/** Clean up all memory used by the Header Field
 *
 * Frees all memory used by the Header Field, which currently is the memory used
 * by the name and value buffers.
 *
 * @pre @a header needs to have at the very least been [initialized](@ref hpack_header_field_init)
 *
 * @param[out] header    Header Field's memory to free.
 *
 * @return     Result of the operation.
 * @retval     ret_ok    Currently the only possible result of the operation.
 *
 */
ret_t
hpack_header_field_mrproper (hpack_header_field_t *header)
{
    ret_t ret;

    ret  = chula_buffer_mrproper (&header->name);
    ret += chula_buffer_mrproper (&header->value);

    return ret;
}


/** Clones a Header Field
 *
 * Copies the contents of one Header Field to another. If the destination buffer
 * already has data in the name or value fields the new content will be simply
 * attached to previous data.
 *
 * @pre @a header and @a tocopy need to have at the very least been [initialized](@ref hpack_header_field_init)
 *
 * @param[in,out] header Header Field destination.
 * @param[in]     tocopy Header Field source.
 *
 * @return Result of the operation.
 * @retval ret_ok     The operation was completed.
 * @retval ret_nomem  There was no memory to complete the operation.
 *
 */
ret_t
hpack_header_field_copy (hpack_header_field_t *header,
                         hpack_header_field_t *tocopy)
{
    ret_t re;

    header->flags = tocopy->flags;

    re =  chula_buffer_add_buffer (&header->name, &tocopy->name);
    re += chula_buffer_add_buffer (&header->value, &tocopy->value);

    return re;
}


/** Check if the Header Field has any contents
 *
 * Checks whether the Header Field is empty or not.
 * The response is based on the name and value fields and not on the flags.
 *
 * @pre @a header needs to have at the very least been [initialized](@ref hpack_header_field_init)
 *
 * @param[in] header Header Field to check.
 * @return           Whether the Header Field is empty or not.
 *
 */
bool
hpack_header_field_is_empty (hpack_header_field_t *header)
{
    if (! chula_buffer_is_empty (&header->name))
        return false;

    return (chula_buffer_is_empty (&header->value));
}


/** Creates a string representation of a Header Field
 *
 * The format used to represent the field is:
 * hpack_header_field@@@e POINTER @e NAME: @e VALUE [ @e REPRESENTATION | Name: @e TYPE_OF_NAME | Value: @e TYPE_OF_VALUE ]
 *
 * @pre @a header needs to have at the very least been [initialized](@ref hpack_header_field_init)
 * @pre @a output chula buffer needs to have been [initialized](@ref chula_buffer_init)
 *
 * @param[in]   header    Header Field to represent.
 * @param[out]  output    Where to output the representation.
 *
 * @return If the operation was successful.
 * @retval ret_ok     The operation was completed successfully.
 * @retval ret_nomem  There was no memory to complete the operation.
 *
 */
ret_t
hpack_header_field_repr (hpack_header_field_t *header,
                         chula_buffer_t       *output)
{
    ret_t ret;

    ret = chula_buffer_add_va (output, "hpack_header_field@%x - ", POINTER_TO_INT(header));

    /* Represent the name-value pair. */
    ret += chula_buffer_add_buffer (output, &header->name);
    ret += chula_buffer_add_str    (output, ": ");
    ret += chula_buffer_add_buffer (output, &header->value);
    ret += chula_buffer_add_str    (output, " [");

    /* Represent the flags. */
    ret += chula_buffer_add_buffer (output,
                                    &hpack_header_field_representations_repr[header->flags.rep]);
    ret += chula_buffer_add_str    (output, " | Name: ");
    ret += chula_buffer_add_buffer (output,
                                    &hpack_header_field_field_type_repr[header->flags.name]);
    ret += chula_buffer_add_str    (output, " | Value: ");
    ret += chula_buffer_add_buffer (output,
                                    &hpack_header_field_field_type_repr[header->flags.value]);
    ret += chula_buffer_add_str    (output, "]");
    ret += chula_buffer_add_str    (output, CRLF);

    return ret;
}


/** Returns the used size of the field in the Header Table
 *
 * Returns the number of bytes used by this field in the Header Table as
 * described in the HPACK specification.
 *
 * HPACK defines the size of a Header Field entry in the Header Table as the sum
 * of its name's length in octets, of its value's length in octets and of the
 * entry structure overhead (32 octets).
 *
 * @pre @a header needs to have at the very least been [initialized](@ref hpack_header_field_init)
 *
 * @param[in]   header    Header Field to get size from.
 * @param[out]  size      Integer where to store the size.
 *
 * @return If the operation was successful.
 * @retval ret_ok     The operation was completed successfully.
 */
ret_t
hpack_header_field_get_size (hpack_header_field_t *header,
                             uint64_t             *size)
{
    if (chula_buffer_is_empty(&header->name) &&
        chula_buffer_is_empty(&header->value))
    {
        *size = 0;
        return ret_ok;
    }

    *size = header->name.len + header->value.len + HPACK_HEADER_ENTRY_OVERHEAD;
    return ret_ok;
}
