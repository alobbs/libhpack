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
#include "buffer.h"

#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "util.h"
#include "md5.h"
#include "util.h"
#include "crc32.h"
#include "sha1.h"
#include "sha512.h"

#define REALLOC_EXTRA_SIZE     16
#define IOS_NUMBUF             64   /* I/O size of digits buffer */

#define TO_HEX(c)               ((c) > 9 ? (c) + 'a' - 10 : (c) + '0')


static int
chula_estimate_va_length (const char *fmt, va_list ap)
{
    char      *p;
    uint8_t   ch;
    int64_t   ll;
    uint64_t  ul;
    bool      lflag;
    bool      llflag;
    uint32_t  width;
    char      padc;
    uint32_t  len    = 0;

#define LEN_NUM(var,base)                       \
    do {                                        \
        var /= base;                            \
        len++;                                  \
    } while (var > 0);                          \
    len++

    for (;;) {
        width = 0;
        padc  = ' ';

        while ((ch = *fmt++) != '%') {
            if (ch == '\0')
                return len+1;
            len++;
        }
        lflag = llflag = false;

    reswitch:
        switch (ch = *fmt++) {
        case 's':
            p = va_arg(ap, char *);
            len += strlen (p ? p : "(null)");
            break;
        case 'd':
            ll = lflag ? va_arg(ap, int32_t) : va_arg(ap, int);
            if (unlikely (ll < 0)) {
                ll = -ll;
                len++;
            }
            LEN_NUM(ll,10);
            break;
        case 'l':
            if (lflag == false)
                lflag = true;
            else
                llflag = true;
            goto reswitch;
        case 'u':
            if (llflag) {
                ul = va_arg(ap, uint64_t);
            } else {
                ul = lflag ? va_arg(ap, long) : va_arg(ap, int);
            }
            LEN_NUM(ul,10);
            break;
        case '0':
            padc = '0';
            goto reswitch;
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            for (width = 0;; ++fmt) {
                width = width * 10 + ch - '0';
                ch = *fmt;
                if (ch < '0' || ch > '9')
                    break;
            }
            len += width;
            goto reswitch;
        case 'c':
            (void) va_arg(ap, int);
            len++;
            break;
        case 'o':
            ll = lflag ? va_arg(ap, int32_t) : va_arg(ap, int);
            if (unlikely (ll < 0)) {
                ll = -ll;
                len++;
            }
            LEN_NUM(ll,8);
            break;
        case 'f':
            ul = va_arg(ap, double); /* FIXME: Add float numbers support */
            len += 30;
            LEN_NUM(ul,10);
            break;
        case 'p':
            len += 2;                /* Pointer: "0x" + hex value */
            if (sizeof(void *) > sizeof(int))
                lflag = true;
        case 'x':
            ll = lflag ? va_arg(ap, int32_t) : va_arg(ap, int);
            if (unlikely (ll < 0)) {
                ll = -ll;
                len++;
            }
            LEN_NUM(ll,16);
            break;
        case '%':
            len++;
        default:
            len+=2;
        }
    }

    return -1;
}


/* Implements _new() and _free()
 */
CHULA_ADD_FUNC_NEW  (buffer);
CHULA_ADD_FUNC_FREE (buffer);

ret_t
chula_buffer_init (chula_buffer_t *buf)
{
    buf->buf  = NULL;
    buf->len  = 0;
    buf->size = 0;

    return ret_ok;
}

void
chula_buffer_fake (chula_buffer_t *buf, const char *str, uint32_t len)
{
    buf->buf  = (uint8_t *)str;
    buf->len  = len;
    buf->size = len + 1;
}

ret_t
chula_buffer_import (chula_buffer_t *buf, char *str, uint32_t len)
{
    ret_t  ret;
    size_t s    = 0;

    if (buf->buf != NULL) {
        free (buf->buf);
    }

    buf->buf = (uint8_t *)str;
    buf->len = len;

    ret = chula_malloc_size (str, &s);
    if (unlikely (ret != ret_ok)) return ret;

    buf->size = s;
    return ret_ok;
}


ret_t
chula_buffer_mrproper (chula_buffer_t *buf)
{
    if (buf->buf) {
        free (buf->buf);
        buf->buf = NULL;
    }

    buf->len  = 0;
    buf->size = 0;

    return ret_ok;
}

void
chula_buffer_clean (chula_buffer_t *buf)
{
    if (buf->buf != NULL)
        buf->buf[0] = (uint8_t) '\0';
    buf->len = 0;
}

void
chula_buffer_swap_buffers (chula_buffer_t *buf, chula_buffer_t *second)
{
    uint8_t  *tmp_buf;
    uint32_t  tmp_len;
    uint32_t  tmp_size;

    tmp_buf  = buf->buf;
    tmp_len  = buf->len;
    tmp_size = buf->size;

    buf->buf  = second->buf;
    buf->len  = second->len;
    buf->size = second->size;

    second->buf = tmp_buf;
    second->len = tmp_len;
    second->size = tmp_size;
}

ret_t
chula_buffer_dup (chula_buffer_t *buf, chula_buffer_t **dup)
{
    CHULA_NEW_STRUCT(n, buffer);

    n->buf = (uint8_t *) malloc(buf->len + 1);
    if (unlikely (n->buf == NULL)) {
        free(n);
        return ret_nomem;
    }

    memcpy (n->buf, buf->buf, buf->len + 1);

    n->len  = buf->len;
    n->size = buf->len + 1;

    *dup = n;
    return ret_ok;
}


static ret_t
realloc_inc_bufsize (chula_buffer_t *buf, size_t incsize)
{
    uint8_t *pbuf;
    size_t   newsize = buf->size + incsize + REALLOC_EXTRA_SIZE + 1;

    pbuf = (uint8_t *) realloc(buf->buf, newsize);
    if (unlikely (pbuf == NULL)) return ret_nomem;

    buf->buf  = pbuf;
    buf->size = (int) newsize;
    return ret_ok;
}


static ret_t
realloc_new_bufsize (chula_buffer_t *buf, size_t newsize)
{
    uint8_t *pbuf;

    newsize += REALLOC_EXTRA_SIZE + 1;

    pbuf = (uint8_t *) realloc(buf->buf, newsize);
    if (unlikely (pbuf == NULL)) return ret_nomem;

    buf->buf = pbuf;
    buf->size = (int) newsize;
    return ret_ok;
}


ret_t
chula_buffer_add (chula_buffer_t *buf, const char *txt, size_t size)
{
    ret_t ret;
    int   available;

    if (unlikely (size <= 0))
        return ret_ok;

    /* Get memory
     */
    available = buf->size - buf->len;

    if ((uint32_t) available < (size+1)) {
        ret = realloc_inc_bufsize(buf, size - available);
        if (unlikely (ret != ret_ok)) return ret;
    }

    /* Copy
     */
    memcpy (buf->buf + buf->len, txt, size);

    buf->len += size;
    buf->buf[buf->len] = (uint8_t) '\0';

    return ret_ok;
}


ret_t
chula_buffer_add_buffer (chula_buffer_t *buf, chula_buffer_t *buf2)
{
    return chula_buffer_add (buf, (const char *)buf2->buf, buf2->len);
}


ret_t
chula_buffer_add_buffer_slice (chula_buffer_t *buf,
                               chula_buffer_t *buf2,
                               ssize_t         begin,
                               ssize_t         end)
{
    ssize_t pos_end;
    ssize_t pos_begin;

    /* Ensure there's something to copy
     */
    if (chula_buffer_is_empty (buf2))
        return ret_ok;

    if ((end   != CHULA_BUF_SLIDE_NONE) &&
        (begin != CHULA_BUF_SLIDE_NONE))
    {
        pos_begin = (begin > 0) ? begin : buf2->len - abs(begin);
        pos_end   = (end   > 0) ? end   : buf2->len - abs(end);

        if (pos_end <= pos_begin) {
            return ret_ok;
        }
    }

    /* Check the end
     */
    if (end == CHULA_BUF_SLIDE_NONE) {
        /* [__:] */
        pos_end = buf2->len;
    } else {
        if (end > 0) {
            /* [__:x] */
            pos_end = end;
        } else {
            if ((-end) <= buf2->len) {
                /* [__:-x] */
                pos_end = buf2->len - (-end);
            } else {
                /* [__:-xxxxx] */
                return ret_ok;
            }
        }
    }

    /* Check the beginning
     */
    if (begin == CHULA_BUF_SLIDE_NONE) {
        /* [:__] */
        pos_begin = 0;
    } else {
        if (begin >= 0) {
            if (begin > buf2->len) {
                /* [xxxx:__]  */
                pos_begin = buf2->len;
            } else {
                /* [x:__] */
                pos_begin = begin;
            }
        } else {
            if ((-begin) < buf2->len) {
                /* [-x:__] */
                pos_begin = buf2->len - (-begin);
            } else {
                /* [-xxxx:__] */
                pos_begin = 0;
            }
        }
    }

    /* Sanity check
     */
    if (unlikely ((pos_begin < 0)       ||
                  (pos_end < 0)         ||
                  (pos_end > buf2->len) ||
                  (pos_end < pos_begin)))
    {
        return ret_ok;
    }

    /* Copy the substring
     */
    return chula_buffer_add (buf, (const char *)buf2->buf + pos_begin, pos_end - pos_begin);
}


