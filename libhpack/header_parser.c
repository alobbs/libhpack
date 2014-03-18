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

#include "header_parser.h"
#include "integer.h"
#include "huffman.h"

ret_t
hpack_header_parser_init (hpack_header_parser_t *parser)
{
    ret_t ret;

    parser->store = NULL;

    ret = hpack_header_table_init (&parser->table);
    if (unlikely (ret != ret_ok)) return ret;

    return ret_ok;
}


ret_t
hpack_header_parser_mrproper (hpack_header_parser_t *parser)
{
    ret_t ret;

    ret = hpack_header_table_mrproper (&parser->table);
    if (unlikely (ret != ret_ok)) return ret;

    return ret_ok;
}


ret_t
hpack_header_parser_reg_store (hpack_header_parser_t *parser,
                               hpack_header_store_t  *store)
{
    parser->store = store;
    return ret_ok;
}


static ret_t
parse_string (chula_buffer_t       *buf,
              unsigned int          offset,
              chula_buffer_t       *string,
              unsigned int         *consumed)
{
    ret_t        ret;
    uint32_t     n    = offset;
    unsigned int con  = 0;
    unsigned int len  = 0;
    bool         huffman;

/*
    0   1   2   3   4   5   6   7        0   1   2   3   4   5   6   7
  +---+---+---+---+---+---+---+---+    +---+---+---+---+---+---+---+---+
  | 1 |  Value Length Prefix (7)  |    | 0 |  Value Length Prefix (7)  |
  +---+---+---+---+---+---+---+---+    +---+---+---+---+---+---+---+---+
  |   Value Length (0-N octets)   |    |   Value Length (0-N octets)   |
  +---+---+---+---+---+---+---+---+    +---+---+---+---+---+---+---+---+
  | Huffman Encoded Data  |Padding|    |  Field Bytes without Encoding |
  +---+---+---+---+---+---+---+---+    +---+---+---+---+---+---+---+---+
            Huffman encoded                    No-Huffman encoded
*/

    huffman = ((uint8_t)buf->buf[n]) & 0x80;

    /* Name */
    ret = integer_decode (7, (unsigned char*)buf->buf + n, buf->len - n, &len, &con);
    if (unlikely (ret != ret_ok)) return ret_error;
    n += con;

    /* Copy information */
    if (buf->len < n + len) {
        return ret_eagain;
    }

    if (huffman) {
        hpack_huffman_decode_context_t context = HUFFMAN_DEC_CTX_INIT;
        chula_buffer_t                 in      = CHULA_BUF_INIT_FAKE_LEN (buf->buf+n, len);

        ret = hpack_huffman_decode (&in, string, &context);
        if (unlikely (ret != ret_ok)) return ret_error;
        n += len;
    }
    else{
        ret = chula_buffer_add (string, buf->buf + n, len);
        if (unlikely (ret != ret_ok)) return ret_error;
        n += len;
    }

    /* Return */
    *consumed = (n - offset);
    return ret_ok;
}


static inline ret_t
parse_indexed (chula_buffer_t       *buf,
               unsigned int          offset,
               hpack_header_table_t *table,
               hpack_header_field_t *field,
               unsigned int         *consumed)
{
    ret_t                 ret;
    unsigned int          num;
    hpack_header_field_t *entry = NULL;
    unsigned int          con   = 0;
    int                   n     = offset;
/*
   Indexed Header Field
     0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+
   | 1 |        Index (7+)         |
   +---+---------------------------+
*/
    /* Read number */
    ret = integer_decode (7, (unsigned char *)buf->buf + n, buf->len - n, &num, &con);
    if (ret != ret_ok) return ret_error;

    /* Retrieve header table entry */
    ret = hpack_header_table_get (table, num, &entry);
    if ((ret != ret_ok) || (entry == NULL)) return ret;

    /* Copy */
    chula_buffer_add_buffer (&field->name, &entry->name);
    chula_buffer_add_buffer (&field->value, &entry->value);

    *consumed = con;
    return ret_ok;
}

