/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/* All files in libchula are Copyright (C) 2014 Alvaro Lopez Ortega.
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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <libchula-qa/libchula-qa.h>
#include <libchula-qa/testing_macros-internal.h>


static bool
str_in_file (chula_buffer_t *filepath,
             const char     *string)
{
    ret_t          ret;
    bool           found;
    chula_buffer_t tmp    = CHULA_BUF_INIT;

    ret = chula_buffer_read_file (&tmp, (char *)filepath->buf);
    if (unlikely(ret != ret_ok)) return false;

    found = (chula_strnstr ((char *)tmp.buf, string, tmp.len) != NULL);

    chula_buffer_mrproper (&tmp);
    return found;
}

START_TEST (logger_fd)
{
    ret_t           ret;
    int             fd;
    chula_log_fd_t *logger = NULL;
    chula_buffer_t  path   = CHULA_BUF_INIT;

    /* Open fd */
    chula_tmp_dir_copy (&path);
    chula_buffer_add_str (&path, "/logger_fd.");
    chula_buffer_add_long10 (&path, getpid());

    fd = chula_open ((const char*)path.buf, O_WRONLY|O_CREAT|O_TRUNC|O_NOFOLLOW, S_IRUSR|S_IWUSR);
    ch_assert (fd >= 0);

    /* Logger */
    ret = chula_log_fd_new (&logger, fd);
    ch_assert (ret == ret_ok);

    ret = chula_log_init (HPACK_LOG(logger));
    ch_assert (ret == ret_ok);

    /* Testing */
    chula_log_info ("%d plus 1 equals %d\n", 2, 3);
    ch_assert (str_in_file (&path, "2 plus 1 equals 3"));

    chula_log_warn ("line %s\nline %s\nline %s", "one", "two", "three");
    ch_assert (str_in_file (&path, "line one"));
    ch_assert (str_in_file (&path, "line two"));
    ch_assert (str_in_file (&path, "line three"));

    chula_log_error ("this is an error");
    chula_log_debug ("and this a debug entry");
    chula_log_trace ("finally, trace");
    ch_assert (! str_in_file (&path, "errorand"));
    ch_assert (! str_in_file (&path, "entryfinally"));

    /* Testing errno */
    chula_log_errno (ERANGE, CHULA_LOG_DEBUG, "Ohoh, %s: ${errno}!", "surprise");
    ch_assert (str_in_file (&path, "Ohoh, surprise:"));
    ch_assert (! str_in_file (&path, "${errno}"));

    /* Report */
    chula_buffer_t tmp = CHULA_BUF_INIT;
    chula_buffer_read_file (&tmp, (char *)path.buf);
    chula_print_repr (chula, buffer, &tmp);
    chula_buffer_mrproper (&tmp);

    /* Shutdown */
    chula_log_shutdown();
    chula_fd_close (fd);

    /* Clean up */
    chula_unlink ((const char*)path.buf);
    chula_buffer_mrproper (&path);
}
END_TEST


int
log_tests (void)
{
    Suite *s1 = suite_create("Log");
    check_add (s1, logger_fd);
    run_test (s1);
}
