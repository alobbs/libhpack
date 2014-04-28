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

#ifndef CHULA_LOG_H
#define CHULA_LOG_H

#if !defined(CHULA_H_INSIDE) && !defined (CHULA_COMPILATION)
# error "Only <libchula/libchula.h> can be included directly."
#endif

#include <libchula/buffer.h>

typedef enum {
    CHULA_LOG_ERROR,
    CHULA_LOG_WARN,
    CHULA_LOG_INFO,
    CHULA_LOG_DEBUG,
    CHULA_LOG_TRACE
} chula_log_level_t;

typedef struct {
    ret_t (*log)  (void *self, chula_log_level_t level, chula_buffer_t *buf);
    void  (*free) (void *self);
} chula_log_base_t;

#define HPACK_LOG(l) ((chula_log_base_t*)(l))

/* Init & Shutdown */
ret_t chula_log_init     (chula_log_base_t *log);
void  chula_log_shutdown (void);

/* Regular logging */
void  chula_log_error (const char *format, ...);
void  chula_log_warn  (const char *format, ...);
void  chula_log_info  (const char *format, ...);
void  chula_log_debug (const char *format, ...);
void  chula_log_trace (const char *format, ...);

/* Utilities */
void  chula_log_errno (int errorno, chula_log_level_t level, const char *format, ...);

/* Internal */
void chula_log_buf           (chula_log_level_t level, chula_buffer_t *buf);
void chula_log_get_level_buf (chula_log_level_t level, chula_buffer_t *buf);

#endif /* CHULA_LOG_H */
