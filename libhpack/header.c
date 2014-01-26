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


/*
     0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+
   | 0 | 1 |      Index (6+)       |
   +---+---+---+-------------------+
   |       Value Length (8+)       |
   +-------------------------------+
   | Value String (Length octets)  |
   +-------------------------------+

           Literal Header Field without Indexing - Indexed Name


     0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+
   | 0 | 1 |           0           |
   +---+---+---+-------------------+
   |       Name Length (8+)        |
   +-------------------------------+
   |  Name String (Length octets)  |
   +-------------------------------+
   |       Value Length (8+)       |
   +-------------------------------+
   | Value String (Length octets)  |
   +-------------------------------+

             Literal Header Field without Indexing - New Name

   00                                      | == Literal indexed ==
   0a                                      |   Literal name (len = 10)
   6375 7374 6f6d 2d6b 6579                | custom-key
   0d                                      |   Literal value (len = 13)
   6375 7374 6f6d 2d68 6561 6465 72        | custom-header
                                           | -> custom-key: custom-header

------------------

   44                                      | == Literal not indexed ==
                                           |   Indexed name (idx = 4) :path
   0c                                      |   Literal value (len = 12)
   2f73 616d 706c 652f 7061 7468           | /sample/path
                                           | -> :path: /sample/path

   Header table (after decoding): empty.

------------------

*/

static inline ret_t
parse_indexed (chula_buffer_t *buf,
               unsigned int    offset,
               unsigned int   *consumed)

{
    ret_t ret;
    int   num;
    int   con  = 0;
    int   n    = offset;

/*
     0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+
   | 1 |        Index (7+)         |
   +---+---------------------------+
*/

    ret = integer_decode (7, buf->buf + n, buf->len - n, &num, &con);
    if (ret != ret_ok) return ret_error;

    hpack_header_entry_static_t *entry = &static_table[num-1];

    printf ("parse_indexed: num %d -> %s : %s\n", num, entry->name.buf, entry->value.buf);

    *consumed = offset + con;
    return ret_ok;
}

static inline ret_t
parse_literal_new (chula_buffer_t *buf,
                   unsigned int    offset,
                   unsigned int   *consumed)
{
    ret_t          ret;
    int            n         = offset;
    int            con       = 0;
    int            len_name  = 0;
    int            len_value = 0;
    chula_buffer_t name      = CHULA_BUF_INIT;
    chula_buffer_t value     = CHULA_BUF_INIT;

/*
   Literal Header Field with Incremental Indexing - New Name

     0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+
   | 0 | 0 |           0           |
   +---+---+---+-------------------+
   |       Name Length (8+)        |
   +-------------------------------+
   |  Name String (Length octets)  |
   +-------------------------------+
   |       Value Length (8+)       |
   +-------------------------------+
   | Value String (Length octets)  |
   +-------------------------------+
*/

    n += 1;

    /* Name
     */
    ret = integer_decode (8, buf->buf + n, buf->len-n, &len_name, &con);
    if (ret != ret_ok) return ret_error;

    n += con;
    ret = chula_buffer_add (&name, buf->buf + n, len_name);
    if (ret != ret_ok) return ret_error;

    /* Value
     */
    n+= len_name;
    ret = integer_decode (8, buf->buf + n, buf->len-n, &len_value, &con);
    if (ret != ret_ok) return ret_error;

    n += con;
    ret = chula_buffer_add (&value, buf->buf + n, len_value);
    if (ret != ret_ok) return ret_error;

    *consumed = n + len_value;
    return ret_ok;
}

static inline ret_t
parse_literal_indexed (chula_buffer_t *buf,
                       unsigned int    offset,
                       unsigned int   *consumed)
{
    ret_t          ret;
    int            num;
    int            n         = offset;
    int            con       = 0;
    int            len_value = 0;
    chula_buffer_t value     = CHULA_BUF_INIT;

/*
  Literal Header Field without Indexing - Indexed Name

     0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+
   | 0 | 1 |      Index (6+)       |
   +---+---+---+-------------------+
   |       Value Length (8+)       |
   +-------------------------------+
   | Value String (Length octets)  |
   +-------------------------------+
*/

    /* Name
     */
    ret = integer_decode (6, buf->buf + n, buf->len - n, &num, &con);
    if (ret != ret_ok) return ret_error;

    hpack_header_entry_static_t *entry = &static_table[num];

    /* Value
     */
    n = con;
    ret = integer_decode (8, buf->buf + n, buf->len-n, &len_value, &con);
    if (ret != ret_ok) return ret_error;

    n += con;
    ret = chula_buffer_add (&value, buf->buf + n, len_value);
    if (ret != ret_ok) return ret_error;

    printf ("%s => %s\n", entry->name.buf, value.buf);

    *consumed = n +len_value;
    return ret_ok;
}


ret_t
hpack_header_parse (chula_buffer_t *buf,
                    unsigned int    offset,
                    unsigned int   *consumed)
{
    char c = buf->buf[0];

    /* Indexed Header Repr */
    if (c & 0x80u) {
        return parse_indexed (buf, offset, consumed);
    }

    /* Literal Header Repr - New Name */
    if ((c == 0x40u) || (c == 0)) {
        return parse_literal_new (buf, offset, consumed);
    }

    /* Literal Header Repr - Indexed Name */
    return parse_literal_indexed (buf, offset, consumed);
}
