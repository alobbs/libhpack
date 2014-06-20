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

#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#include <libchula-qa/libchula-qa.h>
#include <libchula-qa/testing_macros-internal.h>

#ifndef LOG_INFO
# define LOG_INFO 6
#endif

START_TEST (_strerror_r)
{
    char *s;
    char  tmp[256];

    /* Buffer handling */
    s = chula_strerror_r (13, NULL, sizeof(tmp));
    ch_assert (s == NULL);

    s = chula_strerror_r (13, tmp, 0);
    ch_assert (s == NULL);

    /* -1 */
    memset (tmp, 0, sizeof(tmp));
    s = chula_strerror_r (-1, tmp, sizeof(tmp));
    ch_assert (s != NULL);
    ch_assert (strstr((const char*)s, "Unknown error") != NULL);

    /* 0 */
    memset (tmp, 0, sizeof(tmp));
    s = chula_strerror_r (0, tmp, sizeof(tmp));
    ch_assert (s != NULL);
    ch_assert ((strstr((const char*)s, "Success") != NULL) ||
               (strstr((const char*)s, "Undefined") != NULL));

    /* 13: Permission denied */
    memset (tmp, 0, sizeof(tmp));
    s = chula_strerror_r (13, tmp, sizeof(tmp));
    ch_assert (s != NULL);
    ch_assert (strstr((const char*)s, "Permission denied") != NULL);

    /* 98765 */
    memset (tmp, 0, sizeof(tmp));
    s = chula_strerror_r (98765, tmp, sizeof(tmp));
    ch_assert (s != NULL);
    ch_assert (strstr((const char*)s, "Unknown error") != NULL);
}
END_TEST

START_TEST (min_str)
{
    char *A = "a";
    char *B = "b";

    ch_assert (chula_min_str(NULL,NULL) == NULL);
    ch_assert (chula_min_str(A,NULL) == A);
    ch_assert (chula_min_str(NULL,A) == A);
    ch_assert (chula_min_str(A,B) != NULL);
}
END_TEST

START_TEST (max_str)
{
    char *A = "a";
    char *B = "b";

    ch_assert (chula_max_str(NULL,NULL) == NULL);
    ch_assert (chula_max_str(A,NULL) == A);
    ch_assert (chula_max_str(NULL,A) == A);
    ch_assert (chula_max_str(A,B) != NULL);
}
END_TEST

START_TEST (_atoi)
{
    int   re;
    ret_t ret;

    ret = chula_atoi (NULL, &re);
    ch_assert (ret == ret_error);

    ret = chula_atoi ("", &re);
    ch_assert (ret == ret_error);

    ret = chula_atoi ("123", &re);
    ch_assert (re == 123);
    ch_assert (ret == ret_ok);

    ret = chula_atoi ("-321", &re);
    ch_assert (re == -321);
    ch_assert (ret == ret_ok);
}
END_TEST

START_TEST (_atob)
{
    bool  re;
    ret_t ret;

    ret = chula_atob (NULL, &re);
    ch_assert (ret == ret_error);

    ret = chula_atob ("", &re);
    ch_assert (ret == ret_error);

    ret = chula_atob ("123", &re);
    ch_assert (re == true);
    ch_assert (ret == ret_ok);

    ret = chula_atob ("0", &re);
    ch_assert (re == false);
    ch_assert (ret == ret_ok);

    ret = chula_atob ("-0", &re);
    ch_assert (re == false);
    ch_assert (ret == ret_ok);

    ret = chula_atob ("-321", &re);
    ch_assert (re == true);
    ch_assert (ret == ret_ok);
}
END_TEST

START_TEST (is_ipv6)
{
    chula_buffer_t ip;

    chula_buffer_fake_str (&ip, "");
    ch_assert (chula_string_is_ipv6 (&ip) == 0);

    chula_buffer_fake_str (&ip, "127.0.0.1");
    ch_assert (chula_string_is_ipv6 (&ip) == 0);

    chula_buffer_fake_str (&ip, "::1");
    ch_assert (chula_string_is_ipv6 (&ip));

    chula_buffer_fake_str (&ip, "2001:0db8:0000:0000:0000:ff00:0042:8329");
    ch_assert (chula_string_is_ipv6 (&ip));
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
    ch_assert (d == NULL);

    d = chula_opendir ("/");
    ch_assert (d != NULL);

    /* Walk */
    dresult = NULL;
    re = chula_readdir (d, &dentry, &dresult);
    ch_assert (re == 0);
    ch_assert (dresult != NULL);
    ch_assert (strlen(dresult->d_name) > 0);

    /* Close */
    chula_closedir (d);
}
END_TEST

