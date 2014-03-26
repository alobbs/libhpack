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
#include <libhpack/header_field.h>

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
    /* 3C */ HDR_NIL("www-authenticate"),
    /* 3B */ HDR_NIL("via"),
    /* 3A */ HDR_NIL("vary"),
    /* 39 */ HDR_NIL("user-agent"),
    /* 38 */ HDR_NIL("transfer-encoding"),
    /* 37 */ HDR_NIL("strict-transport-security"),
    /* 36 */ HDR_NIL("set-cookie"),
    /* 35 */ HDR_NIL("server"),
    /* 34 */ HDR_NIL("retry-after"),
    /* 33 */ HDR_NIL("refresh"),
    /* 32 */ HDR_NIL("referer"),
    /* 31 */ HDR_NIL("range"),
    /* 30 */ HDR_NIL("proxy-authorization"),
    /* 2F */ HDR_NIL("proxy-authenticate"),
    /* 2E */ HDR_NIL("max-forwards"),
    /* 2D */ HDR_NIL("location"),
    /* 2C */ HDR_NIL("link"),
    /* 2B */ HDR_NIL("last-modified"),
    /* 2A */ HDR_NIL("if-unmodified-since"),
    /* 29 */ HDR_NIL("if-range"),
    /* 28 */ HDR_NIL("if-none-match"),
    /* 27 */ HDR_NIL("if-modified-since"),
    /* 26 */ HDR_NIL("if-match"),
    /* 25 */ HDR_NIL("host"),
    /* 24 */ HDR_NIL("from"),
    /* 23 */ HDR_NIL("expires"),
    /* 22 */ HDR_NIL("expect"),
    /* 21 */ HDR_NIL("etag"),
    /* 20 */ HDR_NIL("date"),
    /* 1F */ HDR_NIL("cookie"),
    /* 1E */ HDR_NIL("content-type"),
    /* 1D */ HDR_NIL("content-range"),
    /* 1C */ HDR_NIL("content-location"),
    /* 1B */ HDR_NIL("content-length"),
    /* 1A */ HDR_NIL("content-language"),
    /* 19 */ HDR_NIL("content-encoding"),
    /* 18 */ HDR_NIL("content-disposition"),
    /* 17 */ HDR_NIL("cache-control"),
    /* 16 */ HDR_NIL("authorization"),
    /* 15 */ HDR_NIL("allow"),
    /* 14 */ HDR_NIL("age"),
    /* 13 */ HDR_NIL("access-control-allow-origin"),
    /* 12 */ HDR_NIL("accept"),
    /* 11 */ HDR_NIL("accept-ranges"),
    /* 10 */ HDR_NIL("accept-language"),
    /* 0F */ HDR_NIL("accept-encoding"),
    /* 0E */ HDR_NIL("accept-charset"),
    /* 0D */ HDR_VAL(":status", "401"),
    /* 0C */ HDR_VAL(":status", "400"),
    /* 0B */ HDR_VAL(":status", "403"),
    /* 0A */ HDR_VAL(":status", "404"),
    /* 09 */ HDR_VAL(":status", "500"),
    /* 08 */ HDR_VAL(":status", "200"),
    /* 07 */ HDR_VAL(":scheme", "https"),
    /* 06 */ HDR_VAL(":scheme", "http"),
    /* 05 */ HDR_VAL(":path", "/index.html"),
    /* 04 */ HDR_VAL(":path", "/"),
    /* 03 */ HDR_VAL(":method", "POST"),
    /* 02 */ HDR_VAL(":method", "GET"),
    /* 01 */ HDR_NIL(":authority"),
};

static hpack_header_block_t static_block = {
    .size    = 0,
    .headers = static_table,
    .len     = (sizeof(static_table) / sizeof(static_table[0])),
    .max     = (sizeof(static_table) / sizeof(static_table[0])),
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
        for (uint32_t n=0; n<block->len; n++) {
            hpack_header_field_mrproper (&block->headers[n]);
        }
        free (block->headers);
        block->headers = NULL;
    }

    block->size = 0;
    block->len  = 0;
    return ret_ok;
}

ret_t
hpack_header_block_clean (hpack_header_block_t *block)
{
    if (block->headers != NULL) {
        for (uint32_t n=0; n<block->len; n++) {
            hpack_header_field_clean (&block->headers[n]);
        }
    }

    block->len = 0;
    return ret_ok;
}

