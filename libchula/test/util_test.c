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

#include "libchula/testing_macros.h"
#include "libchula/util.h"


START_TEST (_strerror_r)
{
    char *s;
    int   re;
    char  tmp[256];

    /* 13: Permission denied */
    memset (tmp, 0, sizeof(tmp));
    s = chula_strerror_r (13, tmp, sizeof(tmp));
    ck_assert (s != NULL);

    re = memcmp (s, "Permission denied", strlen(s));
    ck_assert (re == 0);

    /* 98765 */
    memset (tmp, 0, sizeof(tmp));
    s = chula_strerror_r (98765, tmp, sizeof(tmp));
    ck_assert (s != NULL);

    re = memcmp (s, "Unknown error", 13);
    ck_assert (re == 0);
}
END_TEST

START_TEST (min_str)
{
    char *A = "a";
    char *B = "b";

    ck_assert (chula_min_str(NULL,NULL) == NULL);
    ck_assert (chula_min_str(A,NULL) == A);
    ck_assert (chula_min_str(NULL,A) == A);
    ck_assert (chula_min_str(A,B) != NULL);
}
END_TEST

START_TEST (max_str)
{
    char *A = "a";
    char *B = "b";

    ck_assert (chula_max_str(NULL,NULL) == NULL);
    ck_assert (chula_max_str(A,NULL) == A);
    ck_assert (chula_max_str(NULL,A) == A);
    ck_assert (chula_max_str(A,B) != NULL);
}
END_TEST

START_TEST (_atoi)
{
    int   re;
    ret_t ret;

    ret = chula_atoi (NULL, &re);
    ck_assert (ret == ret_error);

    ret = chula_atoi ("", &re);
    ck_assert (ret == ret_error);

    ret = chula_atoi ("123", &re);
    ck_assert (re == 123);
    ck_assert (ret == ret_ok);

    ret = chula_atoi ("-321", &re);
    ck_assert (re == -321);
    ck_assert (ret == ret_ok);
}
END_TEST

START_TEST (_atob)
{
    bool  re;
    ret_t ret;

    ret = chula_atob (NULL, &re);
    ck_assert (ret == ret_error);

    ret = chula_atob ("", &re);
    ck_assert (ret == ret_error);

    ret = chula_atob ("123", &re);
    ck_assert (re == true);
    ck_assert (ret == ret_ok);

    ret = chula_atob ("0", &re);
    ck_assert (re == false);
    ck_assert (ret == ret_ok);

    ret = chula_atob ("-0", &re);
    ck_assert (re == false);
    ck_assert (ret == ret_ok);

    ret = chula_atob ("-321", &re);
    ck_assert (re == true);
    ck_assert (ret == ret_ok);
}
END_TEST

START_TEST (is_ipv6)
{
    chula_buffer_t ip;

    chula_buffer_fake_str (&ip, "");
    ck_assert (chula_string_is_ipv6 (&ip) == 0);

    chula_buffer_fake_str (&ip, "127.0.0.1");
    ck_assert (chula_string_is_ipv6 (&ip) == 0);

    chula_buffer_fake_str (&ip, "::1");
    ck_assert (chula_string_is_ipv6 (&ip));

    chula_buffer_fake_str (&ip, "2001:0db8:0000:0000:0000:ff00:0042:8329");
    ck_assert (chula_string_is_ipv6 (&ip));
}
END_TEST

START_TEST (_dirs)
{
    int            re;
    DIR           *d;
    struct dirent  dentry;
    struct dirent *dresult;

    /* Open */
    d = chula_opendir ("/most/likely/it/does/not/exists");
    ck_assert (d == NULL);

    d = chula_opendir ("/");
    ck_assert (d != NULL);

    /* Walk */
    dresult = NULL;
    re = chula_readdir (d, &dentry, &dresult);
    ck_assert (re == 0);
    ck_assert (dresult != NULL);
    ck_assert (strlen(dresult->d_name) > 0);

    /* Close */
    chula_closedir (d);
}
END_TEST

START_TEST (_stat)
{
    int         re;
    struct stat st;

    re = chula_stat ("/it/does/not/exist", &st);
    ck_assert (re == -1);

    re = chula_stat ("/", &st);
    ck_assert (re == 0);
}
END_TEST

START_TEST (_lstat)
{
    int         re;
    struct stat st;

    re = chula_lstat ("/it/does/not/exist", &st);
    ck_assert (re == -1);

    re = chula_lstat ("/", &st);
    ck_assert (re == 0);
}
END_TEST

