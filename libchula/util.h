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

#ifndef CHULA_UTIL_H
#define CHULA_UTIL_H

#if !defined(CHULA_H_INSIDE) && !defined (CHULA_COMPILATION)
# error "Only <libchula/libchula.h> can be included directly."
#endif

#include <libchula/common.h>
#include <libchula/buffer.h>

#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>


/* Error buffer size for chula_strerror_r().
 */
#define ERROR_MIN_BUFSIZE	64	/* min. buffer size */
#define ERROR_MAX_BUFSIZE	512	/* max. buffer size */
#define chula_error         errno

/* Missing functions
 */

/* Constants
 */
extern const char hex2dec_tab[256];
extern const char *month[13];

/* String management functions
 */
char   *chula_strerror_r     (int err, char *buf, size_t bufsize);
char   *chula_min_str        (char *s1, char *s2);
char   *chula_max_str        (char *s1, char *s2);
ret_t   chula_atoi           (const char *str, int *ret_value);
ret_t   chula_atol           (const char *str, long *ret_value);
ret_t   chula_atob           (const char *str, bool *ret_value);
int     chula_string_is_ipv6 (chula_buffer_t *ip);
ret_t   chula_malloc_size    (const void *ptr, size_t *size);

/* Files and Directories
 */
DIR * chula_opendir      (const char *dirname);
int   chula_readdir      (DIR *dirstream, struct dirent *entry, struct dirent **result);
int   chula_closedir     (DIR *dirstream);

int   chula_stat         (const char *path, struct stat *buf);
int   chula_lstat        (const char *path, struct stat *buf);
int   chula_fstat        (int filedes, struct stat *buf);
int   chula_access       (const char *pathname, int mode);
int   chula_open         (const char *path, int oflag, int mode);
int   chula_unlink       (const char *path);
int   chula_pipe         (int fildes[2]);

ret_t chula_mkstemp      (chula_buffer_t *buffer, int *fd);
ret_t chula_mkdtemp      (char *template);

ret_t chula_mkdir        (const char *path, int mode);
ret_t chula_mkdir_p      (chula_buffer_t *path, int mode);
ret_t chula_mkdir_p_perm (chula_buffer_t *dir_path, int create_mode, int ensure_perm);
ret_t chula_rm_rf        (chula_buffer_t *path, int uid);

/* File descriptor management
 */
ret_t chula_fd_set_nonblocking (int fd, bool enable);
ret_t chula_fd_set_nodelay     (int fd, bool enable);
ret_t chula_fd_set_closexec    (int fd);
ret_t chula_fd_set_reuseaddr   (int fd);
ret_t chula_fd_close           (int fd);

ret_t chula_sys_fdlimit_get (uint32_t *limit);
ret_t chula_sys_fdlimit_set (uint32_t  limit);

/* Users & Groups
 */
ret_t chula_getpwnam     (const char *name, struct passwd *pwbuf, char *buf, size_t buflen);
ret_t chula_getpwuid     (uid_t uid, struct passwd *pwbuf, char *buf, size_t buflen);
ret_t chula_getpwnam_uid (const char *name, struct passwd *pwbuf, char *buf, size_t buflen);
ret_t chula_getgrnam     (const char *name, struct group *pwbuf, char *buf, size_t buflen);
ret_t chula_getgrgid     (gid_t gid, struct group *pwbuf, char *buf, size_t buflen);
ret_t chula_getgrnam_gid (const char *name, struct group *pwbuf, char *buf, size_t buflen);

/* Time management functions
 */
struct tm *chula_gmtime           (const time_t *timep, struct tm *result);
struct tm *chula_localtime        (const time_t *timep, struct tm *result);
long      *chula_get_timezone_ref (void);

/* Path management
 */
ret_t chula_path_short     (chula_buffer_t *path);
ret_t chula_path_find_exec (const char *bin_name, chula_buffer_t *fullpath);

/* Network related utilities
 */
ret_t chula_gethostbyname (chula_buffer_t *hostname, struct addrinfo **addr);
ret_t chula_gethostname   (chula_buffer_t *buf);
ret_t chula_ntop          (int family, struct sockaddr *addr, char *dst, size_t cnt);

/* Random numbers
 */
void chula_random_seed (void);
long chula_random      (void);

/* Misc
 */
long  chula_eval_formated_time (chula_buffer_t *buf);
ret_t chula_buf_add_backtrace  (chula_buffer_t *buf, int n_skip, const char *new_line, const char *line_pre);

ret_t chula_syslog          (int priority, chula_buffer_t *buf);
ret_t chula_wait_pid        (pid_t pid, int *retcode);
ret_t chula_reset_signals   (void);
ret_t chula_get_shell       (const char **shell, const char **binary);
ret_t chula_tmp_dir_copy    (chula_buffer_t *buffer);

#endif /* CHULA_UTIL_H */