ret_t
chula_buffer_add_fsize (chula_buffer_t *buf, CST_OFFSET size)
{
    ret_t       ret;
    int         remain;
    const char  ord[]  = "KMGTPE";
    const char *o      = ord;

    ret = chula_buffer_ensure_size (buf, buf->len + 8);
    if (unlikely (ret != ret_ok)) return ret;

    if (size < 973)
        return chula_buffer_add_ulong10 (buf, (uint32_t)size);

    do {
        remain = (int)(size & 1023);
        size >>= 10;
        if (size >= 973) {
            ++o;
            continue;
        }
        if (size < 9 || (size == 9 && remain < 973)) {
            remain = ((remain * 5) + 256) / 512;
            if (remain >= 10) {
                ++size, remain = 0;
            }
            return chula_buffer_add_va_fixed (buf, "%d.%d%c", (int) size, remain, *o);
        }
        if (remain >= 512)
            ++size;
        return chula_buffer_add_va_fixed (buf, "%3d%c", (int) size, *o);
    } while (1);

    return ret_ok;
}


ret_t
chula_buffer_add_long10 (chula_buffer_t *buf, int32_t lNum)
{
    ret_t    ret;
    char     szOutBuf[IOS_NUMBUF];
    uint32_t ulNum                 = (uint32_t) lNum;
    uint32_t flgNeg                = 0;
    int      newlen                = 0;
    size_t   i                     = (IOS_NUMBUF - 1);

    if (lNum < 0L) {
        flgNeg = 1;
        ulNum = -ulNum;
    }

    szOutBuf[i] = '\0';

    /* Convert number to string
     */
    do {
        szOutBuf[--i] = (char) ((ulNum % 10) + '0');
    }
    while ((ulNum /= 10) != 0);

    /* Set sign in any case
     */
    szOutBuf[--i] = '-';
    i += (flgNeg ^ 1);

    /* Verify free space in buffer and if needed then enlarge it.
     */
    newlen = buf->len + (int) ((IOS_NUMBUF - 1) - i);
    if (unlikely ((uint32_t)newlen >= buf->size)) {
        ret = realloc_new_bufsize(buf, newlen);
        if (unlikely (ret != ret_ok)) return ret;
    }

    /* Copy including '\0'
     */
    strcpy ((char *)buf->buf + buf->len, &szOutBuf[i]);

    buf->len = newlen;
    return ret_ok;
}


ret_t
chula_buffer_add_llong10 (chula_buffer_t *buf, int64_t lNum)
{
    ret_t    ret;
    char     szOutBuf[IOS_NUMBUF];
    uint64_t ulNum                 = (uint64_t) lNum;
    uint32_t flgNeg                = 0;
    int      newlen                = 0;
    size_t   i                     = (IOS_NUMBUF - 1);

    if (lNum < 0L) {
        flgNeg = 1;
        ulNum = -ulNum;
    }

    szOutBuf[i] = '\0';

    /* Convert number to string
     */
    do {
        szOutBuf[--i] = (char) ((ulNum % 10) + '0');
    }
    while ((ulNum /= 10) != 0);

    /* Set sign in any case
     */
    szOutBuf[--i] = '-';
    i += (flgNeg ^ 1);

    /* Verify free space in buffer and if needed then enlarge it.
     */
    newlen = buf->len + (int) ((IOS_NUMBUF - 1) - i);
    if (unlikely ((uint32_t)newlen >= buf->size)) {
        ret = realloc_new_bufsize(buf, newlen);
        if (unlikely (ret != ret_ok)) return ret;
    }

    /* Copy including '\0'
     */
    strcpy ((char *)buf->buf + buf->len, &szOutBuf[i]);

    buf->len = newlen;

    return ret_ok;
}


ret_t
chula_buffer_add_ulong10 (chula_buffer_t *buf, uint32_t ulNum)
{
    ret_t   ret;
    char    szOutBuf[IOS_NUMBUF];
    int     newlen               = 0;
    size_t  i                    = (IOS_NUMBUF - 1);

    szOutBuf[i] = '\0';

    /* Convert number to string
     */
    do {
        szOutBuf[--i] = (char) ((ulNum % 10) + '0');
    }
    while ((ulNum /= 10) != 0);

    /* Verify free space in buffer and if needed then enlarge it.
     */
    newlen = buf->len + (int) ((IOS_NUMBUF - 1) - i);
    if (unlikely ((uint32_t)newlen >= buf->size)) {
        ret = realloc_new_bufsize(buf, newlen);
        if (unlikely (ret != ret_ok)) return ret;
    }

    /* Copy including '\0'
     */
    strcpy ((char *)buf->buf + buf->len, &szOutBuf[i]);

    buf->len = newlen;

    return ret_ok;
}


ret_t
chula_buffer_add_ullong10 (chula_buffer_t *buf, uint64_t ulNum)
{
    ret_t   ret;
    char    szOutBuf[IOS_NUMBUF];
    int     newlen                = 0;
    size_t  i                     = (IOS_NUMBUF - 1);

    szOutBuf[i] = '\0';

    /* Convert number to string
     */
    do {
        szOutBuf[--i] = (char) ((ulNum % 10) + '0');
    }
    while ((ulNum /= 10) != 0);

    /* Verify free space in buffer and if needed then enlarge it.
     */
    newlen = buf->len + (int) ((IOS_NUMBUF - 1) - i);
    if (unlikely ((uint32_t)newlen >= buf->size)) {
        ret = realloc_new_bufsize(buf, newlen);
        if (unlikely (ret != ret_ok)) return ret;
    }

    /* Copy including '\0'
     */
    strcpy ((char *)buf->buf + buf->len, &szOutBuf[i]);

    buf->len = newlen;

    return ret_ok;
}


/*
** Add a number in hexadecimal format to (buf).
*/
ret_t
chula_buffer_add_ulong16 (chula_buffer_t *buf, uint32_t ulNum)
{
    ret_t   ret;
    char    szOutBuf[IOS_NUMBUF];
    size_t  i                     = (IOS_NUMBUF - 1);
    int     ival                  = 0;
    int     newlen                = 0;

    szOutBuf[i] = '\0';

    /* Convert number to string
     */
    do {
        ival = (int) (ulNum & 0xF);
        szOutBuf[--i] = (char) TO_HEX(ival);
    }
    while ((ulNum >>= 4) != 0);

    /* Verify free space in buffer and if needed then enlarge it.
     */
    newlen = buf->len + (int) ((IOS_NUMBUF - 1) - i);
    if (unlikely ((uint32_t)newlen >= buf->size)) {
        ret = realloc_new_bufsize(buf, newlen);
        if (unlikely (ret != ret_ok)) return ret;
    }

    /* Copy including '\0'
     */
    strcpy ((char *)buf->buf + buf->len, &szOutBuf[i]);

    buf->len = newlen;

    return ret_ok;
}


/*
** Add a number in hexadecimal format to (buf).
*/
ret_t
chula_buffer_add_ullong16 (chula_buffer_t *buf, uint64_t ulNum)
{
    ret_t  ret;
    char   szOutBuf[IOS_NUMBUF];
    size_t i                     = (IOS_NUMBUF - 1);
    int    ival                  = 0;
    int    newlen                = 0;

    szOutBuf[i] = '\0';

    /* Convert number to string
     */
    do {
        ival = (int) (ulNum & 0xF);
        szOutBuf[--i] = (char) TO_HEX(ival);
    }
    while ((ulNum >>= 4) != 0);

    /* Verify free space in buffer and if needed then enlarge it.
     */
    newlen = buf->len + (int) ((IOS_NUMBUF - 1) - i);
    if (unlikely ((uint32_t)newlen >= buf->size)) {
        ret = realloc_new_bufsize(buf, newlen);
        if (unlikely (ret != ret_ok)) return ret;
    }

    /* Copy including '\0'
     */
    strcpy ((char *)buf->buf + buf->len, &szOutBuf[i]);

    buf->len = newlen;

    return ret_ok;
}


