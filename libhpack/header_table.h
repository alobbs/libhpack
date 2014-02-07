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

#ifndef LIBHPACK_HEADER_TABLE_H
#define LIBHPACK_HEADER_TABLE_H

#include <libchula/list.h>
#include <libchula/buffer.h>
#include "libhpack/header_field.h"

typedef struct {
    chula_list_t         entry;
    hpack_header_field_t field;
} hpack_header_table_entry_t;

#define HDR_TABLE_ENTRY(e) ((hpack_header_table_entry_t*)(e))

ret_t hpack_header_table_entry_new  (hpack_header_table_entry_t **entry);
ret_t hpack_header_table_entry_free (hpack_header_table_entry_t  *entry);


typedef struct {
    chula_list_t dynamic;
    uint32_t     dynamic_len;
    uint32_t     dynamic_max;
} hpack_header_table_t;

#define HDR_TABLE(t) ((hpack_header_table_t*)(t))

ret_t hpack_header_table_init     (hpack_header_table_t *table);
ret_t hpack_header_table_mrproper (hpack_header_table_t *table);

ret_t hpack_header_table_set_size (hpack_header_table_t *table, size_t size);
ret_t hpack_header_table_add      (hpack_header_table_t *table, hpack_header_field_t *field);

#endif /* LIBHPACK_HEADER_TABLE_H */