START_TEST (_stat)
{
    int         re;
    struct stat st;

    re = chula_stat ("/it/does/not/exist", &st);
    ch_assert (re == -1);

    re = chula_stat ("/", &st);
    ch_assert (re == 0);
}
END_TEST

START_TEST (_lstat)
{
    int         re;
    struct stat st;

    re = chula_lstat ("/it/does/not/exist", &st);
    ch_assert (re == -1);

    re = chula_lstat ("/", &st);
    ch_assert (re == 0);
}
END_TEST

START_TEST (_fstat)
{
    int         re;
    int         fd;
    struct stat st;

    re = chula_fstat (-1, &st);
    ch_assert (re == -1);

    re = chula_fstat (63333, &st);
    ch_assert (re == -1);

    fd = chula_open ("/", O_RDONLY, 0);
    ch_assert (fd >= 0);

    re = chula_fstat (fd, &st);
    chula_fd_close(fd);
    ch_assert (re == 0);
}
END_TEST

START_TEST (_access)
{
    int re;

    re = chula_access ("/it/does/not/exist", R_OK);
    ch_assert (re == -1);

    re = chula_access ("/", R_OK);
    ch_assert (re == 0);
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
    ch_assert (chula_stat ((const char *)path.buf, &st) == -1);

    /* Create file */
    fd = chula_open ((const char*)path.buf, O_WRONLY|O_CREAT|O_TRUNC|O_NOFOLLOW, S_IRUSR|S_IWUSR);
    ch_assert (fd >= 0);
    chula_fd_close (fd);

    /* It exists */
    ch_assert (chula_stat ((const char *)path.buf, &st) == 0);

    /* Unlink */
    re = chula_unlink ((const char*)path.buf);
    ch_assert (re == 0);

    /* It mustn't exist */
    ch_assert (chula_stat ((const char *)path.buf, &st) == -1);

    chula_buffer_mrproper (&path);
}
END_TEST

START_TEST (_pipe)
{
    int re;
    int fds[2];

    re = chula_pipe (fds);
    ch_assert (re == 0);
    ch_assert (fds[0] >= 0);
    ch_assert (fds[1] >= 0);

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
    ch_assert (ret == ret_ok);
    ch_assert (fd >= 0);
    chula_fd_close (fd);

    chula_buffer_clean (&filename);
    chula_buffer_add_str (&filename, "/tmp/temp.XXXXXX");
    chula_mkdtemp ((char*)filename.buf);
    ch_assert (ret == ret_ok);

    chula_buffer_mrproper (&filename);
}
END_TEST