ret_t
chula_buffer_add_uint16be (chula_buffer_t *buf, uint16_t n)
{
    uint16_t x = htons(n);
    return chula_buffer_add (buf, (char *)&x, sizeof(uint16_t));
}


ret_t
chula_buffer_add_uint32be (chula_buffer_t *buf, uint32_t n)
{
    uint32_t x = htonl(n);
    return chula_buffer_add (buf, (char *)&x, sizeof(uint32_t));
}


ret_t
chula_buffer_add_va_fixed (chula_buffer_t *buf, const char *format, ...)
{
    va_list ap;
    int     len;
    int     size = buf->size - buf->len;    /* final '\0' is always available */

    /* Test for minimum buffer size.
     */
    if (size < 1)
        return ret_error;

    /* Format the string into the buffer.
     * NOTE: len does NOT include '\0', size includes '\0' (len + 1)
     */
    va_start (ap, format);
    len = vsnprintf ((char *)buf->buf + buf->len, size, format, ap);
    va_end (ap);

    if (unlikely (len < 0))
        return ret_error;

    /* Don't expand buffer if there is not enough space.
     */
    if (unlikely (len >= size))
        return ret_error;

    buf->len += len;
    return ret_ok;
}


ret_t
chula_buffer_add_va_list (chula_buffer_t *buf, const char *format, va_list args)
{
    int     len;
    int     estimation;
    int     size;
    ret_t   ret;
    va_list args2;

    va_copy (args2, args);

    /* Estimate resulting formatted string length.
     */
    estimation = chula_estimate_va_length (format, args);
    if (unlikely (estimation) < 0) {
        chula_log_error ("Buffer: Bad memory estimation. The format '%s' estimated a negative length: %d", format, estimation);
        ret = ret_error;
        goto error;
    }

    /* Ensure enough size for buffer.
     */
    ret = chula_buffer_ensure_size (buf, buf->len + estimation + 2);
    if (unlikely (ret != ret_ok)) goto error;

    /* Format the string into the buffer.
     * NOTE: len does NOT include '\0', size includes '\0' (len + 1)
     */
    size = buf->size - buf->len;
    if (unlikely (size < 1)) {
        chula_log_error ("Buffer: No target memory. The format '%s' got a free size of %d (estimated %d).", size, estimation);
        ret = ret_error;
        goto error;
    }

    len = vsnprintf ((char *)buf->buf + buf->len, size, format, args2);
#if 0
    if (unlikely (estimation < len)) {
        chula_log_error ("Buffer: Bad estimation. Too few memory: '%s' -> '%s', esti=%d real=%d size=%d.",
                         format, buf->buf + buf->len, estimation, len, size););
    }
#endif

    if (unlikely (len < 0)) {
        ret = ret_error;
        goto error;
    }

    /* At this point buf-size is always greater than buf-len, thus size > 0.
     */
    if (len >= size) {
        chula_log_error ("Buffer: Bad estimation: Estimation=%d, needed=%d available size=%d: %s.", estimation, len, size, format);

        ret = chula_buffer_ensure_size (buf, buf->len + len + 2);
        if (unlikely (ret != ret_ok)) goto error;

        size = buf->size - buf->len;
        len = vsnprintf ((char *)buf->buf + buf->len, size, format, args2);

        if (unlikely ((len < 0) || (len >= size))) {
            ret = ret_error;
            goto error;
        }
    }

    buf->len += len;

    va_end (args2);
    return ret_ok;

error:
    va_end (args2);
    return ret;
}


ret_t
chula_buffer_add_va (chula_buffer_t *buf, const char *format, ...)
{
    ret_t   ret;
    va_list ap;

    va_start (ap, format);
    ret = chula_buffer_add_va_list (buf, format, ap);
    va_end (ap);

    return ret;
}


ret_t
chula_buffer_add_char (chula_buffer_t *buf, char c)
{
    ret_t ret;

    /* Add char (fast path)
     */
    if (likely (buf->len + 1 < buf->size)) {
        buf->buf[buf->len++] = (uint8_t) c;
        buf->buf[buf->len] = (uint8_t) '\0';
        return ret_ok;
    }

    /* Get memory
     */
    ret = realloc_inc_bufsize (buf, 1);
    if (unlikely (ret != ret_ok)) return ret;

    /* Add char
     */
    buf->buf[buf->len++] = (uint8_t) c;
    buf->buf[buf->len] = (uint8_t) '\0';

    return ret_ok;
}


ret_t
chula_buffer_add_char_n (chula_buffer_t *buf, char c, int num)
{
    ret_t ret;
    int   free = buf->size - buf->len;

    if (num <= 0)
        return ret_ok;

    /* Get memory
     */
    if (free < (num+1)) {
        ret = realloc_inc_bufsize(buf, num - free);
        if (unlikely (ret != ret_ok)) return ret;
    }

    memset (buf->buf+buf->len, c, num);
    buf->len += num;
    buf->buf[buf->len] = (uint8_t) '\0';

    return ret_ok;
}


ret_t
chula_buffer_prepend (chula_buffer_t *buf, const char *txt, size_t size)
{
    ret_t ret;
    int   free = buf->size - buf->len;

    if (size <= 0)
        return ret_ok;

    /* Get memory
     */
    if ((uint32_t) free < (size+1)) {
        ret = realloc_inc_bufsize(buf, size - free);
        if (unlikely (ret != ret_ok)) return ret;
    }

    memmove (buf->buf+size, buf->buf, buf->len);

    memcpy (buf->buf, txt, size);
    buf->len += size;
    buf->buf[buf->len] = (uint8_t) '\0';

    return ret_ok;
}


int
chula_buffer_is_ending (chula_buffer_t *buf, char c)
{
    if (chula_buffer_is_empty(buf))
        return 0;

    return (buf->buf[buf->len - 1] == c);
}


ret_t
chula_buffer_move_to_begin (chula_buffer_t *buf, uint32_t pos)
{
    if (pos == 0)
        return ret_ok;

    if (pos >= buf->len) {
        chula_buffer_clean(buf);
        return ret_ok;
    }

    /* At this point: 0 < pos < buf->len
     */
    memmove (buf->buf, buf->buf+pos, (buf->len - pos) + 1);
    buf->len -= pos;

    return ret_ok;
}


/*
 * Ensure there is enough (addlen) free space left in the buffer.
 */
ret_t
chula_buffer_ensure_addlen (chula_buffer_t *buf, size_t addlen)
{
    if ((buf->len + addlen) < buf->size)
        return ret_ok;

    return chula_buffer_ensure_size (buf, ((size_t)buf->len + addlen + 1));
}


ret_t
chula_buffer_ensure_size (chula_buffer_t *buf, size_t size)
{
    uint8_t *pbuf;

    /* Maybe it doesn't need it
     * if buf->size == 0 and size == 0 then buf can be NULL.
     */
    if (size <= buf->size)
        return ret_ok;

    /* If it is a new buffer, take memory and return
     */
    if (buf->buf == NULL) {
        buf->buf = (uint8_t *) malloc (size);
        if (unlikely (buf->buf == NULL)) {
            buf->len  = 0;
            buf->size = 0;
            return ret_nomem;
        }
        buf->size = size;
        return ret_ok;
    }

    /* It already has memory, but it needs more..
     */
    pbuf = (uint8_t *) realloc(buf->buf, size);
    if (unlikely (pbuf == NULL)) return ret_nomem;

    buf->buf  = pbuf;
    buf->size = size;
    return ret_ok;
}


ret_t
chula_buffer_retract (chula_buffer_t *buf)
{
    uint8_t *pbuf;

    /* Nothing to do when..
     */
    if (chula_buffer_is_empty (buf))
        return ret_ok;

    if (buf->size <= buf->len+1)
        return ret_ok;

    /* Shrink the allocated memory
     */
    pbuf = (uint8_t *) realloc (buf->buf, buf->len+1);
    if (unlikely (pbuf == NULL)) return ret_nomem;

    buf->buf  = pbuf;
    buf->size = buf->len + 1;
    return ret_ok;
}


ret_t
chula_buffer_drop_ending (chula_buffer_t *buffer, uint32_t num_chars)
{
    int num;

    if (buffer->buf == NULL) {
        return ret_ok;
    }

    num = MIN (num_chars, buffer->len);

    buffer->buf[buffer->len - num] = (uint8_t) '\0';
    buffer->len -= num;

    return ret_ok;
}