static inline ret_t
parse_header_pair (chula_buffer_t       *buf,
                   unsigned int          offset,
                   hpack_header_table_t *table,
                   hpack_header_field_t *field,
                   unsigned int         *consumed)
{
    ret_t          ret;
    unsigned int   n    = offset;
    unsigned int   con  = 0;
    uint32_t       len  = 0;

/*
             With Indexing                       Without Indexing

     0   1   2   3   4   5   6   7        0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+    +---+---+---+---+---+---+---+---+
   | 0 | 0 |      Index (6+)       |    | 0 | 1 |      Index (6+)       |
   +---+---+---+-------------------+    +---+---+---+-------------------+
   | H |     Value Length (7+)     |    | H |     Value Length (7+)     |
   +-------------------------------+    +-------------------------------+
   | Value String (Length octets)  |    | Value String (Length octets)  |
   +-------------------------------+    +-------------------------------+

     0   1   2   3   4   5   6   7        0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+    +---+---+---+---+---+---+---+---+
   | 0 | 0 |           0           |    | 0 | 1 |           0           |
   +---+---+---+-------------------+    +---+---+---+-------------------+
   | H |     Value Length (7+)     |    | H |     Value Length (7+)     |
   +-------------------------------+    +-------------------------------+
   |  Name String (Length octets)  |    |  Name String (Length octets)  |
   +-------------------------------+    +-------------------------------+
   | H |     Value Length (7+)     |    | H |     Value Length (7+)     |
   +-------------------------------+    +-------------------------------+
   | Value String (Length octets)  |    | Value String (Length octets)  |
   +-------------------------------+    +-------------------------------+
*/
    /* Name
     */
    if (buf->buf[n] & 0x3F) {
        hpack_header_field_t *entry;

        ret = integer_decode (6, (unsigned char *)buf->buf+n, buf->len-n, &len, &con);
        if (unlikely (ret != ret_ok)) return ret_error;
        n += con;

        ret = hpack_header_table_get (table, len, &entry);
        if (ret != ret_ok) return ret;

        chula_buffer_add_buffer (&field->name, &entry->name);
    }
    else {
        n += 1;

        ret = parse_string (buf, n, &field->name, &con);
        if (ret != ret_ok) return ret;
        n += con;
    }

    /* Value
     */
    ret = parse_string (buf, n, &field->value, &con);
    if (ret != ret_ok) return ret;
    n += con;

    /* Return */
    *consumed = n - offset;
    return ret_ok;
}


static inline ret_t
parse_context_update (chula_buffer_t       *buf,
                      unsigned int          offset,
                      hpack_header_table_t *table,
                      unsigned int         *consumed)
{
    ret_t        ret;
    uint32_t     num;
    unsigned int con  = 0;
    int          n    = offset + 1;

    /* Empty */
    if ((uint8_t)buf->buf[n] == 0x80) {
        hpack_header_block_clean (&table->dynamic);
        *consumed = 2;
        return ret_ok;
    }

    /* Set new length */
    ret = integer_decode (7, (unsigned char *)buf->buf + n, buf->len - n, &num, &con);
    if (ret != ret_ok) return ret_error;

    ret = hpack_header_block_set_max (&table->dynamic, num);
    if (ret != ret_ok) return ret_error;

    *consumed = 1 + con;
    return ret_ok;
}

ret_t
hpack_header_parser_field (hpack_header_parser_t *parser,
                           chula_buffer_t        *buf,
                           unsigned int           offset,
                           hpack_header_field_t  *field,
                           unsigned int          *consumed)
{
    ret_t          ret;
    bool           skip_indexing;
    unsigned char  c              = buf->buf[offset];

    /* Parse field
     */
    if (c == 0x80) {
        /* Context update */
        ret = parse_context_update (buf, offset, &parser->table, consumed);
        return ret;
    }
    else if (c & 0x80u) {
        /* Indexed header field: 1st bit set */
        ret = parse_indexed (buf, offset, &parser->table, field, consumed);
        if (ret != ret_ok) return ret;
    }
    else {
        ret = parse_header_pair (buf, offset, &parser->table, field, consumed);
        if (ret != ret_ok) return ret;
    }

    /* Add to header table
     */
    skip_indexing = ((c & 0xc0) == 0x40u);
    if (!skip_indexing) {
        ret = hpack_header_table_add (&parser->table, field);
        if (ret != ret_ok) return ret;
    }

    return ret_ok;
}


ret_t
hpack_header_parser_all (hpack_header_parser_t *parser,
                         chula_buffer_t        *buf,
                         unsigned int           offset,
                         unsigned int          *consumed)
{
    ret_t                 ret;
    bool                  diff_enc;
    hpack_header_block_t *block     = &parser->table.dynamic;

    diff_enc = !hpack_header_block_is_empty (block);

    /* Parse raw header
     */
    while (true) {
        unsigned int         con   = 0;
        hpack_header_field_t field = HPACK_HDR_FLD_INIT;

        /* Parse a single header field */
        ret = hpack_header_parser_field (parser, buf, offset, &field, &con);
        if (ret != ret_ok) return ret;

        if (con > 0) {
            offset    += con;
            *consumed += con;
        }

        /* Emit
         */
        if ((parser->store) &&
            (! hpack_header_field_is_empty(&field)))
        {
            ret = hpack_header_store_emit (parser->store, &field);
            if (ret != ret_ok) return ret;
        }

        /* Exit */
        if (offset >= buf->len)
            break;
    }

    /* Differential encoding
     */
    if (diff_enc) {
        hpack_header_block_entry_t *entry;

        for (int32_t i=block->len-1; i >= 0 ; i--) {
            entry = &block->headers[i];

            if (entry->is_new)
                continue;

            if (parser->store) {
                ret = hpack_header_store_emit (parser->store, &entry->field);
                if (ret != ret_ok) return ret;
            }
        }
    }

    /*
     */
    for (int32_t i=block->len-1; i >= 0 ; i--) {
        if (block->headers[i].is_new) {
            block->headers[i].is_new = false;
        }
    }

    return ret_ok;
}
