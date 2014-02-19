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

#include "header_field.h"

ret_t
hpack_header_field_init (hpack_header_field_t *header)
{
    chula_buffer_init (&header->name);
    chula_buffer_init (&header->value);
    return ret_ok;
}


ret_t
hpack_header_field_clean (hpack_header_field_t *header)
{
    chula_buffer_clean (&header->name);
    chula_buffer_clean (&header->value);

    if (header->name.buf != NULL) {
        header->name.buf[0] = '\0';
    }
    if (header->value.buf != NULL) {
        header->value.buf[0] = '\0';
    }

    return ret_ok;
}


ret_t
hpack_header_field_mrproper (hpack_header_field_t *header)
{
    chula_buffer_mrproper (&header->name);
    chula_buffer_mrproper (&header->value);
    return ret_ok;
}


ret_t
hpack_header_field_copy (hpack_header_field_t *header,
                         hpack_header_field_t *tocopy)
{
    chula_buffer_add_buffer (&header->name, &tocopy->name);
    chula_buffer_add_buffer (&header->value, &tocopy->value);
    return ret_ok;
}


ret_t
hpack_header_field_repr (hpack_header_field_t *header,
                         chula_buffer_t       *output)
{
    chula_buffer_add_va     (output, "hpack_header_field@%x - ", POINTER_TO_INT(header));
    chula_buffer_add_buffer (output, &header->name);
    chula_buffer_add_str    (output, ": ");
    chula_buffer_add_buffer (output, &header->value);
    chula_buffer_add_str    (output, "\n");

    return ret_ok;
}
