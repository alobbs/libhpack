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

#ifndef CHULA_CSTRINGS_H
#define CHULA_CSTRINGS_H

#if !defined(CHULA_H_INSIDE) && !defined (CHULA_COMPILATION)
# error "Only <libchula/libchula.h> can be included directly."
#endif

#include <stdlib.h>

char   *chula_strsep       (char **str, const char *delims);
size_t  chula_strlcat      (char *dst, const char *src, size_t siz);
char   *chula_strnstr      (const char *s, const char *find, size_t slen);
char   *chula_strcasestr   (const char *s, const char *find);
char   *chula_strncasestr  (const char *s, const char *find, size_t slen);
char   *chula_strncasestrn (const char *s, size_t slen, const char *find, size_t findlen);

/* Macros */
#define chula_strncasestrn_s(s,s_len,lit) chula_strncasestrn(s, s_len, lit, sizeof(lit)-1)
#define chula_strcasestrn_s(s,lit)        chula_strncasestrn(s, lit, sizeof(lit)-1)

#endif /* CHULA_CSTRINGS_H */
