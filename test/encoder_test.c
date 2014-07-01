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

#include <libhpack/libhpack.h>
#include <libchula-qa/libchula-qa.h>
#include <libchula-qa/testing_macros-internal.h>


START_TEST (init_mrproper) {
    ret_t                  ret;
    hpack_header_encoder_t enc;

    ret = hpack_header_encoder_init (&enc);
    ch_assert (ret == ret_ok);

    ret = hpack_header_encoder_mrproper (&enc);
    ch_assert (ret == ret_ok);
}
END_TEST

START_TEST (add) {
    ret_t                  ret;
    hpack_header_encoder_t enc;
    chula_buffer_t         name  = CHULA_BUF_INIT_FAKE("name");
    chula_buffer_t         value = CHULA_BUF_INIT_FAKE("value");

    hpack_header_encoder_init (&enc);

    ret = hpack_header_encoder_add (&enc, &name, &value);
    ch_assert (ret == ret_ok);

    hpack_header_encoder_mrproper (&enc);
}
END_TEST

START_TEST (encode1) {
    ret_t                  ret;
    hpack_header_encoder_t enc;
    chula_buffer_t         buf   = CHULA_BUF_INIT;
    chula_buffer_t         name  = CHULA_BUF_INIT_FAKE("name");
    chula_buffer_t         value = CHULA_BUF_INIT_FAKE("value");

    hpack_header_encoder_init (&enc);

    ret = hpack_header_encoder_add (&enc, &name, &value);
    ch_assert (ret == ret_ok);

    ret = hpack_header_encoder_render (&enc, &buf);
    ch_assert (ret == ret_ok);
    ch_assert (! chula_buffer_is_empty(&buf));

    hpack_header_encoder_mrproper (&enc);
    chula_buffer_mrproper (&buf);
}
END_TEST


int
basics (void)
{
    Suite *s1 = suite_create("Basic header encoding");
    check_add (s1, init_mrproper);
    check_add (s1, add);
    check_add (s1, encode1);
    run_test (s1);
}

int
header_encoding_tests (void)
{
    int re;

    re = basics();
    return re;
}
