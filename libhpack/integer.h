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

#ifndef LIBHPACK_INTEGER_H
#define LIBHPACK_INTEGER_H

#if !defined(HPACK_H_INSIDE) && !defined (HPACK_COMPILATION)
# error "Only <libhpack/libhpack.h> can be included directly."
#endif

#include <limits.h>
#include <libchula/libchula.h>

#define VLQ_MAX_LEN_INTEGER (SIZEOF_INT + 2)

ret_t
hpack_integer_encode (int            N,        /* Prefix length in bits  */
                      unsigned int  value,     /* Number to encode       */
                      unsigned char *mem,      /* Memory to encode it to */
                      unsigned char *mem_len); /* Memory used            */

ret_t
hpack_integer_decode (int            N,         /* Prefix length in bits  */
                      unsigned char *mem,       /* Memory to read         */
                      unsigned char  mem_len,   /* Length of the memory   */
                      unsigned int  *ret,       /* Value return           */
                      unsigned int  *consumed); /* Length of encoded num  */

#endif /* LIBHPACK_INTEGER_H */
