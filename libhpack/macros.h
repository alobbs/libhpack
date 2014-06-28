/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/* All files in libchula are Copyright (C) 2014 Alvaro Lopez Ortega.
 *
 *   Authors:
 *     * Alvaro Lopez Ortega <alvaro@alobbs.com>
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

#ifndef HPACK_MACROS_H
#define HPACK_MACROS_H

#if !defined(HPACK_H_INSIDE) && !defined (HPACK_COMPILATION)
# error "Only <libhpack/libhpack.h> can be included directly."
#endif

#include <libchula/libchula.h>

#define HPACK_NEW(obj,type)        CHULA_GEN_NEW(hpack,obj,type)
#define HPACK_NEW_STRUCT(obj,type) CHULA_GEN_NEW_STRUCT(hpack,obj,type)

#define HPACK_ADD_FUNC_NEW(klass)  CHULA_GEN_ADD_FUNC_NEW(hpack,klass)
#define HPACK_ADD_FUNC_FREE(klass) CHULA_GEN_ADD_FUNC_FREE(hpack,klass)

#define HPACK_NEW_OBJ(klass,...)   CHULA_GEN_NEW_OBJ(hpack,klass,##__VA_ARGS__)

#define SETTINGS_HEADER_TABLE_SIZE       4096

/* By doing this we can optimize the circular buffer calculations of
 * the headers table. We'll have space for 127 entries instead of 125.
 */
#define HPACK_MAX_HEADER_TABLE_ENTRIES   128
#define HPACK_CB_HEADER_OFFSETS_MASK    (HPACK_MAX_HEADER_TABLE_ENTRIES - 1)
#define HPACK_CB_HEADER_DATA_SIZE        SETTINGS_HEADER_TABLE_SIZE
#define HPACK_CB_HEADER_DATA_MASK       (HPACK_CB_HEADER_DATA_SIZE -1)

#endif /* HPACK_MACROS_H */
