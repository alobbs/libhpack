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
 * @file      header_parser.c
 * @brief     HPACK's Header Parser Implementation
 *
 * Implementation of a parser/decoder for HPACK Header Blocks as specified in
 * [HPACK - Header Compression for HTTP/2](http://http2.github.io/http2-spec/compression.html).
 *
 * Current implementation is up to date with Draft 7.
 *
 * @author    Alvaro Lopez Ortega <alvaro@gnu.org>
 * @author    Gorka Eguileor <gorka@eguileor.com>
 * @date      April, 2014
 */

#include "header_parser.h"
#include "integer.h"
#include "huffman.h"


/** Reserves memory for a new Header Parser and initializes it
 *
 * Creates a new Header Parser by reserving the memory needed for it and
 * initializes it.
 *
 * @param[out] parser  Reference to the pointer of the new Parser.
 *
 * @return Result of the operation.
 * @retval ret_nomem  There's no memory for the new Header Parser.
 * @retval ret_ok     Created successfully.
 */
ret_t hpack_header_parser_new (hpack_header_parser_t **parser)
{
    *parser = (hpack_header_parser_t *) malloc (sizeof(hpack_header_parser_t));

    if (NULL == *parser)
        return ret_nomem;

    return (hpack_header_parser_init (*parser));
}


/** Header Parser initializer
 *
 * Initializer function for an HPACK parser.
 * It can be used with Header Parsers created with the [new](@ref hpack_header_parser_new)
 * function and those created directly in the stack i.e. `hpack_header_parser_t hp_variable;`
 *
 * @param parser  Header Parser to initialize
 *
 * @return Result of the initialization.
 * @retval ret_ok  Currently this is the only possible result.
 */
ret_t
hpack_header_parser_init (hpack_header_parser_t *parser)
{
    ret_t ret;

    parser->store = NULL;

    hpack_header_table_set_init (parser->context.reference_set, false);
    hpack_header_table_set_init (parser->context.ref_not_emitted, false);
    hpack_header_table_iter_init (&parser->context.iter_not_emitted, parser->context.ref_not_emitted);

    parser->context.finished = false;

    ret = hpack_header_table_init (&parser->context.table);

    return ret;
}


/** Clean up all memory used by the Header parser
 *
 * Clean up all memory used by a Header Parser previously created with the
 * [new](@ref hpack_header_parser_new) function.
 *
 * The Parser pointer is set to NULL.
 *
 * @pre @a parser was created with [hpack_header_parser_new](@ref hpack_header_parser_new)
 *
 * @param[in,out] parser  Header Parser to free.
 *
 * @return Result of the operation.
 * @retval ret_ok  Only possible value returned.
 */
ret_t
hpack_header_parser_mrproper (hpack_header_parser_t **parser)
{
    free (*parser);
    *parser = NULL;
    return ret_ok;
}


/** Register a Storage for decoded Header Fields
 *
 * Register a new Storage to keep decoded Header Fields when using
 * [hpack_header_parser_all](@ref hpack_header_parser_all).
 *
 * @pre @a parser was [initialized](@ref hpack_header_parser_init) or created with
 * [new](@ref hpack_header_parser_new).
 *
 * @param[out]   parser  Parser to process the registration.
 * @param[in]    store   Store to register.
 *
 * @return Result of the registration.
 * @retval ret_ok  Currently is the only possible result.
 */
ret_t
hpack_header_parser_reg_store (hpack_header_parser_t *parser,
                               hpack_header_store_t  *store)
{
    parser->store = store;
    return ret_ok;
}


/**
 * @cond INTERNAL
 * Adds a Header Field to the Header Table and takes care of the evictions by
 * removing them from the sets of the context.
 *
 * [According to the specifications](http://http2.github.io/http2-spec/compression.html#entry.addition):
 * > It is not an error to attempt to add an entry that is larger than the
 * > maximum size.
 *
 * So if a pointer to a boolean is supplied it will inform on whether the field
 * was actually added to the Header Table or not.
 *
 * @param[in,out] context  Decoding context where we want to add the field.
 * @param[in]     field    Field to add.
 * @param[out]    added    If the field has been added.
 *
 * @return Result of the operation.
 * @endcond
 */
static ret_t
add_field_process_evictions (hpack_header_parser_context_t *context,
                             hpack_header_field_t          *field,
                             bool                          *added)
{
    ret_t       ret;
    hpack_set_t evicted_set;

    /* Add the Header Field. */
    ret = hpack_header_table_add (&context->table, field, evicted_set);
    if (ret_ok != ret)
        return ret;

    /* If we have evictions we have to remove them from the reference set, since
     * they can no longer be referenced because they've been evicted. */
    hpack_header_table_set_relative_comp (context->reference_set, evicted_set);
    hpack_header_table_set_relative_comp (context->ref_not_emitted, evicted_set);

    /* If we have to return whether the field was added or not. */
    if (NULL != added)
        *added = !hpack_header_table_set_is_full (evicted_set);

    /* Even if we couldn't add it it's not an error according to HPACK specs. */
    return ret_ok;
}


/**
 * @cond INTERNAL
 * This function parses a string that uses [HPACK's String Representation](http://http2.github.io/http2-spec/compression.html#string.literal.representation).
 *
 @verbatim
     0   1   2   3   4   5   6   7        0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+    +---+---+---+---+---+---+---+---+
   | 1 |  Value Length Prefix (7)  |    | 0 |  Value Length Prefix (7)  |
   +---+---+---+---+---+---+---+---+    +---+---+---+---+---+---+---+---+
   |   Value Length (0-N octets)   |    |   Value Length (0-N octets)   |
   +---+---+---+---+---+---+---+---+    +---+---+---+---+---+---+---+---+
   | Huffman Encoded Data  |Padding|    |  Field Bytes without Encoding |
   +---+---+---+---+---+---+---+---+    +---+---+---+---+---+---+---+---+
             Huffman encoded                    No-Huffman encoded
 @endverbatim
 *
 * Not only returns the string, but also whether it was Huffman encoded and how
 * many bytes were consumed to decode the String Representation.
 *
 * @param[in]  buf       Buffer with String Representation.
 * @param[in]  offset    Offset of the String Representation in the @a buf.
 * @param[out] string    Destination of decoded string.
 * @param[out] huffman   If it was huffman encoded.
 * @param[out] consumed  How many octects were consumed.
 *
 * @return Result of the operation.
 * @retval ret_eagain  Try again when there's more data in the buffer.
 * @retval ret_ok      String parsed successfully.
 * @retval ret_nomem   Not enough memory to allocate the @a string
 * @endcond
 */
static ret_t
parse_string (chula_buffer_t       *buf,
              unsigned int          offset,
              chula_buffer_t       *string,
              bool                 *huffman,
              unsigned int         *consumed)
{
    ret_t        ret;
    uint32_t     n    = offset;
    unsigned int con  = 0;
    unsigned int len  = 0;

    /* Unless we process the full string we haven't consumed any bytes. */
    *consumed = 0;

    *huffman = ((uint8_t)buf->buf[n]) & 0x80;

    /* Decode the length of the string. */
    ret = hpack_integer_decode (7, (unsigned char*)buf->buf + n, buf->len - n, &len, &con);
    if (unlikely (ret != ret_ok)) return ret_error;
    n += con;

    /** @todo FIX this, because a Huffman encoded string does not need to fulfil this criteria. */
    if (buf->len < n + len) {
        return ret_eagain;
    }

    /* Copy information */
    if (*huffman) {
        hpack_huffman_decode_context_t context = HUFFMAN_DEC_CTX_INIT;
        chula_buffer_t                 in      = CHULA_BUF_INIT_FAKE_LEN (buf->buf+n, len);

        ret = hpack_huffman_decode (&in, string, &context);
        if (unlikely (ret != ret_ok)) return ret_error;
        n += len;
    }
    else{
        ret = chula_buffer_add (string, (const char *)buf->buf + n, len);
        if (unlikely (ret != ret_ok)) return ret_error;
        n += len;
    }

    /* Return */
    *consumed = (n - offset);
    return ret_ok;
}


/**
 * @cond INTERNAL
 * This function parses an Index that uses [HPACK's Indexed Header Field Representation](http://http2.github.io/http2-spec/compression.html#indexed.header.representation).
 *
 @verbatim
         Indexed Header Field
    0   1   2   3   4   5   6   7
  +---+---+---+---+---+---+---+---+
  | 1 |        Index (7+)         |
  +---+---------------------------+
 @endverbatim
 *
 * One it has parsed the Index it Processes the index as described in the [specifications](http://http2.github.io/http2-spec/compression.html#header.representation.processing)
 * > An indexed representation corresponding to an entry present in the reference
 * > set entails the following actions:
 * > - The entry is removed from the reference set.
 * > An indexed representation corresponding to an entry not present in the
 * > reference set entails the following actions:
 * > - If referencing an element of the static table:
 * >   - The header field corresponding to the referenced entry is emitted.
 * >   - The referenced static entry is inserted at the beginning of the header
 * >      table.
 * >   - A reference to this new header table entry is added to the reference set,
 * >     except if this new entry didn't fit in the header table.
 * > - If referencing an element of the header table:
 * >   - The header field corresponding to the referenced entry is emitted.
 * >   - The referenced header table entry is added to the reference set.
 *
 * If the representation already existed in the reference set returned @a field
 * will be empty.
 *
 * Not only returns the Header Field referenced by the Indexed Representation,
 * but also how many bytes were consumed to decode the Representation.
 *
 * @pre @a field is expected to be empty when the function is called.
 *
 * @param[in]     buf       Buffer with Indexed Representation.
 * @param[in]     offset    Offset of the Indexed Representation in the @a buf.
 * @param[in,out] context   Decoding context for the Indexed Representation.
 * @param[out]    field     Field referenced by the Index.
 * @param[out]    consumed  How many octects were consumed.
 *
 * @return Result of the operation.
 * @endcond
 */
static inline ret_t
parse_indexed (chula_buffer_t                *buf,
               unsigned int                   offset,
               hpack_header_parser_context_t *context,
               hpack_header_field_t          *field,
               unsigned int                  *consumed)
{
    ret_t        ret;
    unsigned int num;
    bool         is_static;
    unsigned int con        = 0;
    int          n          = offset;

    /* Unless everything goes OK we haven't consumed any bytes. */
    *consumed = 0;

    /* Read index number. */
    ret = hpack_integer_decode (7, (unsigned char *)buf->buf + n, buf->len - n, &num, &con);
    if (ret != ret_ok) return ret_error;

    /* Invalid index requested. */
    if (num > STATIC_ENTRIES + context->table.num_headers) {
        /** @todo This may means that we have to purge the context, review HPACK specs */
        return ret_error;
    }

    /* If the Index is already in the reference set we must remove it. */
    if (hpack_header_table_set_exists (&context->table, context->reference_set, num)) {
        ret  = hpack_header_table_set_remove (&context->table, context->reference_set,   num);
        ret += hpack_header_table_set_remove (&context->table, context->ref_not_emitted, num);
        if (ret_ok != ret) return ret;

        *consumed = con;
        return ret_ok;
    }

    field->flags.rep = rep_indexed;

    /* Get referred index. */
    ret = hpack_header_table_get (&context->table, num, false, field, &is_static);
    if (ret_ok != ret) return ret;

    /* If it's a static entry it must be added to Header Table. */
    if (is_static) {
        /* We don't need to check if it could be added because we know that all
         * static entries fit in the Header Table.
         */
        ret = add_field_process_evictions (context, field, NULL);
        if (ret_ok != ret) return ret;

        /* Since it has now been added it has a new index which will be used in the reference set. */
        num = 1;
    }

    /* Add to the reference and remove from the not emitted set. */
    hpack_header_table_set_add (&context->table, context->reference_set, num);
    hpack_header_table_set_remove (&context->table, context->ref_not_emitted, num);

    *consumed = con;
    return ret_ok;
}


/**
 * @cond INTERNAL
 * This function parses an [HPACK's Literal Header Field Representation](http://http2.github.io/http2-spec/compression.html#literal.header.representation).
 *
 @verbatim
  LITERAL HEADER FIELD WITH INCREMENTAL INDEXING

    0   1   2   3   4   5   6   7
  +---+---+---+---+---+---+---+---+
  | 0 | 1 |      Index (6+)       |
  +---+---+---+-------------------+
  | H |     Value Length (7+)     |
  +-------------------------------+
  | Value String (Length octets)  |
  +-------------------------------+
             Indexed Name

    0   1   2   3   4   5   6   7
  +---+---+---+---+---+---+---+---+
  | 0 | 1 |           0           |
  +---+---+---+-------------------+
  | H |     Value Length (7+)     |
  +-------------------------------+
  |  Name String (Length octets)  |
  +-------------------------------+
  | H |     Value Length (7+)     |
  +-------------------------------+
  | Value String (Length octets)  |
  +-------------------------------+
              New Name


  LITERAL HEADER FIELD WITHOUT INDEXING

    0   1   2   3   4   5   6   7
  +---+---+---+---+---+---+---+---+
  | 0 | 0 | 0 | 0 |  Index (4+)   |
  +---+---+-----------------------+
  | H |     Value Length (7+)     |
  +---+---------------------------+
  | Value String (Length octets)  |
  +-------------------------------+
             Indexed Name

    0   1   2   3   4   5   6   7
  +---+---+---+---+---+---+---+---+
  | 0 | 0 | 0 | 0 |       0       |
  +---+---+-----------------------+
  | H |     Name Length (7+)      |
  +---+---------------------------+
  |  Name String (Length octets)  |
  +---+---------------------------+
  | H |     Value Length (7+)     |
  +---+---------------------------+
  | Value String (Length octets)  |
  +-------------------------------+
              New Name


  LITERAL HEADER FIELD NEVER INDEXED

    0   1   2   3   4   5   6   7
  +---+---+---+---+---+---+---+---+
  | 0 | 0 | 0 | 1 |  Index (4+)   |
  +---+---+-----------------------+
  | H |     Value Length (7+)     |
  +---+---------------------------+
  | Value String (Length octets)  |
  +-------------------------------+
             Indexed Name

    0   1   2   3   4   5   6   7
  +---+---+---+---+---+---+---+---+
  | 0 | 0 | 0 | 1 |       0       |
  +---+---+-----------------------+
  | H |     Name Length (7+)      |
  +---+---------------------------+
  |  Name String (Length octets)  |
  +---+---------------------------+
  | H |     Value Length (7+)     |
  +---+---------------------------+
  | Value String (Length octets)  |
  +-------------------------------+
              New Name

 @endverbatim
 * Not only returns the Header Field decoded from the buffer, but also how many
 * bytes were consumed to decode the Representation.
 *
 * @pre @a field is expected to be empty when the function is called.
 *
 * @param[in]     buf       Buffer with Literal Representation.
 * @param[in]     offset    Offset of the Literal Representation in the @a buf.
 * @param[in,out] context   Decoding context for the Literal Representation.
 * @param[out]    field     Field to return the Header Pair.
 * @param[out]    consumed  How many octects were consumed.
 *
 * @return Result of the operation.
 * @endcond
 */
static inline ret_t
parse_header_pair (chula_buffer_t                *buf,
                   unsigned int                   offset,
                   hpack_header_parser_context_t *context,
                   hpack_header_field_t          *field,
                   unsigned int                  *consumed)
{
    ret_t          ret;
    unsigned int   n    = offset;
    unsigned int   con  = 0;
    uint32_t       len  = 0;
    bool           huffman;

    /* Unless everything goes OK we haven't consumed any bytes. */
    *consumed = 0;

    /* If The Name is indexed */
    if (buf->buf[n] & 0x0F) {
        bool is_static;
        int  prefix;

        /* We have 2 possible prefixes 6 and 4. */
        prefix = buf->buf[n] & 0xC0? 6 : 4;

        /* Decode the Index. */
        ret = hpack_integer_decode (prefix, (unsigned char *)buf->buf+n, buf->len-n, &len, &con);
        if (unlikely (ret != ret_ok)) return ret_error;
        n += con;

        /* Get the Name from the Header Table. */
        ret = hpack_header_table_get (&context->table, len, true, field, &is_static);
        field->flags.name = is_static? is_indexed_static : is_indexed_ht;
        if (ret != ret_ok) return ret;

    } else {
        n += 1;

        /* Get the Name in String Representation from the buffer. */
        ret = parse_string (buf, n, &field->name, &huffman, &con);
        if (ret != ret_ok) return ret;

        field->flags.name = huffman? is_new_huffman : is_new;
        n += con;
    }

    /* The Value always comes as a String Representation. */
    ret = parse_string (buf, n, &field->value, &huffman, &con);
    if (ret != ret_ok) return ret;
    n += con;

    field->flags.value = huffman? is_new_huffman : is_new;

    /* Return */
    *consumed = n - offset;
    return ret_ok;
}


/**
 * @cond INTERNAL
 * This function processes an [Encoded Context Update](http://http2.github.io/http2-spec/compression.html#encoding.context.update).
 *
 @verbatim
    0   1   2   3   4   5   6   7
  +---+---+---+---+---+---+---+---+
  | 0 | 0 | 1 | 1 |       0       |
  +---+---------------------------+
       Reference Set Emptying

    0   1   2   3   4   5   6   7
  +---+---+---+---+---+---+---+---+
  | 0 | 0 | 1 | 0 | Max size (7+) |
  +---+---------------------------+
   Maximum Header Table Size Change
 @endverbatim
 *
 * If a new Maximum Header Table Size is requested and evictions are needed, they
 *  will be processed according to [specifications](http://http2.github.io/http2-spec/compression.html#entry.eviction).
 *
 * @param[in]     buf       Buffer with the Context Update.
 * @param[in]     offset    Offset of the Context Update in the @a buf.
 * @param[in,out] context   Decoding context to be updated.
 * @param[out]    consumed  How many octects were consumed.
 *
 * @return Result of the operation.
 * @endcond
 */
static inline ret_t
parse_context_update (chula_buffer_t                *buf,
                      unsigned int                   offset,
                      hpack_header_parser_context_t *context,
                      unsigned int                  *consumed)
{
    ret_t        ret;
    uint32_t     num;
    unsigned int con  = 0;
    hpack_set_t  evicted_set;

    /* Unless everything goes OK we haven't consumed any bytes */
    *consumed = 0;

    /* Requested Reference Set Emptying */
    if ((uint8_t)buf->buf[offset] == 0x30) {
        hpack_header_table_set_clear (context->reference_set);
        hpack_header_table_set_clear (context->ref_not_emitted);

        *consumed = 1;
        return ret_ok;
    }

    /* Get new max length. */
    ret = hpack_integer_decode (4, (unsigned char *)buf->buf + offset, buf->len - offset, &num, &con);
    if (ret != ret_ok) return ret_error;

    /* Set the new size and get the set of evicted elements. */
    ret = hpack_header_table_set_max (&context->table, num, evicted_set);
    if (ret != ret_ok) return ret_error;

    /* If we have evictions we have to remove them from the reference set, since
     * they can no longer be referenced.
     */
    hpack_header_table_set_relative_comp (context->reference_set, evicted_set);
    hpack_header_table_set_relative_comp (context->ref_not_emitted, evicted_set);

    *consumed = con;
    return ret_ok;
}


/**
 * @cond INTERNAL
 * This function does the final [Reference Set Emission](http://http2.github.io/http2-spec/compression.html#reference.set.emission).
 *
 * Which means that once all the representations contained in a header block have
 * been processed, the header fields referenced in the reference set which have
 * not previously been emitted during this processing are emitted.
 *
 * The function will return new fields until there are no more pending references
 * to emit. Which will be signaled by @c ret_eof.
 *
 * @pre @a field is expected to be empty when the function is called.
 *
 * @param[in]  context   Decoding context from were to process pending emissions.
 * @param[out] field     Field from the reference set pending to emit.
 * @param[out] consumed  Kept for compatibility with other functions. Always
 *                       returns 0
 *
 * @return Result of the operation.
 * @retval ret_ok  There is a field pending emission and it's returned in @a field.
 * @retval ret_eof No more fields are pending emission.
 * @endcond
 */
static inline ret_t
final_reference_set_process (hpack_header_parser_context_t *context,
                             hpack_header_field_t          *field,
                             unsigned int                  *consumed)
{
    bool is_static;
    int idx;

    *consumed = 0;

    if (context->finished)
        return ret_eof;

    /* Get the next index that is pending emission. */
    idx = hpack_header_table_iter_next (&context->table, &context->iter_not_emitted);

    /* We have finished with the references, so we set it all up for the next header block. */
    if (-1 == idx) {
        /* Mark the block as finished in case they call us again. */
        context->finished = true;

        /* Set the not emitted set and reset the iterator. */
        hpack_header_table_set_set (context->ref_not_emitted, context->reference_set);
        hpack_header_table_iter_reset (&context->iter_not_emitted);

        return ret_eof;
    }

    /* Get Header Field we have to return for emission. */
    hpack_header_table_get (&context->table, idx, false, field, &is_static);

    /* Remove it from the not emitted set. This is not needed because we are
     * using an iterator, but it's done for consistency of the data.
     */
    hpack_header_table_set_remove (&context->table, context->ref_not_emitted, idx);

    return ret_ok;
}


/** Parse data for 1 field
 *
 * This function processes the next Header Field from the buffer as defined in
 * [HPACK's Header Block Decoding](http://http2.github.io/http2-spec/compression.html#header.block.decoding)
 * and if this processing produces the emission of a new Header Field it will be
 * returned in @a field.
 *
 * Since some representations do not emit a header the function could return an
 * empty @a field.
 *
 * Sample code:
 * @code
 * ret_t                  ret;
 * chula_buffer_t         raw;
 * hpack_header_parser_t *parser;
 * hpack_header_field_t   field;
 * unsigned int           consumed = 0;
 *
 * hpack_header_parser_new (&parser);
 * hpack_header_field_init (&field);
 * chula_buffer_fake_str (&raw, "\x44\x0c\x2f\x73\x61\x6d\x70\x6c\x65\x2f\x70\x61\x74\x68");
 *
 * ret = hpack_header_parser_field (parser, &raw, 0, &field, &consumed);
 * if (ret != ret_ok) {
 *     printf ("Error\n");
 *
 * } else {
 *     chula_buffer_t repr;
 *     chula_buffer_init (&repr);
 *     hpack_header_field_repr (&field, &repr);
 *     printf (repr.buf);
 * }
 *
 * hpack_header_field_mrproper (&field);
 * hpack_header_parser_mrproper (&parser);
 * @endcode
 *
 * @param[in,out] parser    Parser used for decoding.
 * @param[in]     buf       Buffer with the Header Block.
 * @param[in]     offset    Offset wihtin the @a buf.
 * @param[out]    field     Header Field emited.
 * @param[out]    consumed  How many octects were consumed.
 *
 * @return Result of the header processing.
 */
ret_t
hpack_header_parser_field (hpack_header_parser_t *parser,
                           chula_buffer_t        *buf,
                           unsigned int           offset,
                           hpack_header_field_t  *field,
                           unsigned int          *consumed)
{
    ret_t          ret;
    bool           do_indexing;
    unsigned char  c              = buf->buf[offset];

    /* Field is empty unless we emit a header. */
    hpack_header_field_clean (field);

    /* If there's no more data it means we have to proceed with the Reference Set Emission. */
    if (offset == buf->len)
        return (final_reference_set_process (&parser->context, field, consumed));

    /** @todo It's not nice to be setting @a context.finished on every iteration */
    parser->context.finished = false;

    /* Parse field
     */
    if ((c & 0xE0) == 0x20) {
        /* Context update */
        ret = parse_context_update (buf, offset, &parser->context, consumed);
        return ret;
    }
    else if (c & 0x80u) {
        /* Indexed header field: 1st bit set */
        ret = parse_indexed (buf, offset, &parser->context, field, consumed);
        if (ret != ret_ok) return ret;
    }
    else {
        ret = parse_header_pair (buf, offset, &parser->context, field, consumed);
        if (ret != ret_ok) return ret;

        /* Add to header table
         */
        do_indexing = ((c & 0xc0) == 0x40u);

        if (do_indexing) {
            bool added;

            field->flags.rep = rep_inc_indexed;
            ret = add_field_process_evictions (&parser->context, field, &added);
            if (ret_ok != ret) return ret;

            /* If we were able to add it to the Header Table. */
            if (added) {

                /* Add to the reference set and remove from the not emitted set. */
                hpack_header_table_set_add (&parser->context.table, parser->context.reference_set, 1);
                hpack_header_table_set_remove (&parser->context.table, parser->context.ref_not_emitted, 1);
            }

        } else {
            field->flags.rep = c & 0xF0 ? rep_never_indx : rep_wo_indexing;
        }
    }

    return ret_ok;
}


/** Parse complete Header Block
 *
 * This function processes a full [HPACK Header Block](http://http2.github.io/http2-spec/compression.html#header.block.decoding)
 * and if a storage has been registered all emitted fields will be added to the
 * storage.
 *
 * Sample code:
 * @code
 * ret_t                  ret;
 * chula_buffer_t         raw;
 * hpack_header_store_t   store;
 * hpack_header_parser_t *parser;
 * unsigned int           consumed = 0;
 *
 * chula_buffer_fake_str (&raw, "\x08\x82\x98\xa7\x18\x85\x73\xd5\xcd\x11\x1f\x22\x98\xef\x6b\x3a\x7a\x0e\x6e\x8f\xa2\x63\xd0\x72\x9a\x6e\x83\x97\xd8\x69\xbd\x87\x37\x47\xbb\xbf\xc7\x30\x90\xce\x31\x74\x3d\x80\x1b\x6d\xb1\x07\xcd\x1a\x39\x62\x44\xb7\x4f");
 *
 * hpack_header_store_init (&store);
 * hpack_header_parser_new (&parser);
 * hpack_header_parser_reg_store (parser, &store);
 *
 * ret = hpack_header_parser_all (parser, &raw, 0, &consumed);
 *
 * if (ret != ret_ok) {
 *     printf ("Error\n");
 *
 * } else {
 *     chula_buffer_t repr;
 *     chula_buffer_init (&repr);
 *     hpack_header_store_repr (&store, &repr);
 *     printf (repr.buf);
 * }
 *
 * hpack_header_store_mrproper (&store);
 * hpack_header_parser_mrproper (&parser);
 * @endcode
 *
 * @param[in,out] parser    Parser used for decoding.
 * @param[in]     buf       Buffer with the Header Block.
 * @param[in]     offset    Offset wihtin the @a buf.
 * @param[out]    consumed  How many octects were consumed to decode the Header Block.
 *
 * @return Result of the header processing.
 */
ret_t
hpack_header_parser_all (hpack_header_parser_t *parser,
                         chula_buffer_t        *buf,
                         unsigned int           offset,
                         unsigned int          *consumed)
{
    ret_t                 ret;
    hpack_header_field_t field;

    hpack_header_field_init (&field);

    /* Parse raw header
     */
    while (true) {
        unsigned int         con   = 0;

        /* Parse a single header field */
        ret = hpack_header_parser_field (parser, buf, offset, &field, &con);

        /* Exit: When we have finished processing all the data + the Reference
         * Header Set. re_eof signals just that and we must actually exit with
         * ret_ok.
         */
        if (ret_eof == ret) {
            ret = ret_ok;
            break;
        }

        /* Exit on error. */
        if (ret != ret_ok) break;;

        if (con > 0) {
            offset    += con;
            *consumed += con;
        }

        /* Emit only when there's a registered store and we have decoded a Field
         * that must be emitted (non empty).
         */
        if ((parser->store) &&
            (! hpack_header_field_is_empty(&field)))
        {
            ret = hpack_header_store_emit (parser->store, &field);
            if (ret != ret_ok) return ret;
        }
    }

    hpack_header_field_mrproper (&field);
    return ret;
}
