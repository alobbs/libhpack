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

#include "common-internal.h"
#include "log.h"
#include "util.h"

static chula_log_base_t *logger = NULL;

static void
free_logger (chula_log_base_t **logger)
{
    chula_log_base_t *l = *logger;

    if (l != NULL) {
        l->free (l);
        *logger = NULL;
    }
}

ret_t
chula_log_init (chula_log_base_t *log)
{
    if (logger) {
        free_logger (&logger);
    }
    logger = log;
    return ret_ok;
}

void
chula_log_shutdown (void)
{
    if (logger) {
        free_logger (&logger);
    }
}

void
chula_log_buf (chula_log_level_t  level,
               chula_buffer_t    *buf)
{
    if (logger != NULL) {
        logger->log (logger, level, buf);
    }
}

void
chula_log_get_level_buf (chula_log_level_t  level,
                         chula_buffer_t    *buf)
{
    switch (level) {
    case CHULA_LOG_ERROR:
        chula_buffer_fake_str (buf, "ERROR");
        break;
    case CHULA_LOG_WARN:
        chula_buffer_fake_str (buf, "WARN");
        break;
    case CHULA_LOG_INFO:
        chula_buffer_fake_str (buf, "INFO");
        break;
    case CHULA_LOG_DEBUG:
        chula_buffer_fake_str (buf, "DEBUG");
        break;
    case CHULA_LOG_TRACE:
        chula_buffer_fake_str (buf, "TRACE");
        break;
    default:
        SHOULDNT_HAPPEN;
    }
}


#define chula_log_GUTS(level)                                   \
    va_list        ap;                                          \
    chula_buffer_t tmp = CHULA_BUF_INIT;                        \
                                                                \
    va_start (ap, format);                                      \
    chula_buffer_add_va_list (&tmp, format, ap);                \
    va_end (ap);                                                \
                                                                \
    chula_log_buf (level, &tmp);                                \
    chula_buffer_mrproper (&tmp)                                \


void
chula_log_error (const char *format, ...)
{
    chula_log_GUTS (CHULA_LOG_ERROR);
}

void
chula_log_warn (const char *format, ...)
{
    chula_log_GUTS (CHULA_LOG_WARN);
}

void
chula_log_info (const char *format, ...)
{
    chula_log_GUTS (CHULA_LOG_INFO);
}

void
chula_log_debug (const char *format, ...)
{
    chula_log_GUTS (CHULA_LOG_DEBUG);
}

void
chula_log_trace (const char *format, ...)
{
    chula_log_GUTS (CHULA_LOG_TRACE);
}


void
chula_log_errno (int                errorno,
                 chula_log_level_t  level,
                 const char        *format, ...)
{
    va_list         ap;
    char           *estr;
    chula_buffer_t  tmp   = CHULA_BUF_INIT;

    /* Format log entry */
    va_start (ap, format);
    chula_buffer_add_va_list (&tmp, format, ap);
    va_end (ap);

    /* errno */
    CHULA_TEMP_VARS(error,128);

    estr = chula_strerror_r (errorno, error, error_size);
    if (likely (estr != NULL)) {
        chula_buffer_replace_string (&tmp, "${errno}", 8, estr, strlen(estr));
    }

    /* Log */
    chula_log_buf (level, &tmp);
    chula_buffer_mrproper (&tmp);
}