START_TEST (_fstat)
{
    int         re;
    int         fd;
    struct stat st;

    re = chula_fstat (-1, &st);
    ck_assert (re == -1);

    re = chula_fstat (63333, &st);
    ck_assert (re == -1);

    fd = chula_open ("/", O_RDONLY, 0);
    ck_assert (fd >= 0);

    re = chula_fstat (fd, &st);
    chula_fd_close(fd);
    ck_assert (re == 0);
}
END_TEST

START_TEST (_access)
{
    int re;

    re = chula_access ("/it/does/not/exist", R_OK);
    ck_assert (re == -1);

    re = chula_access ("/", R_OK);
    ck_assert (re == 0);
}
END_TEST

START_TEST (_unlink)
{
    int            re;
    int            fd;
    struct stat    st;
    chula_buffer_t path = CHULA_BUF_INIT;

    chula_tmp_dir_copy (&path);
    chula_buffer_add_str (&path, "/unlink.test.");
    chula_buffer_add_long10 (&path, getpid());

    /* It doesn't exist */
    ck_assert (chula_stat (path.buf, &st) == -1);

    /* Create file */
    fd = chula_open (path.buf, O_WRONLY|O_CREAT|O_TRUNC|O_NOFOLLOW, S_IRUSR|S_IWUSR);
    ck_assert (fd >= 0);
    chula_fd_close (fd);

    /* It exists */
    ck_assert (chula_stat (path.buf, &st) == 0);

    /* Unlink */
    re = chula_unlink (path.buf);
    ck_assert (re == 0);

    /* It mustn't exist */
    ck_assert (chula_stat (path.buf, &st) == -1);

    chula_buffer_mrproper (&path);
}
END_TEST

START_TEST (_pipe)
{
    int re;
    int fds[2];

    re = chula_pipe (fds);
    ck_assert (re == 0);
    ck_assert (fds[0] >= 0);
    ck_assert (fds[1] >= 0);

    chula_fd_close (fds[0]);
    chula_fd_close (fds[1]);
}
END_TEST

START_TEST (_mktemp)
{
    ret_t          ret;
    int            fd       = -1;
    chula_buffer_t filename = CHULA_BUF_INIT;

    chula_buffer_add_str (&filename, "/tmp/temp.XXXXXX");
    ret = chula_mkstemp (&filename, &fd);
    ck_assert (ret == ret_ok);
    ck_assert (fd >= 0);
    chula_fd_close (fd);

    chula_buffer_clean (&filename);
    chula_buffer_add_str (&filename, "/tmp/temp.XXXXXX");
    chula_mkdtemp (filename.buf);
    ck_assert (ret == ret_ok);

    chula_buffer_mrproper (&filename);
}
END_TEST

START_TEST (_mkdir)
{
    ret_t          ret;
    struct stat    st;
    chula_buffer_t path = CHULA_BUF_INIT;

    chula_tmp_dir_copy (&path);
    chula_buffer_add_va (&path, "/mkdir_pid%d/mkdir/1/22/333/4444", getpid());

    /* Path does not exists */
    ck_assert (chula_stat (path.buf, &st) == -1);

    /* Create dir */
    ret = chula_mkdir_p_perm (&path, 0777, R_OK);
    ck_assert (ret == ret_ok);

    /* Path does exists */
    ck_assert (chula_stat (path.buf, &st) == 0);

    /* Clean up */
    chula_buffer_mrproper (&path);

    chula_buffer_clean (&path);
    chula_tmp_dir_copy (&path);
    chula_buffer_add_va (&path, "/mkdir_pid%d", getpid());

    ret = chula_rm_rf (&path, -1);
    ck_assert (ret == ret_ok);
}
END_TEST


int
util_tests (void)
{
    Suite *s1 = suite_create("AVL");

    check_add (s1, _strerror_r);
    check_add (s1, min_str);
    check_add (s1, max_str);
    check_add (s1, _atoi);
    check_add (s1, _atob);
    check_add (s1, is_ipv6);
    check_add (s1, _dirs);
    check_add (s1, _stat);
    check_add (s1, _lstat);
    check_add (s1, _fstat);
    check_add (s1, _access);
    check_add (s1, _unlink);
    check_add (s1, _pipe);
    check_add (s1, _mktemp);
    check_add (s1, _mkdir);
    run_test (s1);
}
