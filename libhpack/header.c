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

/** Header Entry
 */
typedef struct {
    chula_buffer_t name;
    chula_buffer_t value;
} hpack_header_entry_static_t;

/** Static Table Entry without a value
 */
#define HDR_NIL(n)                      \
    { .name  = CHULA_BUF_INIT_FAKE(n),  \
      .value = CHULA_BUF_INIT           \
    }

/** Static Table Entry with a value
 */
#define HDR_VAL(n,v)                    \
    {  .name  = CHULA_BUF_INIT_FAKE(n), \
       .value = CHULA_BUF_INIT_FAKE(v)  \
    }

/** Static Table (B Appendix)
 *
 * The static table consists of an unchangeable ordered list of (name,
 * value) pairs. The first entry in the table is always represented by
 * the index len(header table)+1, and the last entry in the table is
 * represented by the index len(header table)+len(static table).
 */
static hpack_header_entry_static_t static_table[] = {
    /* 01 */ HDR_NIL(":authority"),
    /* 02 */ HDR_VAL(":method", "GET"),
    /* 03 */ HDR_VAL(":method", "POST"),
    /* 04 */ HDR_VAL(":path", "/"),
    /* 05 */ HDR_VAL(":path", "/index.html"),
    /* 06 */ HDR_VAL(":scheme", "http"),
    /* 07 */ HDR_VAL(":scheme", "https"),
    /* 08 */ HDR_VAL(":status", "200"),
    /* 09 */ HDR_VAL(":status", "500"),
    /* 0A */ HDR_VAL(":status", "404"),
    /* 0B */ HDR_VAL(":status", "403"),
    /* 0C */ HDR_VAL(":status", "400"),
    /* 0D */ HDR_VAL(":status", "401"),
    /* 0E */ HDR_NIL("accept-charset"),
    /* 0F */ HDR_NIL("accept-encoding"),
    /* 10 */ HDR_NIL("accept-language"),
    /* 11 */ HDR_NIL("accept-ranges"),
    /* 12 */ HDR_NIL("accept"),
    /* 13 */ HDR_NIL("access-control-allow-origin"),
    /* 14 */ HDR_NIL("age"),
    /* 15 */ HDR_NIL("allow"),
    /* 16 */ HDR_NIL("authorization"),
    /* 17 */ HDR_NIL("cache-control"),
    /* 18 */ HDR_NIL("content-disposition"),
    /* 19 */ HDR_NIL("content-encoding"),
    /* 1A */ HDR_NIL("content-language"),
    /* 1B */ HDR_NIL("content-length"),
    /* 1C */ HDR_NIL("content-location"),
    /* 1D */ HDR_NIL("content-range"),
    /* 1E */ HDR_NIL("content-type"),
    /* 1F */ HDR_NIL("cookie"),
    /* 20 */ HDR_NIL("date"),
    /* 21 */ HDR_NIL("etag"),
    /* 22 */ HDR_NIL("expect"),
    /* 23 */ HDR_NIL("expires"),
    /* 24 */ HDR_NIL("from"),
    /* 25 */ HDR_NIL("host"),
    /* 26 */ HDR_NIL("if-match"),
    /* 27 */ HDR_NIL("if-modified-since"),
    /* 28 */ HDR_NIL("if-none-match"),
    /* 29 */ HDR_NIL("if-range"),
    /* 2A */ HDR_NIL("if-unmodified-since"),
    /* 2B */ HDR_NIL("last-modified"),
    /* 2C */ HDR_NIL("link"),
    /* 2D */ HDR_NIL("location"),
    /* 2E */ HDR_NIL("max-forwards"),
    /* 2F */ HDR_NIL("proxy-authenticate"),
    /* 30 */ HDR_NIL("proxy-authorization"),
    /* 31 */ HDR_NIL("range"),
    /* 32 */ HDR_NIL("referer"),
    /* 33 */ HDR_NIL("refresh"),
    /* 34 */ HDR_NIL("retry-after"),
    /* 35 */ HDR_NIL("server"),
    /* 36 */ HDR_NIL("set-cookie"),
    /* 37 */ HDR_NIL("strict-transport-security"),
    /* 38 */ HDR_NIL("transfer-encoding"),
    /* 39 */ HDR_NIL("user-agent"),
    /* 3A */ HDR_NIL("vary"),
    /* 3B */ HDR_NIL("via"),
    /* 3C */ HDR_NIL("www-authenticate"),
};


/** Length of the Static Table
 */
static const size_t static_table_len = sizeof(static_table) / sizeof(static_table[0]);

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


/** TMP
 */
ret_t
test (chula_buffer_t *header_name)
{
    hpack_header_entry_static_t *entry;
    int                          found = -1;

    /* Deflate table
     */

    /* Static table
     */
    for (int i=0; i < static_table_len; i++) {
        entry = &static_table[0];
        if (chula_buffer_cmp_buf (&entry->name, header_name) == 0) {
            found = i;
            break;
        }
    }

    return ret_ok;
}


static ret_t
parse_string (chula_buffer_t       *buf,
              unsigned int          offset,
              chula_buffer_t       *string,
              unsigned int         *consumed)
{
    ret_t          ret;
    int            n         = offset;
    int            con       = 0;
    int            len       = 0;

/*
     0   1   2   3   4   5   6   7
   +-------------------------------+
   |       Name Length (8+)        |
   +-------------------------------+
   |  Name String (Length octets)  |
   +-------------------------------+
*/
    /* Name */
    ret = integer_decode (8, buf->buf + n, buf->len - n, &len, &con);
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
               hpack_header_field_t *field,
               unsigned int         *consumed)
{
    ret_t ret;
    int   num;
    int   con  = 0;
    int   n    = offset;

/*
   Indexed Header Field
     0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+
   | 1 |        Index (7+)         |
   +---+---------------------------+
*/
    ret = integer_decode (7, buf->buf + n, buf->len - n, &num, &con);
    if (ret != ret_ok) return ret_error;

    hpack_header_entry_static_t *entry = &static_table[num-1];
    chula_buffer_add_buffer (&field->name, &entry->name);
    chula_buffer_add_buffer (&field->value, &entry->value);

    *consumed = con;
    return ret_ok;
}

static inline ret_t
parse_header_pair (chula_buffer_t       *buf,
                   unsigned int          offset,
                   hpack_header_field_t *field,
                   unsigned int         *consumed)
{
    ret_t ret;
    int   n    = offset;
    int   con  = 0;
    int   len  = 0;

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
        ret = integer_decode (6, buf->buf+n, buf->len-n, &len, &con);
        if (unlikely (ret != ret_ok)) return ret_error;
        n += con;

        hpack_header_entry_static_t *entry = &static_table[len-1];
        chula_buffer_add_buffer (&field->name, &entry->name);

        printf ("parse_indexed: num %d -> %s : %s\n", len, entry->name.buf, entry->value.buf);
    }
    else {
        n += 1;

        ret = parse_string (buf, n, &field->name, &con);
        if (ret != ret_ok) return ret;
        n += con;

        printf ("Header name: %s\n", field->name.buf);
    }

    /* Value
     */
    ret = parse_string (buf, n, &field->value, &con);
    if (ret != ret_ok) return ret;
    n += con;

    printf ("       value: %s\n", field->value.buf);

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
        return parse_indexed (buf, offset, field, consumed);
    }

    ret = parse_header_pair (buf, offset, field, consumed);
    if (ret != ret_ok)
        return ret;

    bool skip_indexing = ((c & 0xc0) == 0x40u);
    if (!skip_indexing) {
        ret = hpack_header_table_add (table, field);
        if (ret != ret_ok) return ret;
    }
}