static inline void
block_remove_n_headers (hpack_header_block_t *block, uint32_t to_remove)
{
    if (to_remove <= 0)
        return;

    memmove (block->headers, &block->headers[to_remove],
             (block->len - to_remove) * sizeof(hpack_header_field_t));

    block->len -= to_remove;
}


ret_t
hpack_header_block_set_max (hpack_header_block_t *block,
                            uint32_t              max)
{
    /* Increase */
    if (max > block->size) {
        block->headers = realloc (block->headers,
                                  (block->size + BLOCK_PTRS_INCREASE) * sizeof(hpack_header_field_t));
        if (unlikely (block->headers == NULL)) return ret_nomem;
        block->size +=  BLOCK_PTRS_INCREASE;
    }

    /* Decrease */
    else if (block->len > max) {
        block_remove_n_headers (block, block->len - max);
    }

    block->max = max;
    return ret_ok;
}

ret_t
hpack_header_block_add (hpack_header_block_t *block,
                        hpack_header_field_t *e)
{
    if (block->len >= block->max) {
        /* This is clearly suboptimal. [TODO]: Round FIFO */
        block_remove_n_headers (block, 1);
    }

    memcpy (&block->headers[block->len], e, sizeof(hpack_header_field_t));
    block->len++;

    return ret_ok;
}

ret_t
hpack_header_block_get (hpack_header_block_t  *block,
                        uint32_t               n,
                        hpack_header_field_t **e)
{
    if (n >= block->len) {
        return ret_not_found;
    }

    if (e != NULL) {
        *e = &block->headers[block->len - (n+1)];
    }

    return ret_ok;
}

bool
hpack_header_block_is_empty (hpack_header_block_t *block)
{
    return (block->len == 0);
}

void
hpack_header_block_repr (hpack_header_block_t *block,
                         chula_buffer_t       *output)
{
    uint32_t max_len = 0;

    chula_buffer_add_va  (output, "hpack_header_block@%x\n", POINTER_TO_INT(block));

    for (uint32_t i=0; i < block->len; i++) {
        max_len = MAX(block->headers[i].name.len, max_len);
    }

    for (int i=block->len-1; i >= 0 ; i--) {
        chula_buffer_add_va     (output, "  [%02d] ", block->len - i);
        chula_buffer_add_buffer (output, &block->headers[i].name);
        chula_buffer_add_str    (output, " ");
        chula_buffer_add_char_n (output, ' ', max_len - block->headers[i].name.len);
        chula_buffer_add_buffer (output, &block->headers[i].value);
        chula_buffer_add_str    (output, CRLF);
    }
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

    ret = hpack_header_block_set_max (&table->dynamic, BLOCK_PTRS_INCREASE);
    if (unlikely (ret != ret_ok)) return ret;

    /* Static block */
    memcpy (&table->statics, &static_block, sizeof(hpack_header_block_t));

    return ret_ok;
}

ret_t
hpack_header_table_mrproper (hpack_header_table_t *table)
{
    hpack_header_block_mrproper (&table->dynamic);
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
    if (n <= table->dynamic.len) {
        /* n:1 == pos:0 */
        return hpack_header_block_get (&table->dynamic, n-1, field);
    }

    uint32_t n_static = n - (table->dynamic.len + 1);
    return hpack_header_block_get (&table->statics, n_static, field);
}

void
hpack_header_table_repr (hpack_header_table_t *table,
                         chula_buffer_t       *output)
{
    hpack_header_block_repr (&table->dynamic, output);
}

ret_t
hpack_header_table_get_size (hpack_header_table_t *table,
                             uint64_t             *size)
{
    ret_t    ret;
    uint64_t total = 0;

    for (uint32_t n=0; n < table->dynamic.len; n++)
    {
        uint64_t              s = 0;
        hpack_header_field_t *e = NULL;

        ret = hpack_header_block_get (&table->dynamic, n, &e);
        if (unlikely (ret != ret_ok)) return ret;

        ret = hpack_header_field_get_size (e, &s);
        if (unlikely (ret != ret_ok)) return ret;

        total += s;
    }

    *size = total;
    return ret_ok;
}
