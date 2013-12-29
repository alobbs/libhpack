/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* All files in chula are Copyright (C) 2012 Alvaro Lopez Ortega.
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

#ifndef CHULA_COMMON_H
#define CHULA_COMMON_H

#include "macros.h"

typedef enum {
	ret_no_sys          = -4,
	ret_nomem           = -3,
	ret_deny            = -2,
	ret_error           = -1,
	ret_ok              =  0,
	ret_eof             =  1,
	ret_eof_have_data   =  2,
	ret_not_found       =  3,
	ret_file_not_found  =  4,
	ret_eagain          =  5,
	ret_ok_and_sent     =  8
} ret_t;

typedef unsigned int        crc_t;

typedef int                 cint_t;
typedef char                cchar_t;
typedef short               cshort_t;
typedef long                clong_t;
typedef long long           cllong_t;

typedef unsigned int        cuint_t;
typedef unsigned char       cuchar_t;
typedef unsigned short      cushort_t;
typedef unsigned long       culong_t;
typedef unsigned long long  cullong_t;

typedef void (*chula_func_free_t) (void *);

#endif /* CHULA_COMMON_H */
