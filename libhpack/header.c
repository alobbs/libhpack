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

#include <libchula/list.h>
#include <libchula/buffer.h>

#include "header.h"
#include "integer.h"


static ret_t
build_string_literal (chula_buffer_t *buf, bool huffman)
{
    /* 4.1.2 String Literal Representation */

    ret_t         ret;
    unsigned char extra    = 0;
    unsigned char prefix[] = {0, 0, 0, 0, 0, 0};

    /* Encode length */
    ret = integer_encode (7, buf->len, prefix, &extra);
    if (unlikely (ret != ret_ok))
        return ret;

    /* Set first bit */
    if (huffman) {
        prefix[0] |= (1 << 7);
    }

    /* Prepend header */
    ret = chula_buffer_prepend (buf, (char *)prefix, extra);
    if (unlikely (ret != ret_ok))
        return ret;

    return ret_ok;
}


static ret_t
parse_string (chula_buffer_t       *buf,
              unsigned int          offset,
              chula_buffer_t       *string,
              unsigned int         *consumed)
{
    ret_t        ret;
    int          n    = offset;
    unsigned int con  = 0;
    int          len  = 0;

/*
     0   1   2   3   4   5   6   7
   +-------------------------------+
   |       Name Length (8+)        |
   +-------------------------------+
   |  Name String (Length octets)  |
   +-------------------------------+
*/
    /* Name */
    ret = integer_decode (8, (unsigned char*)buf->buf + n, buf->len - n, &len, &con);
    if (unlikely (ret != ret_ok)) return ret_error;
    n += con;

    ret = chula_buffer_add (string, buf->buf + n, len);
    if (unlikely (ret != ret_ok)) return ret_error;
    n += len;

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
    int                   num;
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
    int            n    = offset;
    unsigned int   con  = 0;
    int            len  = 0;

/*
             With Indexing                       Without Indexing
     0   1   2   3   4   5   6   7        0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+    +---+---+---+---+---+---+---+---+
   | 0 | 0 |           0           |    | 0 | 1 |           0           |
   +---+---+---+-------------------+    +---+---+---+-------------------+
   |       Name Length (8+)        |    |       Name Length (8+)        |
   +-------------------------------+    +-------------------------------+
   |  Name String (Length octets)  |    |  Name String (Length octets)  |
   +-------------------------------+    +-------------------------------+
   |       Value Length (8+)       |    |       Value Length (8+)       |
   +-------------------------------+    +-------------------------------+
   | Value String (Length octets)  |    | Value String (Length octets)  |
   +-------------------------------+    +-------------------------------+

     0   1   2   3   4   5   6   7        0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+    +---+---+---+---+---+---+---+---+
   | 0 | 0 |      Index (6+)       |    | 0 | 1 |      Index (6+)       |
   +---+---+---+-------------------+    +---+---+---+-------------------+
   |       Value Length (8+)       |    |       Value Length (8+)       |
   +-------------------------------+    +-------------------------------+
   | Value String (Length octets)  |    | Value String (Length octets)  |
   +-------------------------------+    +-------------------------------+
*/
    /* Name
     */
    if (buf->buf[n] & 0x3Fu) {
        hpack_header_field_t *entry;

        ret = integer_decode (6, (unsigned char *)buf->buf+n, buf->len-n, &len, &con);
        if (unlikely (ret != ret_ok)) return ret_error;
        n += con;

        ret = hpack_header_table_get (table, len, &entry);
        if (ret != ret_ok) return ret;

        chula_buffer_add_buffer (&field->name, &entry->name);

//        hpack_header_table_entry_t *entry = &static_table[len-1];
//        printf ("parse_indexed: num %d -> %s : %s\n", len, entry->name.buf, entry->value.buf);
    }
    else {
        n += 1;

        ret = parse_string (buf, n, &field->name, &con);
        if (ret != ret_ok) return ret;
        n += con;

//        printf ("Header name: %s\n", field->name.buf);
    }

    /* Value
     */
    ret = parse_string (buf, n, &field->value, &con);
    if (ret != ret_ok) return ret;
    n += con;

//    printf ("       value: %s\n", field->value.buf);

    /* Return */
    *consumed = n - offset;
    return ret_ok;
}

ret_t
hpack_header_field_parse (chula_buffer_t       *buf,
                          unsigned int          offset,
                          hpack_header_table_t *table,
                          hpack_header_field_t *field,
                          unsigned int         *consumed)
{
    ret_t ret;
    char  c    = buf->buf[offset];

    /* Indexed header field */
    if (c & 0x80u) {
        /* 1st bit set */
        return parse_indexed (buf, offset, table, field, consumed);
    }

    ret = parse_header_pair (buf, offset, table, field, consumed);
    if (ret != ret_ok)
        return ret;

    printf ("%s: %s\n", field->name.buf, field->value.buf);

    bool skip_indexing = ((c & 0xc0) == 0x40u);
    if (!skip_indexing) {
        ret = hpack_header_table_add (table, field);
        if (ret != ret_ok) return ret;
    }

    return ret_ok;
}