START_TEST (_mkdir2)
{
    ret_t          ret;
    chula_buffer_t path = CHULA_BUF_INIT;

    ret = chula_mkdir_p_perm (NULL, 0777, R_OK);
    ch_assert (ret == ret_error);

    ret = chula_mkdir_p_perm (&path, 0777, R_OK);
    ch_assert (ret == ret_ok);
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
    ch_assert (chula_stat ((const char*)path.buf, &st) == -1);

    /* Create dir */
    ret = chula_mkdir_p_perm (&path, 0777, R_OK);
    ch_assert (ret == ret_ok);

    /* Path does exists */
    ch_assert (chula_stat ((const char*)path.buf, &st) == 0);

    /* Create a file (for rm_rf) */
    chula_buffer_clean (&path);
    chula_tmp_dir_copy (&path);
    chula_buffer_add_va (&path, "/mkdir_pid%d/file", getpid());

    fd = open ((const char*)path.buf, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    chula_fd_close (fd);

    /* rm -rf */
    chula_buffer_drop_ending (&path, 5);
    ret = chula_rm_rf (&path, -1);
    ch_assert (ret == ret_ok);

    /* Clean up */
    chula_buffer_mrproper (&path);
}
END_TEST

START_TEST (_fdlimit)
{
    ret_t        ret;
    unsigned int limit = 0;

    ret = chula_sys_fdlimit_get (&limit);
    ch_assert (ret == ret_ok);
    ch_assert (limit > 0);

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
    ch_assert (ret == ret_ok);
    ch_assert (pwd.pw_dir != NULL);
}
END_TEST

START_TEST (_getpwuid)
{
    ret_t         ret;
    struct passwd pwd;
    char          buffer[1024];

    ret = chula_getpwuid (0, &pwd, buffer, sizeof(buffer));
    ch_assert (ret == ret_ok);
    ch_assert (pwd.pw_dir != NULL);
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
    ch_assert (ret == ret_ok);
    ch_assert (pwd.pw_dir != NULL);

    /* Feed it an ID */
    memset (&pwd, 0, sizeof(pwd));
    ret = chula_getpwnam_uid ("0", &pwd, buffer, sizeof(buffer));
    ch_assert (ret == ret_ok);
    ch_assert (pwd.pw_dir != NULL);
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
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (grp.gr_name, group_name);
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
    ch_assert (ret == ret_ok);
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
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (grp.gr_name, group_name);

    /* Feed it an ID as string */
#ifdef OSX
    group_name = "20";
#else
    group_name = "0";
#endif

    grp.gr_gid  = -1;
    grp.gr_name = NULL;
    ret = chula_getgrnam_gid (group_name, &grp, buffer, sizeof(buffer));
    ch_assert (ret == ret_ok);
    ch_assert (grp.gr_name != NULL);
}
END_TEST

START_TEST (_gmtime)
{
    time_t    t;
    struct tm stm;

    memset (&stm, 0, sizeof(struct tm));

    t = time(NULL);
    chula_gmtime (&t, &stm);
    ch_assert (stm.tm_year + 1900 > 1900);
    ch_assert (stm.tm_year + 1900 < 2100);
}
END_TEST

START_TEST (_localtime)
{
    time_t    t;
    struct tm stm;

    memset (&stm, 0, sizeof(struct tm));

    t = time(NULL);
    chula_localtime (&t, &stm);
    ch_assert (stm.tm_year + 1900 > 1900);
    ch_assert (stm.tm_year + 1900 < 2100);
}
END_TEST

START_TEST (_timezone)
{
    const int  _12h1s = (12*60*60) + 1;
    long      *tz     = NULL;

    tz = chula_get_timezone_ref();
    ch_assert (tz != NULL);
    ch_assert (*tz > -_12h1s);
    ch_assert (*tz <  _12h1s);
}
END_TEST

START_TEST (path_short)
{
    ret_t          ret;
    chula_buffer_t path = CHULA_BUF_INIT;

    ret = chula_path_short (&path);
    ch_assert (ret == ret_ok);
    ch_assert (path.len == 0);

    chula_buffer_clean (&path);
    chula_buffer_add_str (&path, "/../");
    ret = chula_path_short (&path);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (path.buf, "/");

    chula_buffer_clean (&path);
    chula_buffer_add_str (&path, "/dir/second/../file");
    ret = chula_path_short (&path);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (path.buf, "/dir/file");

    chula_buffer_clean (&path);
    chula_buffer_add_str (&path, "/in/../1/2/../../a/b/c/../../d");
    ret = chula_path_short (&path);
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (path.buf, "/a/d");

    chula_buffer_mrproper (&path);
}
END_TEST

START_TEST (find_exec)
{
    ret_t          ret;
    chula_buffer_t path = CHULA_BUF_INIT;

    ret = chula_path_find_exec ("find", &path);
    ch_assert (ret == ret_ok);
    ch_assert (strstr((const char*)path.buf, "find") != NULL);

    chula_buffer_clean (&path);
    ret = chula_path_find_exec ("it_does_not_exist", &path);
    ch_assert (ret == ret_not_found);

    chula_buffer_mrproper (&path);
}
END_TEST

START_TEST (_gethostbyname)
{
    ret_t            ret;
    chula_buffer_t   host = CHULA_BUF_INIT;
    struct addrinfo *addr = NULL;

    ret = chula_gethostbyname (&host, &addr);
    ch_assert (ret == ret_error);

    chula_buffer_add_str (&host, "example.com");
    ret = chula_gethostbyname (&host, &addr);
    ch_assert (ret == ret_ok);
    ch_assert (addr != NULL);
    ch_assert ((addr->ai_family == AF_INET) ||
               (addr->ai_family == AF_INET6));

    chula_buffer_mrproper (&host);
}
END_TEST

START_TEST (_gethostname)
{
    ret_t          ret;
    chula_buffer_t name = CHULA_BUF_INIT;

    ret = chula_gethostname (&name);
    ch_assert (ret == ret_ok);
    ch_assert (! chula_buffer_is_empty (&name));

    chula_buffer_mrproper (&name);
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
    ch_assert (ret == ret_ok);
    ch_assert_str_eq (str, ip);
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

    ch_assert (! equal);
}
END_TEST

START_TEST (formated_time)
{
    chula_buffer_t s = CHULA_BUF_INIT;

    ch_assert (chula_eval_formated_time(&s) == 0);

    chula_buffer_fake_str (&s, "0");
    ch_assert (chula_eval_formated_time(&s) == 0);
    chula_buffer_fake_str (&s, "0s");
    ch_assert (chula_eval_formated_time(&s) == 0);
    chula_buffer_fake_str (&s, "0d");
    ch_assert (chula_eval_formated_time(&s) == 0);

    chula_buffer_fake_str (&s, "15");
    ch_assert (chula_eval_formated_time(&s) == 15);
    chula_buffer_fake_str (&s, "15s");
    ch_assert (chula_eval_formated_time(&s) == 15);
    chula_buffer_fake_str (&s, "15h");
    ch_assert (chula_eval_formated_time(&s) == 15*60*60);
    chula_buffer_fake_str (&s, "15w");
    ch_assert (chula_eval_formated_time(&s) == 15*60*60*24*7);

    chula_buffer_fake_str (&s, "111");
    ch_assert (chula_eval_formated_time(&s) == 111);
    chula_buffer_fake_str (&s, "111s");
    ch_assert (chula_eval_formated_time(&s) == 111);
    chula_buffer_fake_str (&s, "111m");
    ch_assert (chula_eval_formated_time(&s) == 111*60);
    chula_buffer_fake_str (&s, "111d");
    ch_assert (chula_eval_formated_time(&s) == 111*60*60*24);
}
END_TEST

START_TEST (_backtrace)
{
    ret_t          ret;
    chula_buffer_t s    = CHULA_BUF_INIT;

    ret = chula_buf_add_backtrace (&s, 0, "\n", "");
    ch_assert (ret == ret_ok);
    ch_assert (s.len > 0);
    ch_assert (strstr((const char*)s.buf, "main") != NULL);
    ch_assert (strstr((const char*)s.buf, "_backtrace") != NULL);

    chula_buffer_mrproper (&s);
}
END_TEST

START_TEST (get_shell)
{
    ret_t       ret;
    const char *shell = NULL;
    const char *bin   = NULL;

    ret = chula_get_shell (&shell, &bin);
    ch_assert (ret == ret_ok);
    ch_assert (shell != NULL);
    ch_assert (bin != NULL);
}
END_TEST

START_TEST (_wait_pid)
{
    ret_t  ret;
    int    retcode;
    pid_t  pid;

    pid = fork();
    ch_assert (pid >= 0);

    if (pid == 0) {
        chula_reset_signals();
        sleep(2);
        exit(123);
    }

    ret = chula_wait_pid (pid, &retcode);
    ch_assert ((ret == ret_ok) || (ret == ret_not_found));
    ch_assert (retcode == 123);
}
END_TEST

START_TEST (_wait_pid2)
{
    ret_t ret;
    int   retcode = 22;

    ret = chula_wait_pid (-11, &retcode);
    ch_assert (ret == ret_not_found);
    ch_assert (retcode == 22);
}
END_TEST

START_TEST (set_nodelay)
{
    ret_t ret;
    int   fd;

    /* Errors */
    ch_assert (chula_fd_set_nodelay (-1, true) == ret_error);

    /* Regular case */
    fd = socket (AF_INET, SOCK_STREAM, 0);
    ch_assert (fd >= 0);

    ret = chula_fd_set_nodelay (fd, true);
    ch_assert (ret == ret_ok);
    ch_assert (fcntl(fd, F_GETFL, 0) & O_NDELAY);

    ret = chula_fd_set_nodelay (fd, false);
    ch_assert (ret == ret_ok);
    ch_assert (fcntl(fd, F_GETFL, 0) & ~O_NDELAY);

    chula_fd_close(fd);
}
END_TEST

START_TEST (set_nonblocking)
{
    ret_t ret;
    int   fd;

    /* Errors */
    ch_assert (chula_fd_set_nonblocking(-1, true) == ret_error);

    /* Regular case */
    fd = socket (AF_INET, SOCK_STREAM, 0);
    ch_assert (fd >= 0);

    ret = chula_fd_set_nonblocking (fd, true);
    ch_assert (ret == ret_ok);
    ch_assert (fcntl(fd, F_GETFL, 0) & O_NONBLOCK);

    ret = chula_fd_set_nonblocking (fd, false);
    ch_assert (ret == ret_ok);
    ch_assert (fcntl(fd, F_GETFL, 0) & ~O_NONBLOCK);

    chula_fd_close(fd);
}
END_TEST

START_TEST (set_closexec)
{
    ret_t ret;
    int   fd;

    /* Errors */
    ch_assert (chula_fd_set_closexec(-1) == ret_error);

    /* Regular case */
    fd = socket (AF_INET, SOCK_STREAM, 0);
    ch_assert (fd >= 0);

    ret = chula_fd_set_closexec (fd);
    ch_assert (ret == ret_ok);
    ch_assert (fcntl(fd, F_GETFD, 0) & FD_CLOEXEC);

    chula_fd_close(fd);
}
END_TEST

START_TEST (set_reuseaddr)
{
    ret_t     ret;
    int       re;
    int       fd;
    int       val     = 0;
    socklen_t val_len = sizeof(val);

    /* Errors */
    ch_assert (chula_fd_set_reuseaddr(-1) == ret_error);

    /* Regular case */
    fd = socket (AF_INET, SOCK_STREAM, 0);
    ch_assert (fd >= 0);

    ret = chula_fd_set_reuseaddr (fd);
    ch_assert (ret == ret_ok);

    re = getsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &val, &val_len);
    ch_assert (re == 0);
    ch_assert (val != 0);
    ch_assert (val_len != 0);

    chula_fd_close(fd);
}
END_TEST

