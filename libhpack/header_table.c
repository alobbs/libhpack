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
 * @file      header_table.c
 * @brief     HPACK's Header Table Implementation
 *
 * HPACK Header Table Implementation specifics:
 * The Header table is actually stored in 2 circular buffers, one that contains
 * the actual header data ([@e hpack_header_table_t.headers_data](@ref hpack_header_table_t))
 * and another that contains the position of the headers in that circular buffer
 * ([@e hpack_header_table_t.headers_offsets](@ref hpack_header_table_t)).
 *
 * In the data circular buffer all headers are stored consecutively one after
 * another and for each header we store first [some info](@ref hpack_header_table_field_info_t)
 * with the length of the name, the length of the value and the flags of the
 * field and then we store the characters from the name and the characters from
 * the value. Both strings, name and value, are not null-terminated, since we
 * already know their length and they are stored in a circular buffer it would do
 * us no good storing the '\0'.
 *
 * In the Header Table all indexes are treated internally as absolute positions
 * in the @c headers_offsets array, so they start with 0 and end with
 * @c HPACK_MAX_HEADER_TABLE_ENTRIES - 1, and they grow up as a queue instead of
 * as a FIFO (like HPACK does). Macro [INDEX_SWITCH_HT_HPACK](@ref INDEX_SWITCH_HT_HPACK)
 * changes from HPACK to absolute index and viceversa.
 *
 * So when you add a new Header Field the other Header Fields keep the same index
 * they had before (this doesn't happen with HPACK indexing method).
 *
 * This is very useful, because every Set we use here will also use that kind of
 * index, and if we were to use HPACK's indexing system we would either have to
 * modify every set in existance whenever we add a new Header Field or we would
 * have to use a more complex Set implementation which would probably also be
 * slower and/or require more memory.
 *
 * All functions intended to use by the Parser take as input HPACK Indexes and
 * not the kind of indexes we use internally.
 *
 * Since all sets returned here do not contain HPACK indexes we have created
 * macros/functions to manipulate the sets in HPACK format.
 *
 * To illustrate this here's an example of a Header Table with the following
 * content:
 *
 * Index #1   :method: GET
 * Index #2   :scheme: http
 * Index #3   :path  : /
 @verbatim
                              HEADERS OFFSETS
   +----------+----------+----------+----------+----------+----------+
   |    0     |    1     |    2     |    3     |    4     |    5     | <- Absolute Index
   +----------+----------+----------+----------+----------+----------+
   |          |          | Header 3 | Header 2 | Header 1 |          | <- HPACK Index
   |   ???    |   ???    |   003    |   00E    |   01E    |   ???    | <- Array content s
   +----------+----------+----------+----------+----------+----------+
                          ^ Head                           ^ Tail


                                           HEADERS DATA
   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
   | 000 | 001 | 002 | 003 | 004 | 005 | 006 | 007 | 008 | 009 | 00A | 00B | 00C | 00D | 00E | 00F | <- Array position
   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
   |  ?  |  ?  |  ?  | Name Len. | Val. Len. |Flags|  :  |  p  |  a  |  t  |  h  |  /  | Name Len. | <- Array contents
   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
                        ^ Head

   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
   | 010 | 011 | 012 | 013 | 014 | 015 | 016 | 017 | 018 | 019 | 01A | 01B | 01C | 01D | 01E | 01F |
   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
   | Val. Len. |Flags|  :  |  s  |  c  |  h  |  e  |  m  |  e  |  h  |  t  |  t  |  p  | Name Len. |
   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+


   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
   | 020 | 021 | 022 | 023 | 024 | 025 | 026 | 027 | 028 | 029 | 02A | 02B | 02C | 02D | 02E | 02F |
   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
   | Val. Len. |Flags|  :  |  m  |  e  |  t  |  h  |  o  |  d  |  G  |  E  |  T  |  ?  |  ?  |  ?  |
   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
                                                                                    ^ Tail

 @endverbatim
 *
 * @author    Alvaro Lopez Ortega <alvaro@gnu.org>
 * @author    Gorka Eguileor <gorka@eguileor.com>
 * @date      April, 2014
 */

#include <libhpack/header_table.h>
#include <libhpack/macros.h>
#include <libhpack/header_table.h>
#include <libhpack/header_field.h>


/** Static Table Entry without a value
 */
#define HDR_NIL(s)                            \
    {   .name  = CHULA_BUF_INIT_FAKE(s),      \
        .value = CHULA_BUF_INIT,              \
        .flags = {.rep = rep_indexed, .name=is_indexed_static, .value=is_indexed_static} , \
    }

/** Static Table Entry with a value
 */
#define HDR_VAL(n,v)                         \
    {   .name  = CHULA_BUF_INIT_FAKE(n),     \
        .value = CHULA_BUF_INIT_FAKE(v),     \
        .flags = {.rep = rep_indexed, .name=is_indexed_static, .value=is_indexed_static} , \
    }

/** Static Table (B Appendix)
 *
 * The static table consists of an unchangeable ordered list of (name,
 * value) pairs. The first entry in the table is always represented by
 * the index len(header table)+1, and the last entry in the table is
 * represented by the index len(header table)+len(static table).
 */
static hpack_header_field_t static_table[STATIC_ENTRIES] = {
    /* 01 */ HDR_NIL(":authority"),
    /* 02 */ HDR_VAL(":method", "GET"),
    /* 03 */ HDR_VAL(":method", "POST"),
    /* 04 */ HDR_VAL(":path", "/"),
    /* 05 */ HDR_VAL(":path", "/index.html"),
    /* 06 */ HDR_VAL(":scheme", "http"),
    /* 07 */ HDR_VAL(":scheme", "https"),
    /* 08 */ HDR_VAL(":status", "200"),
    /* 09 */ HDR_VAL(":status", "204"),
    /* 0A */ HDR_VAL(":status", "206"),
    /* 0B */ HDR_VAL(":status", "304"),
    /* 0C */ HDR_VAL(":status", "400"),
    /* 0D */ HDR_VAL(":status", "404"),
    /* 0E */ HDR_VAL(":status", "500"),
    /* 0F */ HDR_NIL("accept-charset"),
    /* 10 */ HDR_NIL("accept-encoding"),
    /* 11 */ HDR_NIL("accept-language"),
    /* 12 */ HDR_NIL("accept-ranges"),
    /* 13 */ HDR_NIL("accept"),
    /* 14 */ HDR_NIL("access-control-allow-origin"),
    /* 15 */ HDR_NIL("age"),
    /* 16 */ HDR_NIL("allow"),
    /* 17 */ HDR_NIL("authorization"),
    /* 18 */ HDR_NIL("cache-control"),
    /* 19 */ HDR_NIL("content-disposition"),
    /* 1A */ HDR_NIL("content-encoding"),
    /* 1B */ HDR_NIL("content-language"),
    /* 1C */ HDR_NIL("content-length"),
    /* 1D */ HDR_NIL("content-location"),
    /* 1E */ HDR_NIL("content-range"),
    /* 1F */ HDR_NIL("content-type"),
    /* 20 */ HDR_NIL("cookie"),
    /* 21 */ HDR_NIL("date"),
    /* 22 */ HDR_NIL("etag"),
    /* 23 */ HDR_NIL("expect"),
    /* 24 */ HDR_NIL("expires"),
    /* 25 */ HDR_NIL("from"),
    /* 26 */ HDR_NIL("host"),
    /* 27 */ HDR_NIL("if-match"),
    /* 28 */ HDR_NIL("if-modified-since"),
    /* 29 */ HDR_NIL("if-none-match"),
    /* 2A */ HDR_NIL("if-range"),
    /* 2B */ HDR_NIL("if-unmodified-since"),
    /* 2C */ HDR_NIL("last-modified"),
    /* 2D */ HDR_NIL("link"),
    /* 2E */ HDR_NIL("location"),
    /* 2F */ HDR_NIL("max-forwards"),
    /* 30 */ HDR_NIL("proxy-authenticate"),
    /* 31 */ HDR_NIL("proxy-authorization"),
    /* 32 */ HDR_NIL("range"),
    /* 33 */ HDR_NIL("referer"),
    /* 34 */ HDR_NIL("refresh"),
    /* 35 */ HDR_NIL("retry-after"),
    /* 36 */ HDR_NIL("server"),
    /* 37 */ HDR_NIL("set-cookie"),
    /* 38 */ HDR_NIL("strict-transport-security"),
    /* 39 */ HDR_NIL("transfer-encoding"),
    /* 3A */ HDR_NIL("user-agent"),
    /* 3B */ HDR_NIL("vary"),
    /* 3C */ HDR_NIL("via"),
    /* 3D */ HDR_NIL("www-authenticate")
};



/*
 * CIRCULAR BUFFER HELPER FUNCTIONS/MACROS
 */

/**
 * How many bytes are used in the Header Data Circular Buffer.
 */
#define header_data_used(H) (((H)->tail - (H)->head + 1 + HPACK_CB_HEADER_DATA_MASK) & HPACK_CB_HEADER_DATA_MASK)

/**
 * How many bytes are free in the Header Data Circular Buffer.
 */
#define header_data_free(H) (((H)->head - (H)->tail + HPACK_CB_HEADER_DATA_MASK) & HPACK_CB_HEADER_DATA_MASK)

/**
 * How many bytes are used in the Offsets Circular Buffer.
 */
#define header_offsets_free(H) (((H)->head - (H)->tail + HPACK_CB_HEADER_OFFSETS_MASK) & HPACK_CB_HEADER_OFFSETS_MASK)

/**
 * How many bytes are free in the Offsets Circular Buffer.
 */
#define header_offs_is_full(O) ((((O)->head - 1) & HPACK_CB_HEADER_OFFSETS_MASK) == (O)->tail)

/**
 * Move the position of an offset of a Circular Buffer.
 */
#define header_cb_move(CB,INC,MAX,MASK) (CB = (CB+(INC)) & MASK)

/**
 * Check if an entry (HPACK index) in the header table has data.
 */
#define hpack_header_offset_has_data(T,N) ((T)->num_headers >= INDEX_SWITCH_HT_HPACK(T,N))
//#define hpack_header_offset_has_data(CB,N,MAX) ((N >= 0) && (N < MAX) && (CB.tail >= CB.head? (CB.head <= N) && (CB.tail > N) : (CB.head <= N) (CB.tail > N)))


/*
 * HEADER TABLE INTERNAL FUNCTIONS
 */
static ret_t header_offs_add       (hpack_headers_offs_cb_t *offsets, uint16_t offset);
static ret_t header_data_get       (hpack_headers_data_cb_t *h_data, uint16_t offset, char *dst, unsigned int num_bytes);
static ret_t header_data_get_chula (hpack_headers_data_cb_t *h_data, uint16_t offset, chula_buffer_t *dst, unsigned int num_bytes);
static ret_t header_data_add       (hpack_headers_data_cb_t *h_data, char *data, unsigned int data_size);


/**
 * Evict 1 Header Field from the Header Table.
 * Since it's a FIFO you'll alway remove the oldest element which is the one at the Head.
 *
 * @param[in,out] table       Table from where to evict the Header Field.
 * @param[out]    ret_evicted Index of evicted Header Field
 *
 * @return Result of the operation.
 * @retval re_error  There is no room for this new offset.
 * @retval re_ok     The element has been added.
 */
static ret_t
header_table_evict (hpack_header_table_t *table,
                    int                  *ret_evicted)
{
    int                             evicted;
    hpack_header_table_field_info_t info;

    if (unlikely (table == NULL))
        return ret_error;

    /*
     * Remove oldest element (FIFO): We remove the first element in both circular
     *  buffers because they both belong to the same header field.
     */

    if (unlikely (0 == table->num_headers))
        return ret_ok;

    evicted = table->headers_offsets.head;

    /* Both elements must belong to the same header field. */
    if (unlikely (table->headers_data.head != table->headers_offsets.buffer[evicted]))
        return ret_error;

    /* Advance offsets head. */
    header_cb_move (table->headers_offsets.head, 1, HPACK_MAX_HEADER_TABLE_ENTRIES, HPACK_CB_HEADER_OFFSETS_MASK);

    /* Get the header info from the table. */
    header_data_get (&table->headers_data,
                            table->headers_offsets.buffer[evicted],
                            (char *)&info, sizeof(info));

    /* Now that we know how many bytes this field uses we do the actual advance of the data head. */
    header_cb_move (table->headers_data.head,
                          sizeof(info) + info.name_length + info.value_length,
                          HPACK_CB_HEADER_DATA_SIZE,
                          HPACK_CB_HEADER_DATA_MASK);

    --table->num_headers;
    table->used_data -= info.name_length + info.value_length + HPACK_HEADER_ENTRY_OVERHEAD;

    /* Return */
    *ret_evicted = evicted;
    return ret_ok;
}


/** Add an offset to the offset Circular Buffer
 *
 * Add an element to the Header Offsets Circular Buffer.
 *
 * This element is the offset in the Header Data Circular Buffer to the
 * corresponding Header Field.
 *
 * I.e. the head in the offset "points" to the oldest Header Field in the Header
 * Data Circular Buffer.
 *
 * @param[in,out] offsets  Circular buffer of offsets.
 * @param[in]     offset   Offset to add.
 *
 * @return Result of the operation.
 * @retval re_error  There is no room for this new offset.
 * @retval re_ok     The element has been added.
 */
static ret_t
header_offs_add (hpack_headers_offs_cb_t *offsets,
                 uint16_t                 offset)
{
    if (unlikely(offsets == NULL))
        return ret_error;

    if (unlikely(header_offsets_free(offsets) < 1))
        return ret_error;

    offsets->buffer[offsets->tail] = offset;

    header_cb_move (offsets->tail, 1, HPACK_MAX_HEADER_TABLE_ENTRIES, HPACK_CB_HEADER_OFFSETS_MASK);

    return ret_ok;
}


/** Get data from the Header Table data Circular Buffer
 *
 * Get requested number of bytes from the Header Table Data Circular Buffer
 * starting at an offset.
 *
 * The function only checks that you don't try to get more data than what's
 * available in the table, so you could request to get uninitialized or out
 * of use data if you wanted to.
 *
 * @param[in]  h_data     Circular Buffer with the Header Data.
 * @param[in]  offset     Offset inside the Header Data to start getting data.
 * @param[out] dst        Where we want to store the retrieved data.
 * @param[in]  num_bytes  How many bytes we want to get.
 *
 * @return  Result of the operation.
 * @retval ret_error  Tried to read more bytes than the total size of the table.
 * @retval ret_ok     Data in dst is relevant.
 */
static ret_t
header_data_get (hpack_headers_data_cb_t *h_data,
                 uint16_t                 offset,
                 char                    *dst,
                 unsigned int             num_bytes)
{
    unsigned int to_end;

    if (unlikely ((h_data == NULL) || (dst == NULL)))
        return ret_error;

    /* It is unlikely we'll try to read more data than what's available. */
    if (unlikely(num_bytes > HPACK_CB_HEADER_DATA_SIZE))
        return ret_error;

    to_end = HPACK_CB_HEADER_DATA_SIZE - offset - h_data->head;

    memcpy (dst, h_data->buffer + offset, MIN(to_end,num_bytes));

    /* If we couldn't get all data in one go because it continues at the begining. */
    if (to_end < num_bytes)
        memcpy (dst + to_end, h_data->buffer, num_bytes - to_end);

    return ret_ok;
}


/** Get data from the Header Table data Circular Buffer into a Chula Buffer
 *
 * Add requested number of bytes from the Header Table Data Circular Buffer
 * starting at an offset to a Chula buffer.
 *
 * The function only checks that you don't try to get more data than what's
 * available in the table, so you could request to get uninitialized or out
 * of use data if you wanted to.
 *
 * @param[in]  h_data     Circular Buffer with the Header Data.
 * @param[in]  offset     Offset inside the Header Data to start getting data.
 * @param[out] dst        Chula Buffer where we want to store the retrieved data.
 * @param[in]  num_bytes  How many bytes we want to get.
 *
 * @return  Result of the operation.
 * @retval ret_error  Tried to read more bytes than the total size of the table.
 * @retval ret_ok     Data in dst is relevant.
 */
static ret_t
header_data_get_chula (hpack_headers_data_cb_t *h_data,
                       uint16_t                 offset,
                       chula_buffer_t          *dst,
                       unsigned int             num_bytes)
{
    ret_t        ret;
    unsigned int to_end;

    if (unlikely ((h_data == NULL) || (dst == NULL)))
        return ret_error;

    /* It is unlikely we'll try to read more data than what's available. */
    if (unlikely(num_bytes > HPACK_CB_HEADER_DATA_SIZE))
        return ret_error;

    to_end = HPACK_CB_HEADER_DATA_SIZE - offset - h_data->head;

    ret = chula_buffer_add (dst, h_data->buffer + offset, MIN(to_end,num_bytes));

    /* If we couldn't get all data in one go because it continues at the beginning. */
    if (to_end < num_bytes)
        ret += chula_buffer_add (dst, h_data->buffer, num_bytes - to_end);

    return ret;
}

/** Add data to the Header Table data Circular Buffer
 *
 * Add data to the Header Table Data Circular Buffer.
 *
 * Checks if there's enough room to add the data.
 *
 * @param[in,out] h_data     Header Data Circular Buffer.
 * @param[in]     data       Data to add to the Circular Buffer.
 * @param[in]     data_size  How many bytes to add.
 *
 * @return Result of the operation.
 * @retval ret_error  There is not enough room to add this data.
 * @retval ret_ok     Data has been added.
 */
static ret_t
header_data_add (hpack_headers_data_cb_t *h_data,
                 char                    *data,
                 unsigned int             data_size)
{
    unsigned int to_end;

    if (unlikely ((h_data == NULL) || (data == NULL)))
        return ret_error;

    if (unlikely(header_data_free(h_data) < data_size))
        return ret_error;

    to_end = HPACK_CB_HEADER_DATA_SIZE - h_data->tail;

    memcpy (h_data->buffer + h_data->tail, data, MIN(to_end,data_size));

    /* If we couldn't do it in one shot because it continues at the beginning. */
    if (to_end < data_size)
        memcpy (h_data->buffer, data + to_end, data_size - to_end);

    /* Advance the tail of the Circular Buffer */
    header_cb_move (h_data->tail, data_size, HPACK_CB_HEADER_DATA_SIZE, HPACK_CB_HEADER_DATA_MASK);

    return ret_ok;
}



/*
 * HEADER TABLE EXTERNALLY CALLABLE FUNCTIONS
 */


/** Reserves memory for a new Header Table and initializes the Header Table
 *
 * Creates a new Header Table by reserving the memory needed for it and
 * initializes it.
 *
 * @param[out] table  Reference to the pointer of the new table.
 *
 * @return Result of the operation.
 * @retval ret_nomem  There's no memory for the new Header Table.
 * @retval ret_ok     Created successfully.
 */
HPACK_ADD_FUNC_NEW(header_table);
HPACK_ADD_FUNC_FREE(header_table);


/** Clean up all memory used by the Header Table
 *
 * Clean up all memory used by a Header Table previously created with the
 * [new](@ref hpack_header_table_new) function.
 *
 * The table pointer is set to NULL.
 *
 * @pre @a table was created with [hpack_header_table_new](@ref hpack_header_table_new)
 *
 * @param[in,out] table  Header Table to free.
 *
 * @return Result of the operation.
 * @retval ret_ok  Only possible value returned.
 */
ret_t
hpack_header_table_mrproper (hpack_header_table_t *table)
{
    ret_t ret;

    ret = hpack_header_table_clear (table);
    if (unlikely (ret != ret_ok)) return ret;

    return ret_ok;
}

/** Initialize the Header Table
 *
 * Initializer function for a Header Table that basically clears everything.
 * It can be used with Header Tables created with the [new](@ref hpack_header_table_new)
 * function and those created directly in the stack i.e. `hpack_header_table_t ht_variable;`
 *
 * @param[out] table  Header Table to initialize.
 *
 * @return Result of the operation.
 * @retval ret_ok  Currently this is the only possible result.
 */
ret_t
hpack_header_table_init (hpack_header_table_t *table)
{
    ret_t ret;

    ret = hpack_header_table_clear (table);
    if (unlikely (ret != ret_ok)) return ret;

    return ret_ok;
}


/** Empty the Header Table
 *
 * Clears all the header field entries, but does not change the Maximum header
 * size field.
 *
 * @param[out] table  Header Table to empty.
 *
 * @return Result of the operation.
 * @retval ret_ok  Currently this is the only possible result.
 */
ret_t
hpack_header_table_clear (hpack_header_table_t *table)
{
    table->num_headers          = 0;
    table->used_data            = 0;
    table->headers_offsets.head = 0;
    table->headers_offsets.tail = 0;
    table->headers_data.head    = 0;
    table->headers_data.tail    = 0;
    table->max_data             = SETTINGS_HEADER_TABLE_SIZE;

    return ret_ok;
}

/** Add a new Header Field
 *
 * Add a Header Field to the Header Table as specified in [HPACK specifications](http://http2.github.io/http2-spec/compression.html#entry.addition)
 * Header Fields evicted when adding this new Header Field to the Header Table
 * are returned as a set with all Indexes in the internal format, not HPACK index
 * format.
 *
 * If it returns a full set as the evicted set then it means that the entry could
 * not be added and the whole table has been cleared. THIS IS NOT AN ERROR! As
 * explained in HPACK specifications.
 *
 * @param[in,out] table        Header Table where we want the field added.
 * @param[in]     field        Header Field to add.
 * @param[out]    evicted_set  Set with all the evicted indexes.
 *
 * @return The result of the operation.
 * @retval ret_ok     Currently this is the only possible result.
 * @retval ret_error  There's been an error, which is impossible.
 */
ret_t
hpack_header_table_add (hpack_header_table_t *table,
                        hpack_header_field_t *field,
                        hpack_set_t           evicted_set)
{
    ret_t                           ret;
    int                             evicted;
    uint64_t                        field_size;
    uint16_t                        tail_headers;
    uint16_t                        tail_offsets;
    hpack_header_table_field_info_t info;


    /* Initially evicted set is empty. */
    hpack_set_init (evicted_set, false);

    /* If the data doesn't fit we empty the whoe table in one shot instead of go
     * one by one emptying it, and return a set that indicates that everything
     * has been evicted.
     */
    hpack_header_field_get_size (field, &field_size);
    if (unlikely(field_size > table->max_data)) {
        hpack_header_table_clear (table);
        hpack_set_fill (evicted_set);

        return ret_ok;
    }

    /* Here we know it fits, so we create enought room for it. */
    while ((int) field_size > (int) table->max_data - table->used_data) {
        ret = header_table_evict (table, &evicted);
        if (unlikely (ret != ret_ok)) return ret;

        /* We know there are elements, otherwise this new element would have fitted. */
        hpack_set_add (evicted_set, (unsigned int)evicted);
    }

    /* We know beforehand there's going to be enough room in the offsets Circ. Buf. */
    if (unlikely (header_offs_is_full (&table->headers_offsets)))
        return ret_error;

    /* Now we add the Header_Field to the Header Table (offset and the data). */

    /* We can never fail because we made sure we had space to store everything,
     * but just in case we store current positions for a possible rollback.
     */
    tail_headers = table->headers_data.tail;
    tail_offsets = table->headers_offsets.tail;

    /* The new Header Field data data offset is the Header Data tail. */
    ret = header_offs_add (&table->headers_offsets, tail_headers);

    /* We prepare the internal Header Field info that's stored in the Header Data CB. */
    info.name_length = field->name.len;
    info.value_length = field->value.len;
    info.flags = field->flags;

    /* Store the info, followed by the name and the value. No '\0' is stored. */
    ret += header_data_add (&table->headers_data, (char *)&info, sizeof(info));
    ret += header_data_add (&table->headers_data, (char *)field->name.buf , info.name_length);

    if (0 < info.value_length)
        ret += header_data_add (&table->headers_data, (char *)field->value.buf, info.value_length);

    if (unlikely (ret != ret_ok))
        return ret_error;

    /* If there was an error, which is impossible because we checked that we
     * could add all the data, undo add.
     */
    if (unlikely(ret_ok != ret)) {
        table->headers_offsets.tail = tail_offsets;
        table->headers_data.tail = tail_headers;
        return ret_error;
    }

    table->used_data += field_size;
    ++table->num_headers;

    return ret_ok;
}


/** Set a new maximum size
 *
 * Set a new Maximum Table Size and process evictions as specified in [HPACK specifications](http://http2.github.io/http2-spec/compression.html#entry.eviction)
 * Header Fields evicted when setting the new Maximum size of the Header Table
 * are returned as a set with all Indexes in the internal format, not HPACK index
 * format.
 *
 * @param[in,out] table        Header Table where we want the field added.
 * @param[in]     max          New Maximum Size.
 * @param[out]    evicted_set  Set with all the evicted indexes.
 *
 * @return The result of the operation.
 * @retval ret_error  Max size is greater than SETTINGS_HEADER_TABLE_SIZE.
 * @retval ret_ok     New Max size set.
 */
ret_t
hpack_header_table_set_max (hpack_header_table_t *table,
                            uint16_t              max,
                            hpack_set_t           evicted_set)
{
    ret_t ret;
    int   evicted;

    /* Initially evicted set is empty. */
    hpack_set_init (evicted_set, false);

    /* The table can never be larger than HTTP/2's SETTINGS_HEADER_TABLE_SIZE */
    if (unlikely(max) > SETTINGS_HEADER_TABLE_SIZE)
        return ret_error;

    /* Encoder is not going to work with Header Table */
    if (0 == max) {
        hpack_header_table_clear (table);

    /* Decrease and lose data. Evict entries until we don't use more than the new max. */
    } else {
        while (max < table->used_data) {
            ret = header_table_evict (table, &evicted);
            if (unlikely (ret != ret_ok)) return ret;

            /* We know there are elements, otherwise we wouldn't have more than max data. */
            hpack_set_add (evicted_set, (unsigned int) evicted);
        }
    }

    table->max_data = max;

    return ret_ok;
}


/** Get an entry from the Header Table using a non HPACK index
 *
 * Get a Header Field data from the Header Table using an Index from the internal
 * reference (one used in a set).
 *
 * The index can only be from the Header Table, not from the Static Table.
 *
 * You may specify if we only want the data from the name and the header flags.
 *
 * @param[in]  table      Header Table to get the Header from.
 * @param[in]  n          The index of the Header we want.
 * @param[in]  only_name  If we only want the name and flags.
 * @param[out] f          Header Field to store the data.
 *
 * @return The result of the operation.
 * @retval ret_not_found  Requested an entry out of the possible range.
 * @retval ret_nomem      Chula buffer couldn't reserve needed mem.
 * @retval ret_ok         The data was retrieved.
 */
ret_t
hpack_header_table_get_set_idx (hpack_header_table_t *table,
                                uint16_t              n,
                                bool                  only_name,
                                hpack_header_field_t *f)
{
    ret_t                           ret;
    uint16_t                        offset;
    hpack_header_table_field_info_t info;

    if (n >= HPACK_MAX_HEADER_TABLE_ENTRIES)
        return ret_not_found;

    if (unlikely (! hpack_header_offset_has_data(table, n)))
        return ret_error;

    /* Get the position of the header */
    offset = table->headers_offsets.buffer [n];

    /* Get the info of the header */
    header_data_get (&table->headers_data, offset, (char *)&info, sizeof(info));

    f->flags = info.flags;

    /* Get the name data */
    header_cb_move (offset, sizeof(info), HPACK_CB_HEADER_DATA_SIZE, HPACK_CB_HEADER_DATA_MASK);
    ret = header_data_get_chula (&table->headers_data, offset, &f->name, info.name_length);

    if (only_name || (ret_ok != ret ))
        return ret;

    /* Get the value data if there's data in it. */
    if (0 < info.value_length) {
        header_cb_move (offset, info.name_length, HPACK_CB_HEADER_DATA_SIZE, HPACK_CB_HEADER_DATA_MASK);
        ret = header_data_get_chula (&table->headers_data, offset, &f->value, info.value_length);
    }

    return ret;
}



/** Get an entry from the Header Table and the Static Table
 *
 * Get a Header Field data from the Header Table or Static Table using an HPACK
 * Index.
 *
 * The Index must belong to the Index [Address Space](http://http2.github.io/http2-spec/compression.html#rfc.figure.1).
 *
 * You may specify if we only want the data from the name and the header flags.
 *
 * You also get whether the field is from the Header Table or from the Static Table.
 *
 * @param[in]  table      Header Table to get the Header from.
 * @param[in]  n          The index of the Header we want.
 * @param[in]  only_name  If we only want the name and flags.
 * @param[out] f          Header Field to store the data.
 * @param[out] is_static  If requested header is from the Static Table.
 *
 * @return The result of the operation.
 * @retval ret_not_found  Requested an entry out of the possible range.
 * @retval ret_nomem      Chula buffer couldn't reserve needed mem.
 * @retval ret_ok         The data was retrieved.
 */
ret_t
hpack_header_table_get (hpack_header_table_t *table,
                        uint16_t              n,
                        bool                  only_name,
                        hpack_header_field_t *f,
                        bool                 *is_static)
{
    ret_t ret;

    if (unlikely ((f == NULL) || (is_static == NULL)))
        return ret_error;

    if (n > STATIC_ENTRIES + table->num_headers)
        return ret_not_found;

    *is_static = n > table->num_headers;

    /* Retrieve indexed entry from static table */
    if (*is_static) {

        /* Adjust index. */
        n -= table->num_headers + 1;

        f->flags = static_table[n].flags;

        /* Get only the name. */
        ret = chula_buffer_add_buffer (&f->name, &static_table[n].name);

        /* If there's been an error or we only need the name. */
        if ((only_name) || (ret_ok != ret))
            return ret;

        /* Get the value. */
        ret = chula_buffer_add_buffer (&f->value, &static_table[n].value);

        return ret;
    }

    /* Convert the index from HPACK representation to our internal representation */
    n = INDEX_SWITCH_HT_HPACK(table, n);

    /* Get the data and return the result. */
    return (hpack_header_table_get_set_idx (table, n, only_name, f));
}


/** Returns the next existing element in the set
 *
 * Gets the next HPACK index from the Set continuing where it left of in the
 * previous call.
 *
 * The function is oblivious to changes to the Set. So you will only catch
 * changes to the Set if they are from an index with a higher value than the last
 * returned value.
 *
 * If you modify the Set or if you finish retrieving all items from a Set you
 * must [reinitialize](@ref hpack_header_table_iter_init) or [reset](@ref hpack_header_table_iter_reset)
 * the iterator before you can retrieve any more indexes with the iterator.
 *
 * @code
 * hpack_set_t          b_set;
 * hpack_set_iterator_t iter;
 * int16_t              idx;
 *
 * hpack_header_table_set_init (b_set, true);
 * hpack_header_table_iter_init (&iter, b_set);
 * do {
 *     idx = hpack_header_table_iter_next (&iter);
 *     if (-1 != idx) {
 *       printf ("Index %d found\n", idx);
 *     }
 * } while (-1 != idx);
 * @endcode
 *
 * @pre @a iter must have been previously initialized with [hpack_header_table_iter_init](@ref hpack_set_iter_init)
 *
 * @param[in]     table  Table related to the iterator.
 * @param[in,out] iter   Iterator to use.
 *
 * @return The next index in the set.
 * @retval   -1 No more indexes to return.
 * @retval !=-1 The HPACK index found in the set.
 *
 * @see hpack_header_table_iter_reset()
 * @see hpack_header_table_iter_next()
 */
int
hpack_header_table_iter_next (hpack_header_table_t *table,
                              hpack_set_iterator_t *iter)
{
    int i;

    i = hpack_set_iter_next (iter);

    if (-1 == i)
        return -1;

    return (INDEX_SWITCH_HT_HPACK(table, i));
}


/** Creates a string representation
 *
 * Generates a string representation of the supplied Header Table.
 *
 * The format used to represent the Header Table is:
 * hpack_header_table@@@ePOINTER [ @e INDEX @e NAME: @e VALUE [ @e REPRESENTATION | Name: @e TYPE_OF_NAME | Value: @e TYPE_OF_VALUE ]
 *
 * @pre @a table needs to have at the very least been [initialized](@ref hpack_header_table_init)
 * @pre @a output chula buffer needs to have been [initialized](@ref chula_buffer_init)
 *
 * @param[in]   table    Header Table to represent.
 * @param[out]  output   Where to output the representation.
 *
 * @return If the operation was successful.
 * @retval ret_ok     The operation was completed successfully.
 * @retval ret_nomem  There was no memory to complete the operation.
 *
 */
void
hpack_header_table_repr (hpack_header_table_t *table,
                         chula_buffer_t       *output)
{
    hpack_header_field_t field;
    bool                 is_static;

    /** @todo Try to do a nicer presentation where fields are aligned between lines. */
    hpack_header_field_init (&field);

    chula_buffer_add_va  (output, "hpack_header_table@%x\n", POINTER_TO_INT(table));

    /* Lazy and wasteful way to do it, since we don't use the internal structures. */
    for (uint16_t i=1; i <= table->num_headers; i++) {

        /* Get the field. */
        hpack_header_table_get (table, i, false, &field, &is_static);

        /* Represent the name-value pair. */
        chula_buffer_add_va     (output, "  [%03d] ", i);
        chula_buffer_add_buffer (output, &field.name);
        chula_buffer_add_str    (output, ": ");
        chula_buffer_add_buffer (output, &field.value);
        chula_buffer_add_str    (output, " [");

        /* Represent the flags. */
        chula_buffer_add_buffer (output,
                                 &hpack_header_field_representations_repr[field.flags.rep]);
        chula_buffer_add_str    (output, " | Name: ");
        chula_buffer_add_buffer (output,
                                 &hpack_header_field_field_type_repr[field.flags.name]);
        chula_buffer_add_str    (output, " | Value: ");
        chula_buffer_add_buffer (output,
                                 &hpack_header_field_field_type_repr[field.flags.value]);
        chula_buffer_add_str    (output, "]");

        chula_buffer_add_str    (output, CRLF);
        hpack_header_field_clean (&field);
    }

    hpack_header_field_mrproper (&field);
}

/** @endcond */