ret_t
chula_buffer_swap_chars (chula_buffer_t *buffer, char a, char b)
{
    uint32_t i;

    if (buffer->buf == NULL) {
        return ret_ok;
    }

    for (i=0; i < buffer->len; i++) {
        if (buffer->buf[i] == (uint8_t) a) {
            buffer->buf[i] = (uint8_t) b;
        }
    }

    return ret_ok;
}


ret_t
chula_buffer_remove_dups (chula_buffer_t *buffer, char c)
{
    uint8_t       *a      = buffer->buf;
    const uint8_t *end    = buffer->buf + buffer->len;
    uint32_t       offset = 0;

    if (buffer->len < 2) {
        return ret_ok;
    }

    do {
        if ((*a == (uint8_t) c) && (a[offset+1] == (uint8_t) c)) {
            offset++;
            continue;
        }

        a++;
        *a = a[offset];

    } while ((a + offset < end) && (offset+1 < buffer->len));

    buffer->len -= offset;
    buffer->buf[buffer->len] = (uint8_t) '\0';

    return ret_ok;
}


ret_t
chula_buffer_remove_string (chula_buffer_t *buf, char *string, int string_len)
{
    char *tmp;
    int   offset;

    if ((buf->len <= 0) || (string == NULL) || (string_len <= 0))
        return ret_ok;

    while ((tmp = strstr ((char *)buf->buf, string)) != NULL) {
        offset = tmp - (char *)buf->buf;
        memmove (tmp, tmp+string_len, buf->len - (offset+string_len) +1);
        buf->len -= string_len;
    }

    return ret_ok;
}


ret_t
chula_buffer_remove_chunk (chula_buffer_t *buf, uint32_t from, uint32_t len)
{
    uint8_t *end;
    uint8_t *begin;

    if (from >= buf->len)
        return ret_ok;

    if ((from == 0) && (len >= buf->len)) {
        chula_buffer_clean (buf);
        return ret_ok;
    }

    begin = buf->buf + from;
    end   = MIN ((begin + len), (buf->buf + buf->len));

    memmove (begin, end, ((buf->buf + buf->len) - end) + 1);
    buf->len -= len;

    return ret_ok;
}


int32_t
chula_buffer_cmp_buf (chula_buffer_t *A, chula_buffer_t *B)
{
    if (A->len > B->len)
        return A->len - B->len;
    else if (B->len > A->len)
        return - (B->len - A->len);

    return memcmp (A->buf, B->buf, B->len);
}

int32_t
chula_buffer_cmp (chula_buffer_t *buf, char *txt, uint32_t txt_len)
{
    chula_buffer_t tmp;
    chula_buffer_fake (&tmp, txt, txt_len);
    return chula_buffer_cmp_buf (buf, &tmp);
}


int32_t
chula_buffer_case_cmp_buf (chula_buffer_t *A, chula_buffer_t *B)
{
    if (A->len > B->len)
        return A->len - B->len;
    else if (B->len > A->len)
        return - (B->len - A->len);

    return strncasecmp ((char *)A->buf, (char *)B->buf, B->len);
}

int32_t
chula_buffer_case_cmp (chula_buffer_t *buf, char *txt, uint32_t txt_len)
{
    chula_buffer_t tmp;
    chula_buffer_fake (&tmp, txt, txt_len);
    return chula_buffer_case_cmp_buf (buf, &tmp);
}


size_t
chula_buffer_cnt_spn (chula_buffer_t *buf, uint32_t offset, const char *str)
{
    if (unlikely ((buf->buf == NULL) || (buf->len <= offset)))
        return 0;

    return strspn ((char *)buf->buf + offset, str);
}


size_t
chula_buffer_cnt_cspn (chula_buffer_t *buf, uint32_t offset, const char *str)
{
    if (unlikely ((buf->buf == NULL) || (buf->len <= offset)))
        return 0;

    return strcspn ((char *)buf->buf + offset, str);
}


crc_t
chula_buffer_crc32 (chula_buffer_t *buf)
{
    if (chula_buffer_is_empty (buf))
        return 0;

    return crc32_sz ((char *)buf->buf, buf->len);
}


ret_t
chula_buffer_read_file (chula_buffer_t *buf, char *filename)
{
    ret_t       ret;
    int         r, f;
    struct stat info;

    /* Stat() the file
     */
    r = chula_stat (filename, &info);
    if (r != 0) return ret_error;

    /* Is a regular file?
     */
    if (S_ISREG(info.st_mode) == 0)
        return ret_error;

    /* Open the file
     */
    f = chula_open (filename, O_RDONLY | O_BINARY, 0);
    if (f < 0) {
        CHULA_TEMP_VARS (tmp, 256);
        chula_log_error ("Could not open the file: %s, %s", filename, chula_strerror_r(errno, tmp, tmp_size));
        return ret_error;
    }

    chula_fd_set_closexec (f);

    /* Maybe get memory
     */
    ret = chula_buffer_ensure_size (buf, buf->len + info.st_size + 1);
    if (unlikely (ret != ret_ok)) goto error;

    /* Read the content
     */
    r = read (f, buf->buf + buf->len, info.st_size);
    if (r < 0) {
        buf->buf[buf->len] = (uint8_t) '\0';
        ret = ret_error;
        goto error;
    }

    /* Close it and exit
     */
    chula_fd_close(f);

    buf->len += r;
    buf->buf[buf->len] = (uint8_t) '\0';

    return ret_ok;

error:
    chula_fd_close(f);
    return ret;
}


ret_t
chula_buffer_read_from_fd (chula_buffer_t *buf, int fd, size_t size, size_t *ret_size)
{
    ret_t ret;
    int   len;

    if (fd < 0)
        return ret_error;

    /* Ensure there is enough space in buffer
     * NOTE: usually the caller should have already allocated
     *       enough space for the buffer, so this is a security measure
     */
    ret = chula_buffer_ensure_addlen(buf, size);
    if (unlikely (ret != ret_ok)) return ret;

    /* Read data at the end of the buffer
     */
    do {
        len = read (fd, &(buf->buf[buf->len]), size);
    } while ((len == -1) && (errno == EINTR));

    if (len < 0) {
        /* On error
         */
        switch (errno) {
        case EAGAIN:
#if defined(EWOULDBLOCK) && (EWOULDBLOCK != EAGAIN)
        case EWOULDBLOCK:
#endif
            return ret_eagain;
        case EPIPE:
        case EBADF:
        case ECONNRESET:
            return ret_eof;
        case EIO:
            return ret_error;
        }

//      LOG_ERRNO (errno, chula_err_error, CHULA_ERROR_BUFFER_READ_FILE, fd, size);
        return ret_error;
    }
    else if (len == 0) {
        /* On EOF
         */
        return ret_eof;
    }

    /* Add read length, terminate buffer and return
     */
    *ret_size = len;
    buf->len += len;

    buf->buf[buf->len] = (uint8_t) '\0';

    return ret_ok;
}


ret_t
chula_buffer_multiply (chula_buffer_t *buf, int num)
{
    ret_t ret;
    int   i;
    int   initial_size;

    initial_size = buf->len;
    ret = chula_buffer_ensure_size (buf, buf->len * num + 1);
    if (unlikely (ret != ret_ok)) return ret;

    for (i=1; i<num; i++) {
        ret = chula_buffer_add (buf, (const char *)buf->buf, initial_size);
        if (unlikely (ret != ret_ok)) return ret;
    }

    return ret_ok;
}


void
chula_buffer_repr (chula_buffer_t *buf,
                   chula_buffer_t *output)
{
    ret_t     ret;
    uint32_t  i;
    uint8_t   tmp;
    char      text[71]   = {[0 ... 66] = 0};
    char     *hex_text   = NULL;
    char     *ascii_text = NULL;

    for (i=0; i < buf->len; i++) {
        if (i%16 == 0) {
            if (text[0] != 0){
                ret = chula_buffer_add (output, text, strlen(text));
                if (unlikely (ret != ret_ok)) return;
                ret = chula_buffer_add_str (output, CRLF);
                if (unlikely (ret != ret_ok)) return;
            }
            sprintf (text, "%08x%61c", i, ' ');
            hex_text = text + 10;
            ascii_text = text + 51;
            ascii_text[-1] = '|';
            ascii_text[16] = '|';
        }

        tmp = buf->buf[i];
        sprintf (hex_text, "%02x", tmp & 0xFF);
        hex_text += 2;
        *hex_text = ' ';
        if ((i+1)%2 == 0) {
            hex_text++;
        }

        if ((tmp >= ' ') && (tmp < 127))
            *ascii_text = tmp;
        else
            *ascii_text = '.';
        ascii_text += 1;
    }

    ret = chula_buffer_add (output, text, strlen(text));
    if (unlikely (ret != ret_ok)) return;
    ret = chula_buffer_add_str (output, CRLF);
    if (unlikely (ret != ret_ok)) return;
}


