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

#include "header_store.h"


typedef hpack_header_store_entry_t entry_t;

static ret_t
entry_new (entry_t **e)
{
    entry_t *obj;

    obj = (entry_t *) malloc (sizeof(entry_t));
    if (unlikely (obj == NULL)) return ret_nomem;

    INIT_LIST_HEAD (&obj->entry);
    hpack_header_field_init (&obj->field);

    *e = obj;
    return ret_ok;
}

static void
entry_free (entry_t *e)
{
    hpack_header_field_mrproper (&e->field);
    free (e);
}


static ret_t
add (hpack_header_store_t *store,
     hpack_header_field_t *field)
{
    ret_t    ret;
    entry_t *e;

    ret = entry_new (&e);
    if (unlikely (ret != ret_ok)) return ret;

    hpack_header_field_init (&e->field);
    hpack_header_field_copy (&e->field, field);

    chula_list_add_tail (&e->entry, &store->headers);
    return ret_ok;
}

static ret_t
emit (hpack_header_store_t *store,
      hpack_header_field_t *field)
{
    return add(store, field);
}

ret_t
hpack_header_store_emit (hpack_header_store_t *store,
                         hpack_header_field_t *field)
{
    return store->emit (store, field);
}

ret_t
hpack_header_store_add (hpack_header_store_t *store,
                        hpack_header_field_t *field)
{
    return add(store, field);
}


ret_t
hpack_header_store_init (hpack_header_store_t *store)
{
    INIT_LIST_HEAD (&store->headers);
    store->emit = emit;
    return ret_ok;
}


ret_t
hpack_header_store_mrproper (hpack_header_store_t *store)
{
    chula_list_t *i, *tmp;

    list_for_each_safe (i, tmp, &store->headers) {
        entry_t *e = list_entry(i, entry_t, entry);
        entry_free(e);
    }

    return ret_ok;
}


ret_t
hpack_header_store_get_n (hpack_header_store_t  *store,
                          uint32_t               num,
                          hpack_header_field_t **field)
{
    hpack_header_store_entry_t *i;

    list_for_each_entry (i, &store->headers, entry) {
        if (!--num) {
            *field = &i->field;
            return ret_ok;
        }
    }
    return ret_not_found;
}


void
hpack_header_store_repr (hpack_header_store_t *store,
                         chula_buffer_t       *buf)
{
    hpack_header_store_entry_t *i;
    uint32_t                    max_len = 0;

    chula_buffer_add_va (buf, "hpack_header_store@%x\n", POINTER_TO_INT(store));

    /* Find max header name length */
    list_for_each_entry (i, &store->headers, entry) {
        max_len = MAX(i->field.name.len, max_len);
    }

    /* Build representation */
    list_for_each_entry (i, &store->headers, entry) {
        chula_buffer_add_str    (buf, "  ");
        chula_buffer_add_buffer (buf, &i->field.name);
        chula_buffer_add_str    (buf, ": ");
        chula_buffer_add_char_n (buf, ' ', max_len - i->field.name.len);
        chula_buffer_add_buffer (buf, &i->field.value);
        chula_buffer_add_str    (buf, CRLF);
    }
}
