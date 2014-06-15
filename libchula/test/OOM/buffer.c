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

#include "oom.h"


int
buffer_new (void)
{
    ret_t           ret;
    chula_buffer_t *buf  = NULL;

    ret = chula_buffer_new (&buf);
    assert (((ret == ret_ok) && (buf != NULL)) ||
            ((ret == ret_nomem) && (buf == NULL)));

    chula_buffer_free(buf);
    return 0;
}

int
buffer_add (void)
{
    ret_t           ret;
    uint32_t        len;
    chula_buffer_t *buf  = NULL;

    WORK(chula_buffer_new (&buf));

    for (uint32_t n=0; n<20; n++) {
        len = buf->len;
        ret = chula_buffer_add_str (buf, "testing ");
        assert (((ret == ret_ok) && (buf->len > len)) ||
                ((ret == ret_nomem) && (buf->len <= len)));
    }

    for (uint32_t n=0; n<20; n++) {
        len = buf->len;
        ret = chula_buffer_add_char (buf, 'a');
        assert (((ret == ret_ok) && (buf->len > len)) ||
                ((ret == ret_nomem) && (buf->len <= len)));
    }

    for (uint32_t n=0; n<20; n++) {
        len = buf->len;
        ret = chula_buffer_add_long10 (buf, 123456789);
        assert (((ret == ret_ok) && (buf->len > len)) ||
                ((ret == ret_nomem) && (buf->len <= len)));
    }

    chula_buffer_free(buf);
    return 0;
}

int
buffer_dup (void)
{
    ret_t           ret;
    chula_buffer_t *buf  = NULL;

    WORK(chula_buffer_new (&buf));
    WORK(chula_buffer_add_str (buf, "testing "));

    for (uint32_t n=0; n<5; n++) {
        chula_buffer_t *dup = NULL;

        ret = chula_buffer_dup (buf, &dup);
        assert (((ret == ret_nomem) && (dup == NULL)) ||
                ((ret == ret_ok) && (dup != NULL) && (dup->buf != NULL)));

        chula_buffer_free (dup);
    }

    chula_buffer_free(buf);
    return 0;
}

int
buffer_operations (void)
{
    ret_t           ret;
    uint32_t        len;
    chula_buffer_t *buf  = NULL;

    WORK(chula_buffer_new (&buf));

    for (uint32_t n=2; n<12; n++) {
        WORK(chula_buffer_clean (buf));
        WORK(chula_buffer_add_str (buf, "abcdefghijklmopqrstuvwxyz"));

        len = buf->len;
        ret = chula_buffer_multiply (buf, n);
        assert (((ret == ret_ok) && (buf->len > len)) ||
                ((ret == ret_nomem) && (buf->len <= len)));
    }

    chula_buffer_free(buf);
    return 0;
}

int
buffer_encoders (void)
{
    ret_t           ret;
    chula_buffer_t *buf     = NULL;
    chula_buffer_t  encoded = CHULA_BUF_INIT;

#define ASSERT_ENCODED                                                             \
    assert (((ret == ret_ok)    && (encoded.len  > 0) && (encoded.buf != NULL)) || \
            ((ret == ret_nomem) && (encoded.len == 0) && (encoded.buf == NULL)))

    /* Base64 */
    WORK(chula_buffer_new (&buf));
    WORK(chula_buffer_add_str (buf, "abcdefghijklmopqrstuvwxyz"));

    ret = chula_buffer_encode_base64 (buf, &encoded);
    ASSERT_ENCODED;

    /* MD5 */
    WORK(chula_buffer_mrproper(&encoded));
    ret = chula_buffer_encode_md5 (buf, &encoded);
    ASSERT_ENCODED;

    /* SHA1 */
    WORK(chula_buffer_mrproper(&encoded));
    ret = chula_buffer_encode_sha1 (buf, &encoded);
    ASSERT_ENCODED;

    WORK(chula_buffer_mrproper(&encoded));
    ret = chula_buffer_encode_sha1_base64 (buf, &encoded);
    ASSERT_ENCODED;

    /* SHA512 */
    WORK(chula_buffer_mrproper(&encoded));
    ret = chula_buffer_encode_sha512 (buf, &encoded);
    ASSERT_ENCODED;

    chula_buffer_mrproper (&encoded);
    chula_buffer_free(buf);
    return 0;
}

int
buffer_repr (void)
{
    chula_buffer_t buf_A = CHULA_BUF_INIT;
    chula_buffer_t buf_B = CHULA_BUF_INIT;

    WORK(chula_buffer_add_str (&buf_A, "abcdefghijklmopqrstuvwxyz"));

    chula_buffer_repr (&buf_A, &buf_B);

    WORK(chula_buffer_mrproper(&buf_A));
    WORK(chula_buffer_mrproper(&buf_B));
    return 0;
}