ret_t
chula_buffer_print_cstr (chula_buffer_t *buf)
{
    uint32_t i;

    for (i=0; i<buf->len; i++) {
        uint8_t c = buf->buf[i];
        printf("\"\\x%02x\"", c);
    }
    printf (CRLF);

    return ret_ok;
}


static const uint8_t *
utf8_get_next_char (const uint8_t *string)
{
    /* 2 bytes character: 110vvvvv 10vvvvvv
     */
    if (((unsigned char)(string[0]) & 0xE0) == 0xC0) {
        if (!string[1]) {
            return string + 1;
        }
        return string + 2;
    }

    /* 3 bytes character: 1110vvvv 10vvvvvv 10vvvvvv */
    if (((unsigned char)(string[0]) & 0xF0) == 0xE0) {
        if (!string[1]) {
            return string + 1;
        }
        if (!string[2]) {
            return string + 2;
        }
        return string + 3;
    }

    /* 4 bytes characters: 11110vvv 10vvvvvv 10vvvvvv 10vvvvvv */
    if (((unsigned char)(string[0]) & 0xF8) == 0xF0) {
        if (!string[1]) {
            return string + 1;
        }
        if (!string[2]) {
            return string + 2;
        }
        if (!string[3]) {
            return string + 3;
        }
        return string + 4;
    }

    /* Single byte character: 0vvvvvvv */
    return string + 1;
}


ret_t
chula_buffer_get_utf8_len (chula_buffer_t *buf, uint32_t *len)
{
    uint32_t       n;
    const uint8_t *p;
    const uint8_t *end;

    /* Empty buffer
     */
    if ((buf->buf == NULL) || (buf->len == 0)) {
        *len = 0;
        return ret_ok;
    }

    /* Count characters
     */
    p   = buf->buf;
    end = buf->buf + buf->len;

    n = 0;
    do{
        p = (uint8_t *) utf8_get_next_char (p);
        n++;
    } while (p < end);

    *len = n;
    return ret_ok;
}


/*
 * Unescape a string that may have escaped characters %xx
 * where xx is the hexadecimal number equal to the character ascii value.
 */
ret_t
chula_buffer_unescape_uri (chula_buffer_t *buffer)
{
    uint8_t *psrc;
    uint8_t *ptgt;
    int      len;

#define hex2dec_m(c)       ( (int) hex2dec_tab[c] )
#define hex2dec_m2(c1, c2) ( hex2dec_m(c1) * 16 + hex2dec_m(c2) )

    TRACE(ENTRIES, "Prev: %s\n", (char *)buffer->buf);

    if (chula_buffer_is_empty (buffer))
        return ret_ok;

    /* Verify string termination,
     * we assume there are no '\0' inside buffer.
     */
    if (buffer->buf[buffer->len] != (uint8_t) '\0')
        buffer->buf[buffer->len]  = (uint8_t) '\0';

    /* Verify if unescaping is needed.
     */
    if ((psrc = (uint8_t *)strchr((char *)buffer->buf, '%')) == NULL)
        return ret_ok;

    /* Yes, unescape string.
     */
    len = buffer->len;
    for (ptgt = psrc; *psrc != (uint8_t) '\0'; ++ptgt, ++psrc) {
        if (psrc[0] != (uint8_t)'%' ||
            !isxdigit((char)psrc[1]) || !isxdigit((char)psrc[2])) {
            *ptgt = *psrc;
            continue;
        }
        /* Escape sequence %xx
         */
        if (likely ((*ptgt = (uint8_t) hex2dec_m2(psrc[1], psrc[2])) != (uint8_t)'\0')) {
            psrc += 2;
            len  -= 2;
            continue;
        }
        /* Replace null bytes (%00) with
         * spaces, to prevent attacks
         */
        *ptgt = (uint8_t) ' ';
        psrc += 2;
        len  -= 2;
    }
    *ptgt = (uint8_t) '\0';
    buffer->len = len;

#undef hex2dec_m2
#undef hex2dec_m

    TRACE(ENTRIES, "Post: %s\n", (char *) buffer->buf);
    return ret_ok;
}

static ret_t
escape_with_table (chula_buffer_t *buffer,
                   chula_buffer_t *src,
                   uint32_t       *is_char_escaped)
{
    ret_t          ret;
    uint8_t       *t;
    const uint8_t *s, *s_next;
    uint8_t       *end;
    uint32_t       n_escape    = 0;
    static char    hex_chars[] = "0123456789abcdef";

    if (unlikely (chula_buffer_is_empty(src)))
        return ret_ok;

    end = src->buf + src->len;

    /* Count how many characters it'll have to escape. Each *bit*
     * position of the array represents whether or not the character
     * is escaped.
     */
    s = src->buf;
    do {
        s_next = (uint8_t *)utf8_get_next_char (s);

        /* It's single-byte character */
        if ((s_next - s) == 1) {

            /* Check whether it has to be escaped */
            if (is_char_escaped[*s >> 5] & (1 << (*s & 0x1f))) {
                n_escape++;
            }
        }

        /* Prepare for next iteration */
        s = s_next;
    } while (s < end);

    /* Get the memory
     */
    ret = chula_buffer_ensure_addlen (buffer, src->len + (n_escape * 3));
    if (unlikely (ret != ret_ok)) return ret;

    /* Convert it
     */
    s = src->buf;
    t = buffer->buf + buffer->len;

    do {
        s_next = (uint8_t *) utf8_get_next_char (s);

        /* Multi-byte character */
        if ((s_next - s) > 1) {
            while (s < s_next) {
                *t++ = *s++;
            }

            /* Single-byte character */
        } else {
            if (is_char_escaped[*s >> 5] & (1 << (*s & 0x1f))) {
                *t++ = '%';
                *t++ = hex_chars[*s >> 4];
                *t++ = hex_chars[*s & 0xf];
                s++;
            } else {
                *t++ = *s++;
            }
        }

        s = s_next;
    } while (s < end);

    /* ..and the final touch
     */
    *t = (uint8_t) '\0';
    buffer->len += src->len + (n_escape * 2);

    return ret_ok;

}

ret_t
chula_buffer_escape_uri (chula_buffer_t *buffer, chula_buffer_t *src)
{
    static uint32_t escape_uri[] = {
        /* %00-%1F, " ", "#", "%", "?", %7F-%FF */
        0xffffffff, 0x80000029, 0x00000000, 0x80000000,
        0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
    };

    return escape_with_table (buffer, src, escape_uri);
}

ret_t
chula_buffer_escape_uri_delims (chula_buffer_t *buffer, chula_buffer_t *src)
{
    /* It's basically chula_buffer_escape_uri() for paths
     * inside a URI. It escapes the same characters as its
     * sibling, plus a number of delimiters. Please check RFC 3986
     * (Uniform Resource Identifier) for further information:
     *
     *  ":", "?", "#", "[", "]", "@"
     */
    static uint32_t escape_uri[] = {
        0xffffffff, 0x84000029, 0x28000001, 0x80000000,
        0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
    };

    return escape_with_table (buffer, src, escape_uri);
}

ret_t
chula_buffer_escape_arg (chula_buffer_t *buffer, chula_buffer_t *src)
{
    static uint32_t escape_arg[] = {
        /* %00-%1F, " ", "#", "%", "&", "+", "?", %7F-%FF */
        0xffffffff, 0x88000869, 0x00000000, 0x80000000,
        0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
    };

    return escape_with_table (buffer, src, escape_arg);
}


