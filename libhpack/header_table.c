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

#include <libhpack/macros.h>
#include <libhpack/header_table.h>

/** Static Table Entry without a value
 */
#define HDR_NIL(s)                            \
    {   .name  = CHULA_BUF_INIT_FAKE(s),      \
        .value = CHULA_BUF_INIT,              \
    }

/** Static Table Entry with a value
 */
#define HDR_VAL(n,v)                         \
    {   .name  = CHULA_BUF_INIT_FAKE(n),     \
        .value = CHULA_BUF_INIT_FAKE(v),     \
    }

/** Static Table (B Appendix)
 *
 * The static table consists of an unchangeable ordered list of (name,
 * value) pairs. The first entry in the table is always represented by
 * the index len(header table)+1, and the last entry in the table is
 * represented by the index len(header table)+len(static table).
 */
static hpack_header_field_t static_table[] = {
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

static hpack_header_block_t static_block = {
    .size    = 0,
    .headers = (hpack_header_field_t **)&static_table,
    .len     = (sizeof(static_table) / sizeof(static_table[0])),
    .max     = (sizeof(static_table) / sizeof(static_table[0])) + 1,
};

/* Block
 */

#define BLOCK_PTRS_INCREASE 20

ret_t
hpack_header_block_init (hpack_header_block_t *block)
{
    block->headers = NULL;
    block->len     = 0;
    block->size    = 0;
    block->max     = 0;

    return ret_ok;
}

ret_t
hpack_header_block_mrproper (hpack_header_block_t *block)
{
    if (block->headers != NULL) {
        free (block->headers);
        block->headers = NULL;
    }

    block->size = 0;
    block->len  = 0;

    return ret_ok;
}

ret_t
hpack_header_block_set_max (hpack_header_block_t *block,
                            uint32_t              max)
{
    if (max < block->len) {
        /* TODO */
    }

    block->max = max;
    return ret_ok;
}

ret_t
hpack_header_block_add (hpack_header_block_t *block,
                        hpack_header_field_t *e)
{
    if (block->size - block->len <= 0) {
        block->headers = realloc (block->headers, sizeof(void *) * (block->size +  BLOCK_PTRS_INCREASE));
        if (unlikely (block->headers == NULL)) return ret_nomem;
        block->size +=  BLOCK_PTRS_INCREASE;
    }

    block->headers[block->len] = e;
    block->len++;

    return ret_ok;
}

ret_t
hpack_header_block_get (hpack_header_block_t  *block,
                        uint32_t               n,
                        hpack_header_field_t **e)
{
    if (n <= block->len) {
        return ret_not_found;
    }

    if (e != NULL) {
        *e = block->headers[n-1];
    }

    return ret_ok;
}


/* Table
 */

ret_t
hpack_header_table_init (hpack_header_table_t *table)
{
    ret_t ret;

    /* Dynamic block */
    ret = hpack_header_block_init (&table->dynamic);
    if (unlikely (ret != ret_ok)) return ret;

    /* Static block */
    memcpy (&table->statics, &static_block, sizeof(hpack_header_block_t));

    return ret_ok;
}

ret_t
hpack_header_table_mrproper (hpack_header_table_t *table)
{
    hpack_header_block_mrproper (&table->dynamic);
    hpack_header_block_mrproper (&table->statics);
    return ret_ok;
}

ret_t
hpack_header_table_add (hpack_header_table_t *table,
                        hpack_header_field_t *field)
{
    return hpack_header_block_add (&table->dynamic, field);
}

ret_t
hpack_header_table_get (hpack_header_table_t  *table,
                        uint32_t               n,
                        hpack_header_field_t **field)
{
    if (n < table->dynamic.len) {
        return hpack_header_block_get (&table->dynamic, n, field);
    }

    uint32_t n_static = n - table->dynamic.len;
    return hpack_header_block_get (&table->statics, n_static, field);
}
