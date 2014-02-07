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

/* Entry
 */

ret_t
hpack_header_table_entry_new (hpack_header_table_entry_t **entry)
{
    HPACK_NEW_STRUCT (n, header_table_entry);

    INIT_LIST_HEAD (&n->entry);
    hpack_header_field_init (&n->field);

    *entry = n;
    return ret_ok;
}

ret_t
hpack_header_table_entry_free (hpack_header_table_entry_t *entry)
{
    chula_list_del (&entry->entry);
    hpack_header_field_mrproper (&entry->field);

    free(entry);
    return ret_ok;
}


/* Table
 */

ret_t
hpack_header_table_init (hpack_header_table_t *table)
{
    INIT_LIST_HEAD (&table->dynamic);
    table->dynamic_len = 0;
    table->dynamic_max = 50;
    return ret_ok;
}

ret_t
hpack_header_table_mrproper (hpack_header_table_t *table)
{
    chula_list_t *i, *tmp;

    list_for_each_safe (i, tmp, &table->dynamic) {
        hpack_header_table_entry_free (HDR_TABLE_ENTRY(i));
    }

    return ret_ok;
}

ret_t
hpack_header_table_add (hpack_header_table_t *table,
                        hpack_header_field_t *field)
{
    HPACK_NEW_STRUCT (te, header_table_entry);

    /* Copy the header_field to a table_entry */
    memcpy (&te->field, field, sizeof(hpack_header_field_t));

    /* Add it to the list */
    chula_list_add (&te->entry, &table->dynamic);

    return ret_ok;
}

ret_t
hpack_header_table_set_size (hpack_header_table_t *table,
                             size_t                size)
{
    if (size < table->dynamic_max) {
        ;
    }

    table->dynamic_max = size;
    return ret_ok;
}