ret_t
chula_buffer_add_escape_html (chula_buffer_t *buf, chula_buffer_t *src)
{
    ret_t    ret;
    size_t   len0 = 0;
    size_t   extra = 0;
    uint8_t *p0, *p1, *p2;

    /* Verify that source string is not empty.
     */
    if (chula_buffer_is_empty(src))
        return ret_ok;

    /* Verify string termination,
     * we assume there are no '\0' inside buffer.
     */
    if (src->buf[src->len] != (uint8_t) '\0')
        src->buf[src->len]  = (uint8_t) '\0';

    /* Verify if string has to be escaped.
     */
    if ((p0 = (uint8_t*) strpbrk ((char *)src->buf, "<>&\"")) == NULL) {
        /* No escape found, simply add src to buf.
         */
        return chula_buffer_add_buffer (buf, src);
    }

    /* Count extra characters
     */
    for (p1 = p0; *p1 != (uint8_t) '\0'; ++p1) {
        switch((char)*p1) {
        case '<':   /* &lt; */
        case '>':   /* &gt; */
            extra += 3;
            continue;
        case '&':   /* &amp; */
            extra += 4;
            continue;
        case '"':   /* &quot; */
            extra += 5;
            continue;
        case '#':   /* &#35; */
        case '\'':  /* &#39; */
        case '/':   /* &#47; */
            extra += 4;
        default:
            continue;
        }
    }

    /* Verify there are no embedded '\0'.
     */
    if (unlikely ((uint32_t)(p1 - src->buf) != src->len))
        return ret_error;

    /* Ensure there is proper buffer size.
     */
    ret = chula_buffer_ensure_addlen (buf, src->len + extra + 1);
    if (unlikely (ret != ret_ok)) return ret;

    /* Escape and copy data to destination buffer.
     */
    if (p0 != src->buf) {
        len0 = (size_t) (p0 - src->buf);
        memcpy (&buf->buf[buf->len], src->buf, len0);
    }

    p2 = &buf->buf[buf->len + len0];

    for (p1 = p0; *p1 != (uint8_t) '\0'; ++p1) {
        switch ((char)*p1) {
        case '<':
            memcpy (p2, "&lt;", 4);
            p2 += 4;
            continue;

        case '>':
            memcpy (p2, "&gt;", 4);
            p2 += 4;
            continue;

        case '&':
            memcpy (p2, "&amp;", 5);
            p2 += 5;
            continue;

        case '"':
            memcpy (p2, "&quot;", 6);
            p2 += 6;
            continue;

        case '#':
            memcpy (p2, "&#35;", 5);
            p2 += 5;
            continue;

        case '\'':
            memcpy (p2, "&#39;", 5);
            p2 += 5;
            continue;

        case '/':
            memcpy (p2, "&#47;", 5);
            p2 += 5;
            continue;

        default:
            *p2++ = *p1;
            continue;
        }
    }

    /* Set the new length
     */
    buf->len += src->len + extra;
    buf->buf[buf->len] = (uint8_t) '\0';

    return ret_ok;
}


ret_t
chula_buffer_escape_html (chula_buffer_t *buf, chula_buffer_t *src)
{
    chula_buffer_clean (buf);
    return chula_buffer_add_escape_html (buf, src);
}


