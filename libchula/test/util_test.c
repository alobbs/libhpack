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

#include "config.h"
#include "libchula/testing_macros.h"
#include "libchula/util.h"


START_TEST (_strerror_r)
{
    char *s;
    char  tmp[256];

    /* -1 */
    memset (tmp, 0, sizeof(tmp));
    s = chula_strerror_r (-1, tmp, sizeof(tmp));
    ck_assert (s != NULL);
    ck_assert (strstr(s, "Unknown error") != NULL);

    /* 0 */
    memset (tmp, 0, sizeof(tmp));
    s = chula_strerror_r (0, tmp, sizeof(tmp));
    ck_assert (s != NULL);
    ck_assert (strstr(s, "Undefined") != NULL);

    /* 13: Permission denied */
    memset (tmp, 0, sizeof(tmp));
    s = chula_strerror_r (13, tmp, sizeof(tmp));
    ck_assert (s != NULL);
    ck_assert (strstr(s, "Permission denied") != NULL);

    /* 98765 */
    memset (tmp, 0, sizeof(tmp));
    s = chula_strerror_r (98765, tmp, sizeof(tmp));
    ck_assert (s != NULL);
    ck_assert (strstr(s, "Unknown error") != NULL);
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
    int            fd;
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

    /* Create a file (for rm_rf) */
    chula_buffer_clean (&path);
    chula_tmp_dir_copy (&path);
    chula_buffer_add_va (&path, "/mkdir_pid%d/file", getpid());

    fd = open (path.buf, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    chula_fd_close (fd);

    /* rm -rf */
    chula_buffer_drop_ending (&path, 5);
    ret = chula_rm_rf (&path, -1);
    ck_assert (ret == ret_ok);

    /* Clean up */
    chula_buffer_mrproper (&path);
}
END_TEST

START_TEST (_fdlimit)
{
    ret_t        ret;
    unsigned int limit = 0;

    ret = chula_sys_fdlimit_get (&limit);
    ck_assert (ret == ret_ok);
    ck_assert (limit > 0);

    /* Odds are it won't get away with it, so realisticly there is
     * nothing to be tested after this call */
    chula_sys_fdlimit_set (limit + 1);
}
END_TEST

START_TEST (_getpwnam)
{
    ret_t         ret;
    struct passwd pwd;
    char          buffer[1024];

    ret = chula_getpwnam ("root", &pwd, buffer, sizeof(buffer));
    ck_assert (ret == ret_ok);
    ck_assert (pwd.pw_dir != NULL);
}
END_TEST

START_TEST (_getpwuid)
{
    ret_t         ret;
    struct passwd pwd;
    char          buffer[1024];

    ret = chula_getpwuid (0, &pwd, buffer, sizeof(buffer));
    ck_assert (ret == ret_ok);
    ck_assert (pwd.pw_dir != NULL);
}
END_TEST

START_TEST (_getpwnam_uid)
{
    ret_t         ret;
    struct passwd pwd;
    char          buffer[1024];

    /* Feed it a name */
    memset (&pwd, 0, sizeof(pwd));
    ret = chula_getpwnam_uid ("root", &pwd, buffer, sizeof(buffer));
    ck_assert (ret == ret_ok);
    ck_assert (pwd.pw_dir != NULL);

    /* Feed it an ID */
    memset (&pwd, 0, sizeof(pwd));
    ret = chula_getpwnam_uid ("0", &pwd, buffer, sizeof(buffer));
    ck_assert (ret == ret_ok);
    ck_assert (pwd.pw_dir != NULL);
}
END_TEST

START_TEST (_getgrnam)
{
    ret_t         ret;
    struct group  grp;
    char          buffer[1024];
    char         *group_name;

#ifdef OSX
    group_name = "staff";
#else
    group_name = "root";
#endif

    grp.gr_gid = -1;
    ret = chula_getgrnam (group_name, &grp, buffer, sizeof(buffer));
    ck_assert (ret == ret_ok);
    ck_assert (grp.gr_gid >= 0);
    ck_assert_str_eq (grp.gr_name, group_name);
}
END_TEST

START_TEST (_getgrgid)
{
    ret_t        ret;
    struct group grp;
    char         buffer[1024];
    int          group_id;

#ifdef OSX
    /* staff */
    group_id = 20;
#else
    group_id = 0;
#endif

    grp.gr_gid = -1;
    ret = chula_getgrgid (group_id, &grp, buffer, sizeof(buffer));
    ck_assert (ret == ret_ok);
    ck_assert (grp.gr_gid >= 0);
}
END_TEST

START_TEST (_getgrnam_gid)
{
    ret_t         ret;
    struct group  grp;
    char          buffer[1024];
    char         *group_name;

    /* Feed it a name */
#ifdef OSX
    group_name = "staff";
#else
    group_name = "root";
#endif

    grp.gr_gid  = -1;
    grp.gr_name = NULL;
    ret = chula_getgrnam_gid (group_name, &grp, buffer, sizeof(buffer));
    ck_assert (ret == ret_ok);
    ck_assert (grp.gr_gid >= 0);
    ck_assert_str_eq (grp.gr_name, group_name);

    /* Feed it an ID as string */
#ifdef OSX
    group_name = "20";
#else
    group_name = "0";
#endif

    grp.gr_gid  = -1;
    grp.gr_name = NULL;
    ret = chula_getgrnam_gid (group_name, &grp, buffer, sizeof(buffer));
    ck_assert (ret == ret_ok);
    ck_assert (grp.gr_gid >= 0);
    ck_assert (grp.gr_name != NULL);
}
END_TEST

START_TEST (_gmtime)
{
    time_t    t;
    struct tm stm;

    memset (&stm, 0, sizeof(struct tm));

    t = time(NULL);
    chula_gmtime (&t, &stm);
    ck_assert (stm.tm_year + 1900 > 1900);
    ck_assert (stm.tm_year + 1900 < 2100);
}
END_TEST

START_TEST (_localtime)
{
    time_t    t;
    struct tm stm;

    memset (&stm, 0, sizeof(struct tm));

    t = time(NULL);
    chula_localtime (&t, &stm);
    ck_assert (stm.tm_year + 1900 > 1900);
    ck_assert (stm.tm_year + 1900 < 2100);
}
END_TEST

START_TEST (_timezone)
{
    const int  _12h1s = (12*60*60) + 1;
    long      *tz     = NULL;

    tz = chula_get_timezone_ref();
    ck_assert (tz != NULL);
    ck_assert (*tz > -_12h1s);
    ck_assert (*tz <  _12h1s);
}
END_TEST

START_TEST (path_short)
{
    ret_t          ret;
    chula_buffer_t path = CHULA_BUF_INIT;

    ret = chula_path_short (&path);
    ck_assert (ret == ret_ok);
    ck_assert (path.len == 0);

    chula_buffer_clean (&path);
    chula_buffer_add_str (&path, "/../");
    ret = chula_path_short (&path);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (path.buf, "/");

    chula_buffer_clean (&path);
    chula_buffer_add_str (&path, "/dir/second/../file");
    ret = chula_path_short (&path);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (path.buf, "/dir/file");

    chula_buffer_clean (&path);
    chula_buffer_add_str (&path, "/in/../1/2/../../a/b/c/../../d");
    ret = chula_path_short (&path);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (path.buf, "/a/d");

    chula_buffer_mrproper (&path);
}
END_TEST

START_TEST (find_exec)
{
    ret_t          ret;
    chula_buffer_t path = CHULA_BUF_INIT;

    ret = chula_path_find_exec ("find", &path);
    ck_assert (ret == ret_ok);
    ck_assert (strstr(path.buf, "find") != NULL);

    chula_buffer_clean (&path);
    ret = chula_path_find_exec ("it_does_not_exist", &path);
    ck_assert (ret == ret_not_found);

    chula_buffer_mrproper (&path);
}
END_TEST

START_TEST (_gethostbyname)
{
    ret_t            ret;
    chula_buffer_t   host = CHULA_BUF_INIT;
    struct addrinfo *addr = NULL;

    ret = chula_gethostbyname (&host, &addr);
    ck_assert (ret == ret_error);

    chula_buffer_add_str (&host, "example.com");
    ret = chula_gethostbyname (&host, &addr);
    ck_assert (ret == ret_ok);
    ck_assert (addr->ai_family == 2);

    chula_buffer_mrproper (&host);
}
END_TEST

START_TEST (_gethostname)
{
    ret_t          ret;
    chula_buffer_t name = CHULA_BUF_INIT;

    ret = chula_gethostname (&name);
    ck_assert (ret == ret_ok);
    ck_assert (! chula_buffer_is_empty (&name));
}
END_TEST

START_TEST (_ntop)
{
    ret_t            ret;
    const char      *ip  = "1.2.3.4";
    struct sockaddr  s;
    char             str[INET_ADDRSTRLEN];

    /* Set address */
    inet_pton (AF_INET, ip, &((struct sockaddr_in *)(&s))->sin_addr);

    /* Read it */
    ret = chula_ntop (AF_INET, &s, str, INET_ADDRSTRLEN);
    ck_assert (ret == ret_ok);
    ck_assert_str_eq (str, ip);
}
END_TEST

START_TEST (_random)
{
    bool equal;

    chula_random_seed();

    equal = true;
    for (int n=0; n<5; n++) {
        equal &= (chula_random() == chula_random());
    }

    ck_assert (! equal);
}
END_TEST

START_TEST (formated_time)
{
    chula_buffer_t s = CHULA_BUF_INIT;

    ck_assert (chula_eval_formated_time(&s) == 0);

    chula_buffer_fake_str (&s, "0");
    ck_assert (chula_eval_formated_time(&s) == 0);
    chula_buffer_fake_str (&s, "0s");
    ck_assert (chula_eval_formated_time(&s) == 0);
    chula_buffer_fake_str (&s, "0d");
    ck_assert (chula_eval_formated_time(&s) == 0);

    chula_buffer_fake_str (&s, "15");
    ck_assert (chula_eval_formated_time(&s) == 15);
    chula_buffer_fake_str (&s, "15s");
    ck_assert (chula_eval_formated_time(&s) == 15);
    chula_buffer_fake_str (&s, "15h");
    ck_assert (chula_eval_formated_time(&s) == 15*60*60);
    chula_buffer_fake_str (&s, "15w");
    ck_assert (chula_eval_formated_time(&s) == 15*60*60*24*7);

    chula_buffer_fake_str (&s, "111");
    ck_assert (chula_eval_formated_time(&s) == 111);
    chula_buffer_fake_str (&s, "111s");
    ck_assert (chula_eval_formated_time(&s) == 111);
    chula_buffer_fake_str (&s, "111m");
    ck_assert (chula_eval_formated_time(&s) == 111*60);
    chula_buffer_fake_str (&s, "111d");
    ck_assert (chula_eval_formated_time(&s) == 111*60*60*24);
}
END_TEST

START_TEST (_backtrace)
{
    ret_t          ret;
    chula_buffer_t s    = CHULA_BUF_INIT;

    ret = chula_buf_add_backtrace (&s, 0, "\n", "");
    ck_assert (ret == ret_ok);
    ck_assert (s.len > 0);
    ck_assert (strstr(s.buf, "main") != NULL);
    ck_assert (strstr(s.buf, "_backtrace") != NULL);

    chula_buffer_mrproper (&s);
}
END_TEST

START_TEST (get_shell)
{
    ret_t       ret;
    const char *shell = NULL;
    const char *bin   = NULL;

    ret = chula_get_shell (&shell, &bin);
    ck_assert (ret == ret_ok);
    ck_assert (shell != NULL);
    ck_assert (bin != NULL);
}
END_TEST

START_TEST (_wait_pid)
{
    ret_t  ret;
    int    retcode;
    pid_t  pid;

    pid = fork();
    ck_assert (pid >= 0);

    if (pid == 0) {
        chula_reset_signals();
        sleep(2);
        exit(123);
    }

    ret = chula_wait_pid (pid, &retcode);
    ck_assert ((ret == ret_ok) || (ret == ret_not_found));
    ck_assert (retcode == 123);
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
    check_add (s1, _fdlimit);
    check_add (s1, _getpwnam);
    check_add (s1, _getpwuid);
    check_add (s1, _getpwnam_uid);
    check_add (s1, _getgrnam);
    check_add (s1, _getgrgid);
    check_add (s1, _getgrnam_gid);
    check_add (s1, _gmtime);
    check_add (s1, _localtime);
    check_add (s1, _timezone);
    check_add (s1, path_short);
    check_add (s1, find_exec);
    check_add (s1, _gethostbyname);
    check_add (s1, _gethostname);
    check_add (s1, _ntop);
    check_add (s1, _random);
    check_add (s1, formated_time);
    check_add (s1, _backtrace);
    check_add (s1, get_shell);
    check_add (s1, _wait_pid);
    run_test (s1);
}