START_TEST (_syslog)
{
    ret_t          ret;
    chula_buffer_t info = CHULA_BUF_INIT;

    /* Empty */
    ret = chula_syslog (LOG_INFO, &info);
    ch_assert (ret == ret_ok);

    /* One liner */
    chula_buffer_add_str (&info, "one liner");
    ret = chula_syslog (LOG_INFO, &info);
    ch_assert (ret == ret_ok);

    /* Multiple lines */
    chula_buffer_clean (&info);
    chula_buffer_add_str (&info, "first line\nsecond one");
    ret = chula_syslog (LOG_INFO, &info);
    ch_assert (ret == ret_ok);

    chula_buffer_mrproper (&info);
}
END_TEST

START_TEST (_malloc_size)
{
    ret_t   ret;
    char   *ptr;
    size_t  size = 999;

    ptr = malloc (1234);
    ch_assert (ptr != NULL);

    ret = chula_malloc_size (ptr, &size);
    ch_assert (ret == ret_ok);
    ch_assert (size >= 1234);

    /* 'size' may be actually larger than requested, and so we should
     * make sure we don't go out of boundaries when reading and / or
     * writing at the beginning and end of the allocated memory. If by
     * any reason the size we got was wrong (bigger) this should raise
     * an exception (or error under valgrind).
     */
    ptr[0] = 0;
    ptr[1] = 1;
    ptr[2] = 2;
    ch_assert (ptr[0] == 0);
    ch_assert (ptr[1] == 1);
    ch_assert (ptr[2] == 2);

    ptr[size-3] = 7;
    ptr[size-2] = 8;
    ptr[size-1] = 9;
    ch_assert (ptr[size-3] == 7);
    ch_assert (ptr[size-2] == 8);
    ch_assert (ptr[size-1] == 9);
}
END_TEST


int
util_tests (void)
{
    Suite *s1 = suite_create("Utility Functions");

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
    check_add (s1, _mkdir2);
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
    check_add (s1, _wait_pid2);
    check_add (s1, set_nodelay);
    check_add (s1, set_nonblocking);
    check_add (s1, set_closexec);
    check_add (s1, set_reuseaddr);
    check_add (s1, _syslog);
    check_add (s1, _malloc_size);
    run_test (s1);
}