ret_t
chula_buffer_decode_base64 (chula_buffer_t *buf)
{
    uint32_t i;
    uint8_t  space[128];
    int      space_idx = 0;
    int      phase     = 0;
    int      d, prev_d = 0;
    int      buf_pos   = 0;

    if (chula_buffer_is_empty (buf))
        return ret_ok;

    /* Base-64 decoding: This represents binary data as printable
     * ASCII characters. Three 8-bit binary bytes are turned into
     * four 6-bit values, like so:
     *
     *   [11111111]  [22222222]  [33333333]
     *   [111111] [112222] [222233] [333333]
     *
     * Then the 6-bit values are represented using the characters
     * "A-Za-z0-9+/".
     */

    static const signed char
        b64_decode_tab[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

    for (i=0; i < buf->len; i++) {
        d = b64_decode_tab[buf->buf[i]];
        if (d != -1) {
            switch (phase) {
            case 0:
                ++phase;
                break;
            case 1:
                space[space_idx++] = (( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ));
                ++phase;
                break;
            case 2:
                space[space_idx++] = (( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ));
                ++phase;
                break;
            case 3:
                space[space_idx++] = (( ( prev_d & 0x03 ) << 6 ) | d );
                phase = 0;
                break;
            }
            prev_d = d;
        }

        if (space_idx == 127) {
            memcpy (buf->buf + buf_pos, space, 127);
            buf_pos += 127;
            space_idx = 0;
        }
    }

    space[space_idx]=(uint8_t) '\0';

    memcpy (buf->buf + buf_pos, space, space_idx+1);
    buf->len = buf_pos + space_idx;

    return ret_ok;
}


/* Encode base64 from source (buf) to destination (encoded).
 * NOTE: resulting (encoded) content is always longer than source (buf).
 * Source (buf) is not touched (rewritten or reallocated).
 */
ret_t
chula_buffer_encode_base64 (chula_buffer_t *buf, chula_buffer_t *encoded)
{
    uint8_t  *in;
    uint8_t  *out;
    ret_t     ret;
    uint32_t  i, j;
    uint32_t  inlen = buf->len;

    static const char base64tab[]=
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    if (chula_buffer_is_empty (buf))
        return ret_ok;

    /* Get memory
     */
    ret = chula_buffer_ensure_size (encoded, (buf->len+4)*4/3 + 1);
    if (unlikely (ret != ret_ok)) return ret;

    /* Cleanup destination buffer
     */
    chula_buffer_clean (encoded);

    /* Encode source to destination
     */
    in  = buf->buf;
    out = encoded->buf;

    for (i=0, j=0; i < inlen; i += 3) {
        int a=0,b=0,c=0;
        int d, e, f, g;

        a=in[i];
        b= i+1 < inlen ? in[i+1]:0;
        c= i+2 < inlen ? in[i+2]:0;

        d = base64tab [a >> 2 ];
        e = base64tab [((a & 3 ) << 4) | (b >> 4)];
        f = base64tab [((b & 15) << 2) | (c >> 6)];
        g = base64tab [c & 63 ];

        if (i + 1 >= inlen)
            f = (int) '=';
        if (i + 2 >= inlen)
            g = (int) '=';

        out[j++] = (uint8_t) d;
        out[j++] = (uint8_t) e;
        out[j++] = (uint8_t) f;
        out[j++] = (uint8_t) g;
    }

    out[j] = (uint8_t) '\0';
    encoded->len = j;

    return ret_ok;
}


/* Documentation:
 * RFC 1321, `The MD5 Message-Digest Algorithm'
 * http://www.alobbs.com/modules.php?op=modload&name=rfc&file=index&content_file=rfc1321.php
 *
 * The MD5 message-digest algorithm takes as input a message of
 * arbitrary length and produces as output a 128-bit "fingerprint" or
 * "message digest" of the input.
 */

ret_t
chula_buffer_encode_md5_digest (chula_buffer_t *buf)
{
    ret_t             ret;
    int               i;
    struct MD5Context context;
    unsigned char     digest[16];

    MD5Init (&context);
    MD5Update (&context, (md5byte *)buf->buf, buf->len);
    MD5Final (digest, &context);

    ret = chula_buffer_ensure_size (buf, 34);
    if (unlikely (ret != ret_ok)) return ret;

    for (i = 0; i < 16; ++i) {
        int tmp;

        tmp = ((digest[i] >> 4) & 0xf);
        buf->buf[i*2] = (uint8_t) TO_HEX(tmp);

        tmp = (digest[i] & 0xf);
        buf->buf[(i*2)+1] = (uint8_t) TO_HEX(tmp);
    }
    buf->buf[32] = (uint8_t) '\0';
    buf->len = 32;

    return ret_ok;
}


ret_t
chula_buffer_encode_md5 (chula_buffer_t *buf, chula_buffer_t *encoded)
{
    ret_t             ret;
    struct MD5Context context;

    ret = chula_buffer_ensure_size (encoded, 17);
    if (unlikely (ret != ret_ok)) return ret;

    MD5Init (&context);
    MD5Update (&context, (md5byte *)buf->buf, buf->len);
    MD5Final ((unsigned char *)encoded->buf, &context);

    encoded->buf[16] = (uint8_t) '\0';
    encoded->len = 16;

    return ret_ok;
}


/* Encode sha1, source buffer (buf) is not touched,
 * whereas destination buffer (encoded) is overwritten
 * but possibly not reallocated.
 */
ret_t
chula_buffer_encode_sha1 (chula_buffer_t *buf, chula_buffer_t *encoded)
{
    ret_t    ret;
    SHA_INFO sha1;

    sha_init (&sha1);
    sha_update (&sha1, (unsigned char*) buf->buf, buf->len);

    ret = chula_buffer_ensure_size (encoded, SHA1_DIGEST_SIZE + 1);
    if (unlikely (ret != ret_ok)) return ret;

    sha_final (&sha1, (unsigned char *) encoded->buf);

    encoded->len = SHA1_DIGEST_SIZE;
    encoded->buf[encoded->len] = (uint8_t) '\0';

    return ret_ok;
}


ret_t
chula_buffer_encode_sha1_digest (chula_buffer_t *buf)
{
    ret_t         ret;
    int           i;
    unsigned char digest[SHA1_DIGEST_SIZE];
    SHA_INFO      sha1;

    sha_init (&sha1);
    sha_update (&sha1, (unsigned char*) buf->buf, buf->len);
    sha_final (&sha1, digest);

    ret = chula_buffer_ensure_size (buf, (2 * SHA1_DIGEST_SIZE)+1);
    if (unlikely (ret != ret_ok)) return ret;

    for (i = 0; i < SHA1_DIGEST_SIZE; ++i) {
        int tmp;

        tmp = ((digest[i] >> 4) & 0xf);
        buf->buf[i*2] = (uint8_t) TO_HEX(tmp);

        tmp = (digest[i] & 0xf);
        buf->buf[(i*2)+1] = (uint8_t) TO_HEX(tmp);
    }

    buf->buf[2 * SHA1_DIGEST_SIZE] = (uint8_t) '\0';
    buf->len = 2 * SHA1_DIGEST_SIZE;

    return ret_ok;
}


/* Encode sha1 in base64, both source (buf) and destination (encoded)
 * buffers are overwritten, but possibly not reallocated.
 */
ret_t
chula_buffer_encode_sha1_base64 (chula_buffer_t *buf, chula_buffer_t *encoded)
{
    ret_t ret;

    /* Prepare destination buffer
     */
    ret = chula_buffer_ensure_size (encoded, (SHA1_DIGEST_SIZE * 2) + 1);
    if (unlikely (ret != ret_ok)) return ret;

    chula_buffer_clean (encoded);

    /* Encode sha1 + base64
     */
    chula_buffer_encode_sha1 (buf, encoded);
    chula_buffer_encode_base64 (encoded, buf);

    /* Copy result to destination buffer
     */
    chula_buffer_clean (encoded);

    ret = chula_buffer_add_buffer (encoded, buf);
    if (unlikely (ret != ret_ok)) return ret;

    return ret_ok;
}


ret_t
chula_buffer_encode_sha512 (chula_buffer_t *buf, chula_buffer_t *encoded)
{
    ret_t      ret;
    SHA512_CTX sha512;

    SHA512_Init (&sha512);
    SHA512_Update (&sha512, (void *)buf->buf, buf->len);

    ret = chula_buffer_ensure_size (encoded, SHA512_DIGEST_LENGTH + 1);
    if (unlikely (ret != ret_ok)) return ret;

    SHA512_Final (&sha512, (void *)encoded->buf);

    encoded->len = SHA512_DIGEST_LENGTH;
    encoded->buf[encoded->len] = (uint8_t) '\0';

    return ret_ok;
}


ret_t
chula_buffer_encode_sha512_digest (chula_buffer_t *buf)
{
    ret_t         ret;
    int           i;
    unsigned char digest[SHA512_DIGEST_LENGTH];
    SHA512_CTX    sha512;

    SHA512_Init   (&sha512);
    SHA512_Update (&sha512, (void *) buf->buf, buf->len);
    SHA512_Final  (&sha512, digest);

    ret = chula_buffer_ensure_size (buf, (2 * SHA512_DIGEST_LENGTH)+1);
    if (unlikely (ret != ret_ok)) return ret;

    for (i = 0; i < SHA512_DIGEST_LENGTH; ++i) {
        int tmp;

        tmp = ((digest[i] >> 4) & 0xf);
        buf->buf[i*2] = (uint8_t) TO_HEX(tmp);

        tmp = (digest[i] & 0xf);
        buf->buf[(i*2)+1] = (uint8_t) TO_HEX(tmp);
    }

    buf->buf[2 * SHA512_DIGEST_LENGTH] = (uint8_t) '\0';
    buf->len = 2 * SHA512_DIGEST_LENGTH;

    return ret_ok;
}

ret_t
chula_buffer_encode_sha512_base64 (chula_buffer_t *buf, chula_buffer_t *encoded)
{
    ret_t ret;

    /* Prepare destination buffer
     */
    ret = chula_buffer_ensure_size (encoded, (SHA512_DIGEST_LENGTH * 2) + 1);
    if (unlikely (ret != ret_ok)) return ret;

    chula_buffer_clean (encoded);

    /* Encode sha1 + base64
     */
    ret = chula_buffer_encode_sha512 (buf, encoded);
    if (unlikely (ret != ret_ok)) return ret;

    ret = chula_buffer_encode_base64 (encoded, buf);
    if (unlikely (ret != ret_ok)) return ret;

    /* Copy result to destination buffer
     */
    chula_buffer_clean (encoded);

    ret = chula_buffer_add_buffer (encoded, buf);
    if (unlikely (ret != ret_ok)) return ret;

    return ret_ok;
}


/* Encode in hexadecimal characters, source buffer (buf) is not touched,
 * whereas destination buffer (encoded) is overwritten
 * but possibly not reallocated.
 */
ret_t
chula_buffer_encode_hex (chula_buffer_t *buf, chula_buffer_t *encoded)
{
    ret_t    ret;
    uint8_t *in;
    uint8_t *out;
    uint32_t j;
    uint32_t i;
    uint32_t inlen = buf->len;

    /* Prepare destination buffer
     */
    ret = chula_buffer_ensure_size (encoded, (inlen * 2 + 1));
    if (unlikely (ret != ret_ok)) return ret;

    chula_buffer_clean (encoded);

    /* Encode source to destination
     */
    in  = buf->buf;
    out = encoded->buf;

    for (i = 0; i != inlen; ++i) {
        j = ( (*in >> 4) & 0xf );
        *out++ = (uint8_t) TO_HEX(j);

        j =   (*in++ & 0xf);
        *out++ = (uint8_t) TO_HEX(j);
    }

    *out = (uint8_t) '\0';
    encoded->len = (int) (inlen * 2);

    return ret_ok;
}


ret_t
chula_buffer_decode_hex (chula_buffer_t *buf)
{
    uint32_t i;

    static char hex_to_bin [128] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*            */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*            */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*            */
         0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,    /*   0..9     */
        -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*   A..F     */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*            */
        -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*   a..f     */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };  /*            */

    if (chula_buffer_is_empty (buf))
        return ret_ok;

    for (i=0; i<buf->len/2; i++) {
        /* It uses << 1 rather than * 2
         */
        int32_t b1 = buf->buf[(i << 1)] & 127;
        int32_t b2 = buf->buf[(i << 1) + 1] & 127;

        b1 = hex_to_bin[b1];
        b2 = hex_to_bin[b2];

        if ((b1 == -1) || (b2 == -1))
            break;

        buf->buf[i] = (uint8_t) (((b1 << 4) & 0xF0) | (b2 & 0x0F));
    }

    buf->len /= 2;
    buf->buf[buf->len] = (uint8_t) '\0';

    return ret_ok;
}


char
chula_buffer_end_char (chula_buffer_t *buf)
{
    if (chula_buffer_is_empty (buf))
        return '\0';

    return (char)buf->buf[buf->len-1];
}


