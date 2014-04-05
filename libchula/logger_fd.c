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
#include "logger_fd.h"
#include "util.h"

#include <time.h>
#include <errno.h>
#include <unistd.h>

static ret_t
do_render (void *self, chula_log_level_t level, chula_buffer_t *buf)
{
    size_t         re;
    time_t         now;
    struct tm      localtime;
    char           tmp[30];
    chula_buffer_t fake;

    UNUSED (self);

    /* Prefix */
    chula_log_get_level_buf (level, &fake);
    chula_buffer_prepend_str (buf, " ");
    chula_buffer_prepend_buf (buf, &fake);

    time (&now);
    chula_localtime (&now, &localtime);
    re = strftime (tmp, sizeof(tmp), "[%Y-%m-%d %H:%M:%S] ", &localtime);
    chula_buffer_prepend (buf, tmp, re);

    /* New line suffix */
    if (! chula_buffer_is_ending (buf, CHR_LF)) {
        chula_buffer_add_char (buf, CHR_LF);
    }

    return ret_ok;
}

static ret_t
do_log (void *self, chula_log_level_t level, chula_buffer_t *buf)
{
    int             re;
    chula_log_fd_t *log = CHULA_LOG_FD(self);

    UNUSED(level);

    do {
        re = write (log->fd, buf->buf, buf->len);
    } while ((re == -1) && (errno == EAGAIN));

    return ret_ok;
}


static ret_t
_fd_log (void *self, chula_log_level_t level, chula_buffer_t *buf)
{
    ret_t ret;

    ret = do_render (self, level, buf);
    if (unlikely (ret != ret_ok)) return ret;

    ret = do_log (self, level, buf);
    if (unlikely (ret != ret_ok)) return ret;

    return ret_ok;
}

static void
_fd_free (void *self)
{
    free (self);
}

ret_t
chula_log_fd_new (chula_log_fd_t **logger, int fd)
{
    CHULA_NEW_STRUCT (n, log_fd);

    n->fd              = fd;
    HPACK_LOG(n)->log  = _fd_log;
    HPACK_LOG(n)->free = _fd_free;

    *logger = n;
    return ret_ok;
}
