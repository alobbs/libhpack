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
    HDR_NIL(":authority"),
    HDR_VAL(":method", "GET"),
    HDR_VAL(":method", "POST"),
    HDR_VAL(":path", "/"),
    HDR_VAL(":path", "/index.html"),
    HDR_VAL(":scheme", "http"),
    HDR_VAL(":scheme", "https"),
    HDR_VAL(":status", "200"),
    HDR_VAL(":status", "500"),
    HDR_VAL(":status", "404"),
    HDR_VAL(":status", "403"),
    HDR_VAL(":status", "400"),
    HDR_VAL(":status", "401"),
    HDR_NIL("accept-charset"),
    HDR_NIL("accept-encoding"),
    HDR_NIL("accept-language"),
    HDR_NIL("accept-ranges"),
    HDR_NIL("accept"),
    HDR_NIL("access-control-allow-origin"),
    HDR_NIL("age"),
    HDR_NIL("allow"),
    HDR_NIL("authorization"),
    HDR_NIL("cache-control"),
    HDR_NIL("content-disposition"),
    HDR_NIL("content-encoding"),
    HDR_NIL("content-language"),
    HDR_NIL("content-length"),
    HDR_NIL("content-location"),
    HDR_NIL("content-range"),
    HDR_NIL("content-type"),
    HDR_NIL("cookie"),
    HDR_NIL("date"),
    HDR_NIL("etag"),
    HDR_NIL("expect"),
    HDR_NIL("expires"),
    HDR_NIL("from"),
    HDR_NIL("host"),
    HDR_NIL("if-match"),
    HDR_NIL("if-modified-since"),
    HDR_NIL("if-none-match"),
    HDR_NIL("if-range"),
    HDR_NIL("if-unmodified-since"),
    HDR_NIL("last-modified"),
    HDR_NIL("link"),
    HDR_NIL("location"),
    HDR_NIL("max-forwards"),
    HDR_NIL("proxy-authenticate"),
    HDR_NIL("proxy-authorization"),
    HDR_NIL("range"),
    HDR_NIL("referer"),
    HDR_NIL("refresh"),
    HDR_NIL("retry-after"),
    HDR_NIL("server"),
    HDR_NIL("set-cookie"),
    HDR_NIL("strict-transport-security"),
    HDR_NIL("transfer-encoding"),
    HDR_NIL("user-agent"),
    HDR_NIL("vary"),
    HDR_NIL("via"),
    HDR_NIL("www-authenticate"),
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