ret_t
chula_buffer_replace_string (chula_buffer_t *buf,
                             const char *substring,   int substring_length,
                             const char *replacement, int replacement_length)
{
    int            remaining_length;
    int            result_length;
    uint8_t       *result;
    uint8_t       *result_position;
    const uint8_t *p;
    const uint8_t *substring_position;

    /* Verify formal parameters
     * (those which are not tested would raise a segment violation).
     * We keep the corner case of a NULL replacement string, since our
     * beloved chula_buffer_t's might be passed to us, and a
     * NULL buf->buf is legit for 0-sized buffers.
     */
    if (replacement == NULL) {
        if (unlikely (replacement_length != 0))
            return ret_deny;

        replacement = "";
    }

    if (chula_buffer_is_empty (buf))
        return ret_ok;

    if ((substring == NULL) ||
        (substring_length < 1) ||
        (replacement_length < 0))
    {
        return ret_deny;
    }

    /* Calculate the new size
     */
    result_length = buf->len;
    for (p = buf->buf; ; p = substring_position + substring_length) {
        substring_position = (uint8_t *) strstr ((char *)p, substring);

        if (substring_position == NULL)
            break;

        result_length += (replacement_length - substring_length);
    }

    /* If no substring has been found, then return now.
     */
    if (p == buf->buf)
        return ret_ok;

    /* If resulting length is zero, then return now.
     */
    if (result_length < 1) {
        buf->buf[0] = (uint8_t) '\0';
        buf->len = 0;
        return ret_ok;
    }

    /* Take the new memory chunk
     */
    result = (uint8_t *) malloc (result_length + 1);
    if (unlikely (result == NULL)) return ret_nomem;

    /* Build the new string
     */
    result_position = result;

    for (p = buf->buf; ; p = substring_position + substring_length) {
        substring_position = (uint8_t *) strstr ((char *)p, substring);

        if (substring_position == NULL) {
            remaining_length = strlen ((char*)p);
            memcpy (result_position, p, remaining_length);
            result_position += remaining_length;
            break;
        }
        memcpy (result_position, p, substring_position - p);
        result_position += (substring_position - p);

        memcpy (result_position, (const uint8_t *)replacement, replacement_length);
        result_position += replacement_length;
    }
    *result_position = '\0';

    /* Change the internal buffer content
     */
    free (buf->buf);

    buf->buf  = result;
    buf->len  = result_length;
    buf->size = result_length + 1;

    return ret_ok;
}


/* Substitute (substring)s found in (bufsrc) with (replacement)
 * and writes the resulting content to (bufdst).
 * NOTE: (bufdst) is written only if at least one (substring) instance
 *       is found in (bufsrc); in that case return value is ret_ok;
 *       if (substring) is NOT found in (bufsrc) then nothing is done
 *       in order to avoid an unnecessary copy of data.
 * Returns:
 *  ret_ok          bufdst has been written with the substitution string(s)
 *  ret_not_found   substring not found in bufsrc
 *  ret_deny        bad formal parameters
 *  ret_xxx         fatal error (failed allocation, etc.)
 */
ret_t
chula_buffer_substitute_string (chula_buffer_t *bufsrc,
                                chula_buffer_t *bufdst,
                                char *substring,   int substring_length,
                                char *replacement, int replacement_length)
{
    ret_t          ret;
    int            remaining_length;
    int            result_length;
    uint8_t       *result_position;
    const uint8_t *p;
    const uint8_t *substring_position;

    /* Verify formal parameters
     * (those which are not tested would raise a segment violation).
     */
    if (chula_buffer_is_empty (bufsrc))
        return ret_ok;

    if (substring == NULL || substring_length < 1)
        return ret_deny;

    if (unlikely (replacement == NULL)) {
        replacement = "";
    }

    /* Clean / reset destination buffer.
     */
    chula_buffer_clean (bufdst);

    /* Calculate the new size
     */
    result_length = bufsrc->len;
    for (p = bufsrc->buf; ; p = substring_position + substring_length) {
        substring_position = (uint8_t *) strstr((char *)p, substring);

        if (substring_position == NULL)
            break;

        result_length += (replacement_length - substring_length);
    }

    /* If no substring has been found, then return now.
     */
    if (p == bufsrc->buf)
        return ret_not_found;

    /* If resulting length is zero, then return now.
     */
    if (result_length < 1) {
        return ret_ok;
    }

    /* Preset size of destination buffer.
     */
    ret = chula_buffer_ensure_size (bufdst, result_length + 2);
    if (unlikely (ret != ret_ok)) return ret;

    /* Build the new string
     */
    result_position = bufdst->buf;

    for (p = bufsrc->buf; ; p = substring_position + substring_length) {
        substring_position = (uint8_t *) strstr ((char *)p, substring);

        if (substring_position == NULL) {
            remaining_length = (int) (&(bufsrc->buf[bufsrc->len]) - p);
            memcpy (result_position, p, remaining_length);
            result_position += remaining_length;
            break;
        }
        memcpy (result_position, p, substring_position - p);
        result_position += (int) (substring_position - p);

        memcpy (result_position, (const uint8_t *)replacement, replacement_length);
        result_position += replacement_length;
    }

    /* Terminate the destination buffer
     */
    *result_position = (uint8_t) '\0';
    bufdst->len  = result_length;

    return ret_ok;
}


ret_t
chula_buffer_add_comma_marks (chula_buffer_t *buf)
{
    ret_t    ret;
    uint32_t off, num, i;
    uint8_t *p;

    if ((buf->buf == NULL) || (buf->len <= 3))
        return ret_ok;

    num = buf->len / 3;
    off = buf->len % 3;

    ret = chula_buffer_ensure_size (buf, buf->len + num + 2);
    if (unlikely (ret != ret_ok)) return ret;

    if (off == 0) {
        p = buf->buf + 3;
        num--;
    } else {
        p = buf->buf + off;
    }

    for (i = 0; i < num; i++) {
        int len = (buf->buf + buf->len) - p;
        memmove(p+1, p, len);
        *p = (uint8_t) ',';
        p +=4;
        buf->len++;
    }

    buf->buf[buf->len] = (uint8_t) '\0';
    return ret_ok;
}


ret_t
chula_buffer_trim (chula_buffer_t *buf)
{
    uint32_t s, e;
    uint32_t len;

    if (chula_buffer_is_empty (buf))
        return ret_ok;

    for (s=0; s < buf->len; s++) {
        char c = (char) buf->buf[s];

        if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
            break;
    }

    for (e=0; e < (buf->len - s); e++) {
        char c = (char) buf->buf[buf->len-(e+1)];

        if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
            break;
    }

    len = buf->len - (s + e);

    memmove (buf->buf, buf->buf+s, len);

    buf->len = len;
    buf->buf[len] = (uint8_t) '\0';

    return ret_ok;
}


ret_t
chula_buffer_to_lowcase (chula_buffer_t *buf)
{
    char     c;
    uint32_t i;

    for (i=0; i<buf->len; i++) {
        c = (char) buf->buf[i];
        if ((c >= 'A') && (c <= 'Z')) {
            buf->buf[i] = (uint8_t) c + ('a'-'A');
        }
    }

    return ret_ok;
}


ret_t
chula_buffer_insert (chula_buffer_t *buf,
                     char           *txt,
                     size_t          txt_len,
                     size_t          pos)
{
    ret_t  ret;
    size_t posn;

    /* Sanity check */
    if ((txt == NULL) || (txt_len <= 0))
        return ret_ok;

    /* Memory allocation */
    ret = chula_buffer_ensure_size (buf, buf->len + txt_len + 1);
    if (unlikely (ret != ret_ok)) return ret;

    posn = MIN(pos, buf->len);

    /* Make room */
    memmove (buf->buf + posn + txt_len,
             buf->buf + posn,
             buf->len - posn);

    /* Insert the string */
    memcpy (buf->buf + posn, (uint8_t *)txt, txt_len);

    buf->len += txt_len;
    buf->buf[buf->len] = (uint8_t)'\0';

    return ret_ok;
}


ret_t
chula_buffer_insert_buffer (chula_buffer_t *buf,
                            chula_buffer_t *src,
                            size_t          pos)
{
    return chula_buffer_insert (buf, (char *)src->buf, src->len, pos);
}


ret_t
chula_buffer_split_lines (chula_buffer_t *buf,
                          int             columns,
                          const char     *indent)
{
    ret_t     ret;
    uint8_t  *p;
    uint8_t  *prev_space     = NULL;
    uint8_t  *latest_newline = NULL;
    int       since_prev     = 0;
    int       indent_len     = 0;

    if (indent) {
        indent_len = strlen(indent);
    }

    for (p = buf->buf; p < buf->buf + buf->len; p++) {
        since_prev += 1;

        if (*p != (uint8_t) ' ') {
            continue;
        }

        /* White found */
        if ((prev_space == NULL) || (since_prev <= columns)) {
            prev_space = p;
        }

        if (since_prev >= columns) {
            if (prev_space) {
                /* Split */
                *prev_space = (uint8_t) '\n';

                /* Reset */
                since_prev = (p - prev_space);
                latest_newline = prev_space;
                prev_space = NULL;
            } else {
                /* len(word) > columns */
                *p = (uint8_t) '\n';
                since_prev = 0;
                latest_newline = p;
            }

            /* Line just split */
            if (indent) {
                int offset = p - buf->buf;

                ret = chula_buffer_insert (buf, (char *)indent, indent_len,
                                           (latest_newline - buf->buf)+1);
                if (ret != ret_ok)
                    return ret;

                since_prev += indent_len;
                p = buf->buf + offset + indent_len;
                latest_newline = NULL;
            }
        }
    }

    return ret_ok;
}
