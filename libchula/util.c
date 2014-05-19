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
#include "util.h"
#include "log.h"
#include "cstrings.h"
#include "chula-ret.h"

#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#else
# include <time.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif

#ifdef HAVE_SYS_FILIO_H
# include <sys/filio.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif

#if defined (HAVE_SYS_RESOURCE_H)
# include <sys/resource.h>
#elif defined (HAVE_RESOURCE_H)
# include <resource.h>
#endif

#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif

#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif

#ifdef HAVE_EXECINFO_H
# include <execinfo.h>
#endif

#ifdef HAVE_SYS_UTSNAME_H
# include <sys/utsname.h>
#endif

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_SYS_UIO_H
# include <sys/uio.h>
#endif

#ifdef HAVE_PATHS_H
# include <paths.h>
#endif

#if defined (HAVE_MALLOC_H)
# include <malloc.h>
#elif defined (HAVE_MALLOC_MALLOC_H)
# include <malloc/malloc.h>
#endif

#ifndef HOST_NAME_MAX
# define HOST_NAME_MAX 255
#endif

#ifndef NSIG
# define NSIG 32
#endif

#define ENTRIES "util"

const char *chula_version    = PACKAGE_VERSION;


const char hex2dec_tab[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 00-0F */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 10-1F */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 20-2F */
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,  /* 30-3F */
    0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 40-4F */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 50-5F */
    0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 60-6F */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 70-7F */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 80-8F */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 90-9F */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* A0-AF */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* B0-BF */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* C0-CF */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* D0-DF */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* E0-EF */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   /* F0-FF */
};

const char *month[13] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    NULL
};


/* Given an error number (errno) it returns an error string.
 * Parameters "buf" and "bufsize" are passed by caller
 * in order to make the function "thread safe".
 * If the error number is unknown
 * then an "Unknown error nnn" string is returned.
 */
char *
chula_strerror_r (int err, char *buf, size_t bufsize)
{
    if (buf == NULL)
        return NULL;

    if (bufsize < ERROR_MIN_BUFSIZE)
        return NULL;

#ifdef HAVE_GLIBC_STRERROR_R
    char *re = strerror_r (err, buf, bufsize);
    if (re != NULL)
        return re;
#elif defined HAVE_POSIX_STRERROR_R
    int re = strerror_r (err, buf, bufsize);
    if (re == 0)
        return buf;
#else
    char *p;
    p = strerror(err);
    if (p != NULL)
        return p;
#endif

    /* Unhandled error
     */
    buf[0] = '\0';
    snprintf (buf, bufsize, "Unknown error %d (errno)", err);
    buf[bufsize-1] = '\0';
    return buf;
}


char *
chula_min_str (char *s1, char *s2)
{
    if ((s1 == NULL) &&
        (s2 == NULL)) return NULL;

    if ((s1 != NULL) &&
        (s2 == NULL)) return s1;

    if ((s2 != NULL) &&
        (s1 == NULL)) return s2;

    return (s1<s2) ? s1 : s2;
}


char *
chula_max_str (char *s1, char *s2)
{
    if ((s1 == NULL) &&
        (s2 == NULL)) return NULL;

    if ((s1 != NULL) &&
        (s2 == NULL)) return s1;

    if ((s2 != NULL) &&
        (s1 == NULL)) return s2;

    return (s1>s2) ? s1 : s2;
}

ret_t
chula_atoi (const char *str, int *ret_value)
{
    int   tmp;
    char *endptr = NULL;

    if (str == NULL) {
        return ret_error;
    }

    errno = 0;
    tmp = strtol (str, &endptr, 10);
    if (errno != 0) {
        return ret_error;
    }

    if (str == endptr) {
        /* No digits were found */
        return ret_error;
    }

    *ret_value = tmp;
    return ret_ok;
}


ret_t
chula_atol (const char *str, long *ret_value)
{
    long  tmp;
    char *endptr = NULL;

    if (str == NULL) {
        return ret_error;
    }

    errno = 0;
    tmp = strtol (str, &endptr, 10);
    if (errno != 0) {
        return ret_error;
    }

    if (str == endptr) {
        /* No digits were found */
        return ret_error;
    }

    *ret_value = tmp;
    return ret_ok;
}


ret_t
chula_atob (const char *str, bool *ret_value)
{
    ret_t ret;
    int   tmp;

    if (str == NULL) {
        return ret_error;
    }

    ret = chula_atoi (str, &tmp);
    if (ret != ret_ok) {
        return ret;
    }

    *ret_value = !!tmp;
    return ret_ok;
}

int
chula_string_is_ipv6 (chula_buffer_t *ip)
{
    uint32_t i;
    uint32_t colons = 0;

    for (i=0; i<ip->len; i++) {
        if (ip->buf[i] == (uint8_t) ':') {
            colons += 1;
            if (colons == 2)
                return 1;
        }
    }

    return 0;
}


/* Files and Directories
 */


DIR *
chula_opendir (const char *dirname)
{
    DIR *re;

    do {
        re = opendir (dirname);
    } while ((re == NULL) && (errno == EINTR));

    return re;
}

#if !defined(HAVE_READDIR_R)
static pthread_mutex_t readdir_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/* The readdir subroutine is reentrant when an application program
 * uses different DirectoryPointer parameter values (returned from the
 * opendir subroutine). Use the readdir_r subroutine when multiple
 * threads use the same directory pointer.
 */

int
chula_readdir (DIR *dirstream, struct dirent *entry, struct dirent **result)
{
#ifndef HAVE_READDIR
# warning "readdir() unimplemented"
    return ENOSYS;
#else
# ifdef HAVE_READDIR_R_2
    /* We cannot rely on the return value of readdir_r as it
     * differs between various platforms (HPUX returns 0 on
     * success whereas Solaris returns non-zero)
     */
    entry->d_name[0] = '\0';

    do {
        errno = 0;
        readdir_r (dirstream, entry);
    } while (errno == EINTR);

    if (entry->d_name[0] != '\0') {
        *result = entry;
        return 0;
    }

    *result = NULL;
    return errno;

# elif defined(HAVE_READDIR_R_3)
    return readdir_r (dirstream, entry, result);
# else
    struct dirent *ptr;
    int            ret = 0;

    CHULA_MUTEX_LOCK (&readdir_mutex);

    do {
        errno = 0;
        ptr = readdir (dirstream);
    } while ((ptr == NULL) && (errno == EINTR));

    if ((ptr == NULL) && (errno != 0))
        ret = errno;

    if (ptr)
        memcpy(entry, ptr, sizeof(*ptr));

    *result = ptr;

    CHULA_MUTEX_UNLOCK (&readdir_mutex);

    return ret;

# endif
#endif
}

int
chula_closedir (DIR *dirstream)
{
    int re;

    do {
        re = closedir (dirstream);
    } while ((re < 0) && (errno == EINTR));

    return re;
}

int
chula_stat (const char *path, struct stat *buf)
{
    int re;

    do {
        re = stat (path, buf);
    } while ((re == -1) && (errno == EINTR));

    return re;
}

int
chula_lstat (const char *path, struct stat *buf)
{
    int re;

    do {
        re = lstat (path, buf);
    } while ((re == -1) && (errno == EINTR));

    return re;
}

int
chula_fstat (int filedes, struct stat *buf)
{
    int re;

    do {
        re = fstat (filedes, buf);
    } while ((re == -1) && (errno == EINTR));

    return re;
}

int
chula_access (const char *pathname, int mode)
{
    int re;

    do {
        re = access (pathname, mode);
    } while ((re == -1) && (errno == EINTR));

    return re;
}

int
chula_open (const char *path, int oflag, int mode)
{
    int re;

    do {
        re = open (path, oflag, mode);
    } while ((re < 0) && (errno == EINTR));

    return re;
}

int
chula_unlink (const char *path)
{
    int re;

    do {
        re = unlink (path);
    } while ((re < 0) && (errno == EINTR));

    return re;
}

int
chula_pipe (int fildes[2])
{
    int re;

    do {
        re = pipe (fildes);
    } while ((re < 0) && (errno == EINTR));

    return re;
}

ret_t
chula_mkstemp (chula_buffer_t *buffer, int *fd)
{
    int re;

    re = mkstemp ((char *)buffer->buf);
    if (re < 0) return ret_error;

    *fd = re;
    return ret_ok;
}

ret_t
chula_mkdtemp (char *template)
{
    char *re;

    re = mkdtemp (template);
    if (unlikely (re == NULL)) {
        return ret_error;
    }

    return ret_ok;
}

ret_t
chula_mkdir (const char *path, int mode)
{
    int re;

    do {
        re = mkdir (path, mode);
    } while ((re < 0) && (errno == EINTR));

	return re;
}

ret_t
chula_mkdir_p (chula_buffer_t *path, int mode)
{
    int          re;
    uint8_t     *p;
    int          err;
    struct stat  foo;

	/* There is no directory
	 */
    if (unlikely (path == NULL)) {
        return ret_error;
    }
	if (unlikely (chula_buffer_is_empty (path))) {
		return ret_ok;
	}

	/* Check whether the directory exists
	 */
	re = chula_stat ((char *)path->buf, &foo);
	if (re == 0) {
		return ret_ok;
	}

	/* Create the directory tree
	 */
	p = path->buf;
	while (true) {
		p = (uint8_t *) strchr ((char *)p+1, '/');
		if (p == NULL)
			break;

		*p = (uint8_t) '\0';

		re = chula_stat ((char *)path->buf, &foo);
		if (re != 0) {
			re = chula_mkdir ((char *)path->buf, mode);
			if ((re != 0) && (errno != EEXIST)) {
				err = errno;
				*p = (uint8_t) '/';

                chula_log_errno (err, CHULA_LOG_ERROR,
                                 "Could not mkdir '%s' (UID %d): ${errno}",
                                 path->buf, getuid());
				return ret_error;
			}
		}

		*p = (uint8_t) '/';

		p++;
		if (p > path->buf + path->len)
			return ret_ok;
	}

	re = chula_mkdir ((char *)path->buf, mode);
	if ((re != 0) && (errno != EEXIST)) {
		err = errno;

        chula_log_errno (err, CHULA_LOG_ERROR,
                         "Could not mkdir '%s' (UID %d): ${errno}",
                         path->buf, getuid());
		return ret_error;
	}

	return ret_ok;
}

ret_t
chula_mkdir_p_perm (chula_buffer_t *dir_path,
                    int             create_mode,
                    int             ensure_perm)
{
	int         re;
	ret_t       ret;
	struct stat foo;

    /* Sanity checks
     */
    if (unlikely (dir_path == NULL)) {
        return ret_error;
    }
	if (unlikely (chula_buffer_is_empty (dir_path))) {
		return ret_ok;
	}

	/* Does it exist?
	 */
	re = chula_stat ((char *)dir_path->buf, &foo);
	if (re != 0) {
		/* Create the directory
		 */
		ret = chula_mkdir_p (dir_path, create_mode);
		if (ret != ret_ok) {
			return ret_error;
		}
	}

	/* Check permissions
	 */
	re = chula_access ((char *)dir_path->buf, ensure_perm);
	if (re != 0) {
		return ret_deny;
	}

	return ret_ok;
}

ret_t
chula_rm_rf (chula_buffer_t *path,
             int             only_uid)
{
	int             re;
    ret_t           ret;
	DIR            *d;
	struct dirent  *entry;
	char            entry_buf[512];
	struct stat     info;
	chula_buffer_t  tmp = CHULA_BUF_INIT;

	/* Remove the directory contents
	 */
	d = chula_opendir ((char *)path->buf);
	if (d == NULL) {
		return ret_ok;
	}

	while (true) {
		re = chula_readdir (d, (struct dirent *)entry_buf, &entry);
		if ((re != 0) || (entry == NULL))
			break;

        if (!strncmp (entry->d_name, ".",  1)) continue;
        if (!strncmp (entry->d_name, "..", 2)) continue;

		chula_buffer_clean (&tmp);

		ret = chula_buffer_add_buffer (&tmp, path);
        if (unlikely (ret != ret_ok)) goto error;
		ret = chula_buffer_add_char (&tmp, '/');
        if (unlikely (ret != ret_ok)) goto error;
		ret = chula_buffer_add (&tmp, entry->d_name, strlen(entry->d_name));
        if (unlikely (ret != ret_ok)) goto error;

        re = chula_stat ((char *)tmp.buf, &info);
        if (re != 0) continue;

		if (only_uid != -1) {
			if (info.st_uid != (uid_t)only_uid)
				continue;
		}

		if (S_ISDIR (info.st_mode)) {
			chula_rm_rf (&tmp, only_uid);
			TRACE (ENTRIES, "Removing dir: %s\n", (char *)tmp.buf, re);
		} else if (S_ISREG (info.st_mode)) {
			re = unlink ((char *)tmp.buf);
			TRACE (ENTRIES, "Removing file: %s, re=%d\n", (char *)tmp.buf, re);
		}
	}

	chula_closedir (d);

	/* It should be empty by now
	 */
	re = rmdir ((char *)path->buf);
	TRACE (ENTRIES, "Removing top level directory: %s, re=%d\n", (char *)path->buf, re);

	/* Clean up
	 */
	chula_buffer_mrproper (&tmp);
	return ret_ok;

error:
	chula_closedir (d);
	chula_buffer_mrproper (&tmp);
	return ret_error;
}



/* File Descriptors
 */


ret_t
chula_fd_set_nodelay (int fd, bool enable)
{
	int re;
	int flags = 0;

	/* Disable the Nagle algorithm. This means that segments are
     * always sent as soon as possible, even if there is only a
     * small amount of data. When not set, data is buffered until
     * there is a sufficient amount to send out, thereby avoiding
     * the frequent sending of small packets, which results in
     * poor utilization of the network.
	 */

#ifdef FIONBIO
    bool e = enable;

	/* Even though the right thing to do would be to use POSIX's
	 * O_NONBLOCK, we are using FIONBIO here. It requires a single
	 * syscall, while using O_NONBLOCK would require us to call
	 * fcntl(F_GETFL) and fcntl(F_SETFL, O_NONBLOCK)
	 */
	re = ioctl (fd, FIONBIO, &e);

#else
	/* Use POSIX's O_NONBLOCK
	 */
 	flags = fcntl (fd, F_GETFL, 0);
	if (unlikely (flags == -1)) {
        chula_log_errno (errno, CHULA_LOG_ERROR, "fcntl (F_GETFL, fd=%d, 0): ${errno}", fd);
		return ret_error;
	}

	if (enable)
		BIT_SET (flags, O_NDELAY);
	else
		BIT_UNSET (flags, O_NDELAY);

	re = fcntl (fd, F_SETFL, flags);
#endif

	if (unlikely (re < 0)) {
        chula_log_errno (errno, CHULA_LOG_ERROR, "fcntl (F_SETFL, fd=%d, flags=%d (+%s)): ${errno}", fd, flags, "O_NDELAY");
		return ret_error;
	}

	return ret_ok;
}

ret_t
chula_fd_set_nonblocking (int fd, bool enable)
{
	int re;
	int flags = 0;

	flags = fcntl (fd, F_GETFL, 0);
	if (flags < 0) {
        chula_log_errno (errno, CHULA_LOG_ERROR, "fcntl (F_GETFL, fd=%d, 0): ${errno}", fd);
		return ret_error;
	}

	if (enable)
		BIT_SET (flags, O_NONBLOCK);
	else
		BIT_UNSET (flags, O_NONBLOCK);

	re = fcntl (fd, F_SETFL, flags);
	if (re < 0) {
        chula_log_errno (errno, CHULA_LOG_ERROR, "fcntl (F_SETFL, fd=%d, flags=%d (+%s)): ${errno}", fd, flags, "O_NONBLOCK");
		return ret_error;
	}

	return ret_ok;
}

ret_t
chula_fd_set_closexec (int fd)
{
	int re;
	int flags = 0;

	flags = fcntl (fd, F_GETFD, 0);
	if (flags < 0) {
        chula_log_errno (errno, CHULA_LOG_ERROR, "fcntl (F_GETFD, fd=%d, 0): ${errno}", fd);
		return ret_error;
	}

	BIT_SET (flags, FD_CLOEXEC);

	re = fcntl (fd, F_SETFD, flags);
	if (re < 0) {
        chula_log_errno (errno, CHULA_LOG_ERROR, "fcntl (F_SETFD, fd=%d, flags=%d (+%s)): ${errno}", fd, flags, "FD_CLOEXEC");
		return ret_error;
	}

	return ret_ok;
}

ret_t
chula_fd_set_reuseaddr (int fd)
{
	int re;
	int on = 1;

	re = setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (re != 0) {
		return ret_error;
	}

	return ret_ok;
}

inline ret_t
chula_fd_close (int fd)
{
	int re;

	if (unlikely (fd < 0)) {
		return ret_error;
	}

    re = close (fd);
	TRACE (ENTRIES",close_fd", "fd=%d re=%d\n", fd, re);
	return (re == 0) ? ret_ok : ret_error;
}

ret_t
chula_sys_fdlimit_get (uint32_t *limit)
{
#ifdef HAVE_GETRLIMIT
        struct rlimit rlp;

        rlp.rlim_cur = rlp.rlim_max = RLIM_INFINITY;
        if (getrlimit (RLIMIT_NOFILE, &rlp))
            return ret_error;

        *limit = rlp.rlim_cur;
	return ret_ok;
#else
#ifdef HAVE_GETDTABLESIZE
	int nfiles;

	nfiles = getdtablesize();
        if (nfiles <= 0) return ret_error;

	*limit = nfiles;
	return ret_ok;
#else
#ifdef OPEN_MAX
        *limit = OPEN_MAX;         /* need to include limits.h somehow */
	return ret_ok;
#else
        *limit = FD_SETSIZE;
	return ret_ok;
#endif
#endif
#endif
}


ret_t
chula_sys_fdlimit_set (uint32_t limit)
{
	int           re;
	struct rlimit rl;

	rl.rlim_cur = limit;
	rl.rlim_max = limit;

	re = setrlimit (RLIMIT_NOFILE, &rl);
	if (re != 0) {
		return ret_error;
	}

	return ret_ok;
}



/* Users & Groups
 */

#if !defined(HAVE_GETPWNAM_R) || !defined(HAVE_GETPWUID_R)
static ret_t
clone_struct_passwd (struct passwd *source, struct passwd *target, char *buf, size_t buflen)
{
	char   *ptr;
	size_t  pw_name_len   = 0;
	size_t  pw_passwd_len = 0;
	size_t  pw_gecos_len  = 0;
	size_t  pw_dir_len    = 0;
	size_t  pw_shell_len  = 0;

	if (source->pw_name)   pw_name_len   = strlen(source->pw_name);
	if (source->pw_passwd) pw_passwd_len = strlen(source->pw_passwd);
	if (source->pw_gecos)  pw_gecos_len  = strlen(source->pw_gecos);
	if (source->pw_dir)    pw_dir_len    = strlen(source->pw_dir);
	if (source->pw_shell)  pw_shell_len  = strlen(source->pw_shell);

	if ((pw_name_len + pw_passwd_len +
	     pw_gecos_len + pw_dir_len + pw_shell_len + 5) >= buflen) {
		/* Buffer overflow.
		 */
		return ret_error;
	}
	memset (buf, 0, buflen);
	ptr = buf;

	target->pw_uid = source->pw_uid;
	target->pw_gid = source->pw_gid;

	if (source->pw_dir) {
		memcpy (ptr, source->pw_dir, pw_dir_len);
		target->pw_dir = ptr;
		ptr += pw_dir_len + 1;
	}

	if (source->pw_passwd) {
		memcpy (ptr, source->pw_passwd, pw_passwd_len);
		target->pw_passwd = ptr;
		ptr += pw_passwd_len + 1;
	}

	if (source->pw_name) {
		memcpy (ptr, source->pw_name, pw_name_len);
		target->pw_name = ptr;
		ptr += pw_name_len + 1;
	}

	if (source->pw_gecos) {
		memcpy (ptr, source->pw_gecos, pw_gecos_len);
		target->pw_gecos = ptr;
		ptr += pw_gecos_len + 1;
	}

	if (source->pw_shell) {
		memcpy (ptr, source->pw_shell, pw_shell_len);
		target->pw_shell = ptr;
		ptr += pw_shell_len + 1;
	}

	return ret_ok;
}
#endif

#ifndef HAVE_GETGRGID_R
static ret_t
clone_struct_group (struct group *source, struct group *target, char *buf, size_t buflen)
{
	char   *ptr;
	size_t  gr_name_len   = 0;
	size_t  gr_passwd_len = 0;

	if (source->gr_name)   gr_name_len   = strlen(source->gr_name);
	if (source->gr_passwd) gr_passwd_len = strlen(source->gr_passwd);

	if ((gr_name_len + gr_passwd_len) >= buflen) {
		/* Buffer overflow */
		return ret_error;
	}

	memset (buf, 0, buflen);
	ptr = buf;

	target->gr_gid = source->gr_gid;

	if (source->gr_name) {
		memcpy (ptr, source->gr_name, gr_name_len);
		target->gr_name = ptr;
		ptr += gr_name_len + 1;
	}

	if (source->gr_passwd) {
		memcpy (ptr, source->gr_passwd, gr_passwd_len);
		target->gr_passwd = ptr;
		ptr += gr_passwd_len + 1;
	}

	/* TODO: Duplicate '**gr_mem' as well
	 */

	return ret_ok;
}
#endif

#if !defined(HAVE_GETPWNAM_R)
static pthread_mutex_t __global_getpwnam_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

ret_t
chula_getpwnam (const char *name, struct passwd *pwbuf, char *buf, size_t buflen)
{
#ifndef HAVE_GETPWNAM_R
	ret_t          ret;
	struct passwd *tmp;

	CHULA_MUTEX_LOCK (&__global_getpwnam_mutex);

	do {
		tmp = getpwnam (name);
	} while ((tmp == NULL) && (errno == EINTR));

	if (tmp == NULL) {
		CHULA_MUTEX_UNLOCK (&__global_getpwnam_mutex);
		return ret_error;
	}

	ret = clone_struct_passwd (tmp, pwbuf, buf, buflen);
	if (ret != ret_ok) {
		CHULA_MUTEX_UNLOCK (&__global_getpwnam_mutex);
		return ret_error;
	}

	CHULA_MUTEX_UNLOCK (&__global_getpwnam_mutex);
	return ret_ok;

#elif defined(HAVE_GETPWNAM_R_5)
	int            re;
	struct passwd *tmp = NULL;

	/* MacOS X:
	 * int getpwnam_r (const char     *login,
	 *                 struct passwd  *pwd,
	 *                 char           *buffer,
	 *                 size_t          bufsize,
	 *                 struct passwd **result);
	 *
	 * Linux:
	 * int getpwnam_r (const char     *name,
     * 	               struct passwd  *pwbuf,
	 *                 char           *buf,
     *                 size_t          buflen,
	 *                 struct passwd **pwbufp);
	 */
	do {
		re = getpwnam_r (name, pwbuf, buf, buflen, &tmp);
	} while ((re != 0) && (errno == EINTR));

	if ((re != 0) || (tmp == NULL))
		return ret_error;

	return ret_ok;

#elif defined(HAVE_GETPWNAM_R_4)
	struct passwd * result;

#ifdef _POSIX_PTHREAD_SEMANTICS
	int re;

	do {
		re = getpwnam_r (name, pwbuf, buf, buflen, &result);
	} while ((re != 0) && (errno == EINTR));

	if (re != 0)
		return ret_error;
#else
	do {
		result = getpwnam_r (name, pwbuf, buf, buflen);
	} while ((result == NULL) && (errno == EINTR));

	if (result == NULL)
		return ret_error;
#endif

	return ret_ok;
#endif

	return ret_no_sys;
}


#if !defined(HAVE_GETPWUID_R)
static pthread_mutex_t __global_getpwuid_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

ret_t
chula_getpwuid (uid_t uid, struct passwd *pwbuf, char *buf, size_t buflen)
{
#ifdef HAVE_GETPWUID_R_5
	int            re;
	struct passwd *result = NULL;

	/* Linux:
	 *  int getpwuid_r (uid_t uid, struct passwd *pwd, char *buf,    size_t buflen,  struct passwd **result);
	 * MacOS X:
	 *  int getpwuid_r (uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result);
	 */
	do {
		re = getpwuid_r (uid, pwbuf, buf, buflen, &result);
	} while ((re != 0) && (errno == EINTR));

	if ((re != 0) || (result == NULL)) {
		return ret_error;
	}

	return ret_ok;

#elif defined(HAVE_GETPWUID_R_4)
	struct passwd *result;

	/* Solaris:
	 * struct passwd *getpwuid_r (uid_t uid, struct passwd *pwd, char *buffer, int  buflen);
	 */
	do {
		result = getpwuid_r (uid, pwbuf, buf, buflen);
	} while ((result == NULL) && (errno == EINTR));

	if (result == NULL) {
		return ret_error;
	}

	return ret_ok;
#else
	ret_t          ret;
	struct passwd *tmp;

	/* struct passwd *getpwuid (uid_t uid);
	 */
	CHULA_MUTEX_LOCK (&__global_getpwuid_mutex);

	do {
		tmp = getpwuid (uid);
	} while ((tmp == NULL) && (errno == EINTR));

	if (tmp == NULL) {
		CHULA_MUTEX_UNLOCK (&__global_getpwuid_mutex);
		return ret_error;
	}

	ret = clone_struct_passwd (tmp, pwbuf, buf, buflen);
	if (ret != ret_ok) {
		CHULA_MUTEX_UNLOCK (&__global_getpwuid_mutex);
		return ret_error;
	}

	CHULA_MUTEX_UNLOCK (&__global_getpwuid_mutex);
	return ret_ok;
#endif

	return ret_no_sys;
}

ret_t
chula_getpwnam_uid (const char *name, struct passwd *pwbuf, char *buf, size_t buflen)
{
	ret_t ret;
	long  tmp_uid;

	memset (buf, 0, buflen);
	memset (pwbuf, 0, sizeof(struct passwd));

	ret = chula_getpwnam (name, pwbuf, buf, buflen);
	if ((ret == ret_ok) && (pwbuf->pw_dir)) {
		return ret_ok;
	}

	errno   = 0;
	tmp_uid = strtol (name, NULL, 10);
	if (errno != 0) {
		return ret_error;
	}

	ret = chula_getpwuid ((uid_t)tmp_uid, pwbuf, buf, buflen);
	if ((ret != ret_ok) || (pwbuf->pw_dir == NULL)) {
		return ret_error;
	}

	return ret_ok;
}


#if !defined(HAVE_GETGRNAM_R)
static pthread_mutex_t __global_getgrnam_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

ret_t
chula_getgrnam (const char *name, struct group *grbuf, char *buf, size_t buflen)
{
#ifndef HAVE_GETGRNAM_R
	size_t        gr_name_len   = 0;
	size_t        gr_passwd_len = 0;
	char         *ptr;
	struct group *tmp;

	CHULA_MUTEX_LOCK (&__global_getgrnam_mutex);

	do {
		tmp = getgrnam (name);
	} while ((tmp == NULL) && (errno == EINTR));

	if (tmp == NULL) {
		CHULA_MUTEX_UNLOCK (&__global_getgrnam_mutex);
		return ret_error;
	}

	if (tmp->gr_name)
		gr_name_len   = strlen(tmp->gr_name);
	if (tmp->gr_passwd)
		gr_passwd_len = strlen(tmp->gr_passwd);

	if ((gr_name_len + gr_passwd_len + 2) >= buflen) {
		CHULA_MUTEX_UNLOCK (&__global_getgrnam_mutex);
		return ret_error;
	}
	memset (buf, 0, buflen);
	ptr = buf;

	grbuf->gr_gid = tmp->gr_gid;

	if (tmp->gr_name) {
		memcpy (ptr, tmp->gr_name, gr_name_len);
		grbuf->gr_name = ptr;
		ptr += gr_name_len + 1;
	}

	if (tmp->gr_passwd) {
		memcpy (ptr, tmp->gr_passwd, gr_passwd_len);
		grbuf->gr_passwd = ptr;
		ptr += gr_passwd_len + 1;
	}

	/* TODO: Duplicate char **tmp->gr_mem
	 */

	CHULA_MUTEX_UNLOCK (&__global_getgrnam_mutex);

	return ret_ok;

#elif defined(HAVE_GETGRNAM_R_5)
	int           re;
	struct group *tmp;

	do {
		re = getgrnam_r (name, grbuf, buf, buflen, &tmp);
	} while ((re != 0) && (errno == EINTR));

	if (re != 0)
		return ret_error;

	return ret_ok;

#elif defined(HAVE_GETGRNAM_R_4)
	struct group  *result;

#ifdef _POSIX_PTHREAD_SEMANTICS
	int re;
	do {
		re = getgrnam_r (name, grbuf, buf, buflen, &result);
	} while ((re != 0) && (errno == EINTR));

	if (re != 0)
		return ret_error;
#else
	do {
		result = getgrnam_r (name, grbuf, buf, buflen);
	} while ((result == NULL) && (errno == EINTR));

	if (result == NULL)
		return ret_error;
#endif

	return ret_ok;
#endif

	return ret_no_sys;
}


#if !defined(HAVE_GETGRGID_R)
static pthread_mutex_t __global_getgrgid_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

ret_t
chula_getgrgid (gid_t gid, struct group *grbuf, char *buf, size_t buflen)
{
#ifdef HAVE_GETGRGID_R_5
	int            re;
	struct group *result = NULL;

	/* Linux:
	 *  int getgrgid_r (gid_t gid, struct group *grp, char *buf, size_t buflen, struct group **result);
	 * MacOS X:
	 *  int getgrgid_r (gid_t gid, struct group *grp, char *buffer, size_t bufsize, struct group **result);
	 */
	do {
		re = getgrgid_r (gid, grbuf, buf, buflen, &result);
	} while ((re != 0) && (errno == EINTR));

	if ((re != 0) || (result == NULL)) {
		return ret_error;
	}

	return ret_ok;

#elif defined(HAVE_GETGRGID_R_4)
	struct group *result = NULL;

	/* Solaris:
	 *  struct group *getgrgid_r (gid_t gid, struct group *grp, char *buffer, int bufsize);
	 */
	do {
		result = getgrgid_r (gid, grbuf, buf, buflen);
	} while ((result == NULL) && (errno == EINTR));

	if (result == NULL) {
		return ret_error;
	}

	return ret_ok;
#else
	ret_t         ret;
	struct group *tmp;

	CHULA_MUTEX_LOCK (&__global_getgrgid_mutex);

	do {
		tmp = getgrgid (gid);
	} while ((tmp == NULL) && (errno == EINTR));

	if (tmp == NULL) {
		CHULA_MUTEX_UNLOCK (&__global_getgrgid_mutex);
		return ret_error;
	}

	ret = clone_struct_group (tmp, grbuf, buf, buflen);
	if (ret != ret_ok) {
		CHULA_MUTEX_UNLOCK (&__global_getgrgid_mutex);
		return ret_error;
	}

	CHULA_MUTEX_UNLOCK (&__global_getgrgid_mutex);
	return ret_ok;
#endif

	return ret_no_sys;
}

ret_t
chula_getgrnam_gid (const char *name, struct group *grbuf, char *buf, size_t buflen)
{
	ret_t ret;
	long  tmp_gid;

	memset (buf, 0, buflen);
	memset (grbuf, 0, sizeof(struct group));

	ret = chula_getgrnam (name, grbuf, buf, buflen);
	if ((ret == ret_ok) && (grbuf->gr_name)) {
		return ret_ok;
	}

    ret = chula_atol (name, &tmp_gid);
    if (ret != ret_ok)
        return ret;

	ret = chula_getgrgid ((gid_t)tmp_gid, grbuf, buf, buflen);
	if ((ret != ret_ok) || (grbuf->gr_name == NULL)) {
		return ret_error;
	}

	return ret_ok;
}



/* Time management functions
 */


#if !defined(HAVE_GMTIME_R)
static pthread_mutex_t gmtime_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

struct tm *
chula_gmtime (const time_t *timep, struct tm *result)
{
#ifdef HAVE_GMTIME_R
	/* Use the thread safe version anyway (no data copy).
	 */
	return gmtime_r (timep, result);
#else
	struct tm *tmp;

	CHULA_MUTEX_LOCK (&gmtime_mutex);
	if (likely ((tmp = gmtime (timep)) != NULL))
		memcpy (result, tmp, sizeof(struct tm));
	CHULA_MUTEX_UNLOCK (&gmtime_mutex);

	return (tmp == NULL ? NULL : result);
#endif
}


#if !defined(HAVE_LOCALTIME_R)
static pthread_mutex_t localtime_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

struct tm *
chula_localtime (const time_t *timep, struct tm *result)
{
#ifdef HAVE_LOCALTIME_R
	/* Use the thread safe version anyway (no data copy).
	 */
	return localtime_r (timep, result);
#else
	struct tm *tmp;

	CHULA_MUTEX_LOCK (&localtime_mutex);
	if (likely ((tmp = localtime (timep)) != NULL))
		memcpy (result, tmp, sizeof(struct tm));
	CHULA_MUTEX_UNLOCK (&localtime_mutex);

	return (tmp == NULL ? NULL : result);
#endif
}


long *
chula_get_timezone_ref (void)
{
#ifdef HAVE_STRUCT_TM_GMTOFF
	struct tm   tm;
	time_t      timestamp;
	static long tz         = 43201; /* 12h+1s: out of range */

	if (unlikely (tz == 43201)) {
		timestamp = time(NULL);
		chula_localtime (&timestamp, &tm);
		tz = -tm.tm_gmtoff;
	}
	return &tz;
#else
# ifdef HAVE_INT_TIMEZONE
	return &timezone;
# else
	static long _faked_timezone = 0;
	return &_faked_timezone;
# endif
#endif
}



/* Paths Management
 */



ret_t
chula_path_short (chula_buffer_t *path)
{
	uint8_t *p   = path->buf;
	uint8_t *end = path->buf + path->len;

	while (p < end) {
		uint8_t  *dots_end;
		uint8_t  *prev_slash;
		uint32_t  len;

		if (p[0] != (uint8_t)'.') {
			p++;
			continue;
		}

		if ((p[1] == (uint8_t)'/') && (p > path->buf) && (*(p-1) == (uint8_t)'/')) {
			chula_buffer_remove_chunk (path, p - path->buf, 2);
			p -= 1;
			continue;
		}

		if (end < p+2) {
			return ret_ok;
		}

		if (p[1] != (uint8_t)'.') {
			p+=2;
			continue;
		}

		dots_end = p + 2;
		while ((dots_end < end) && (*dots_end == (uint8_t)'.')) {
			dots_end++;
		}

		if (dots_end >= end)
			return ret_ok;

		prev_slash = p-1;

		if (prev_slash < path->buf)
			return ret_ok;

		if (*prev_slash != (uint8_t) '/') {
			p = dots_end;
			continue;
		}

		if (prev_slash > path->buf)
			prev_slash--;

		while ((prev_slash > path->buf) && (*prev_slash != (uint8_t) '/')) {
			prev_slash--;
		}

		len = dots_end - prev_slash;

		chula_buffer_remove_chunk (path, (prev_slash - path->buf), len);

		end = path->buf + path->len;
		p -= (len - (dots_end - p));
	}

	return ret_ok;
}

ret_t
chula_path_find_exec (const char     *bin_name,
                      chula_buffer_t *fullpath)
{
	int    re;
    ret_t  ret;
	char  *p, *q;
	char  *path;

	p = getenv("PATH");
	if (p == NULL) {
		return ret_not_found;
	}

	path = strdup(p);
	if (unlikely (path == NULL)) {
		return ret_nomem;
	}

	p = path;
	do {
		q = strchr (p, ':');
		if (q) {
			*q = '\0';
		}

		chula_buffer_clean   (fullpath);

		ret = chula_buffer_add (fullpath, p, strlen(p));
        if (unlikely (ret != ret_ok)) goto error;
		ret = chula_buffer_add_str (fullpath, "/");
        if (unlikely (ret != ret_ok)) goto error;
		ret = chula_buffer_add (fullpath, bin_name, strlen(bin_name));
        if (unlikely (ret != ret_ok)) goto error;

		re = chula_access ((char *)fullpath->buf, X_OK);
		if (re == 0) {
			free (path);
			return ret_ok;
		}

		p = q + 1;
	} while(q);

	free (path);
	return ret_not_found;

error:
	free (path);
	return ret_error;
}



/* Network related utilities
 */


ret_t
chula_gethostbyname (chula_buffer_t *hostname, struct addrinfo **addr)
{
	int              n;
	struct addrinfo  hints;

    /* Sanity check */
    if (unlikely (chula_buffer_is_empty(hostname)))
        return ret_error;

	/* What we are trying to get
	 */
	memset (&hints, 0, sizeof(struct addrinfo));

	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

#ifdef AI_ADDRCONFIG
	/* Workaround for loopback host addresses:
	 *
	 * If a computer does not have any outgoing IPv6 network
	 * interface, but its loopback network interface supports
	 * IPv6, a getaddrinfo call on "localhost" with AI_ADDRCONFIG
	 * won't return the IPv6 loopback address "::1", because
	 * getaddrinfo() thinks the computer cannot connect to any
	 * IPv6 destination, ignoring the remote vs. local/loopback
	 * distinction.
	 */
	if ((chula_buffer_cmp_str (hostname, "::1")       == 0) ||
	    (chula_buffer_cmp_str (hostname, "127.0.0.1") == 0) ||
	    (chula_buffer_cmp_str (hostname, "localhost")  == 0) ||
	    (chula_buffer_cmp_str (hostname, "localhost6") == 0) ||
	    (chula_buffer_cmp_str (hostname, "localhost.localdomain")   == 0) ||
	    (chula_buffer_cmp_str (hostname, "localhost6.localdomain6") == 0))
	{
		hints.ai_flags = AI_ADDRCONFIG;
	}
#endif

	/* Resolve address
	 */
	n = getaddrinfo ((char *)hostname->buf, NULL, &hints, addr);
	if (n < 0) {
		return ret_error;
	}

	return ret_ok;
}

ret_t
chula_gethostname (chula_buffer_t *buf)
{
	int   re;
    ret_t ret;

#ifdef HAVE_GETHOSTNAME
	char host_name[HOST_NAME_MAX + 1];

	re = gethostname (host_name, HOST_NAME_MAX);
	if (re) {
		return ret_error;
	}

	ret = chula_buffer_add (buf, host_name, strlen(host_name));
    if (unlikely (ret != ret_ok)) return ret;

	return ret_ok;

#elif defined(HAVE_SYS_UTSNAME_H) && defined(HAVE_UNAME)
	struct utsname info;

	re = uname (&info);
	if (re) {
		return ret_error;
	}

	ret = chula_buffer_add (buf, info.nodename, sizeof(info.nodename));
    if (unlikely (ret != ret_ok)) return ret;

	return ret_ok;
#endif

	return ret_error;
}

ret_t
chula_ntop (int family, struct sockaddr *addr, char *dst, size_t cnt)
{
	const char *str = NULL;
	errno = EAFNOSUPPORT;

	/* Only old systems without inet_ntop() function
	 */
#ifndef HAVE_INET_NTOP
    UNUSED(family);

    str = inet_ntoa (((struct sockaddr_in *)addr)->sin_addr);
    memcpy(dst, str, strlen(str));

    return ret_ok;
#else
# ifdef HAVE_IPV6
	if (family == AF_INET6) {
		struct in6_addr *addr6 = &(((struct sockaddr_in6 *)addr)->sin6_addr);

		if (IN6_IS_ADDR_V4MAPPED (addr6) ||
		    IN6_IS_ADDR_V4COMPAT (addr6))
		{
			const void *p = &(addr6)->s6_addr[12];

			str = inet_ntop (AF_INET, p, dst, cnt);
			if (str == NULL) {
				goto error;
			}
		} else {
			str = (char *) inet_ntop (AF_INET6, addr6, dst, cnt);
			if (str == NULL) {
				goto error;
			}
		}

        return ret_ok;
	}
# endif
    struct in_addr *addr4 = &((struct sockaddr_in *)addr)->sin_addr;

    UNUSED(family);

    str = inet_ntop (AF_INET, addr4, dst, cnt);
    if (str == NULL) {
        goto error;
    }
#endif

	return ret_ok;

error:
	dst[0] = '\0';
	return ret_error;
}



/* Random Numbers
 */


void
chula_random_seed (void)
{
#ifdef HAVE_SRANDOMDEV
	srandomdev();
#else
	int            fd;
	ssize_t        re;
	unsigned       seed;
	struct timeval tv;

	/* Read from device
	 */
	if ((fd = open("/dev/hwrng", O_RDONLY)) >= 0 ||
	    (fd = open("/dev/random", O_RDONLY)) >= 0 ||
	    (fd = open("/dev/urandom", O_RDONLY)) >= 0)
	{
		do {
			re = read (fd, &seed, sizeof(seed));
		} while ((re == -1) && (errno == EINTR));

		chula_fd_close(fd);

		if (re == sizeof(seed))
			goto out;
	}

	/* Home-made seed
	 */
    gettimeofday (&tv, NULL);
    seed = (getpid() << 16) ^ tv.tv_sec ^ tv.tv_usec;

out:
	/* Set the seed
	 */
# ifdef HAVE_SRANDOM
	srandom (seed);
# else
	srand (seed);
# endif
#endif
}


long
chula_random (void)
{
#ifdef HAVE_RANDOM
	return random();
#else
	return rand();
#endif
}



/* Misc
 */


long
chula_eval_formated_time (chula_buffer_t *buf)
{
    ret_t ret;
	char  end;
    long  val;
	int   mul = 1;

	if (unlikely (chula_buffer_is_empty (buf)))
		return 0;

	end = chula_buffer_end_char (buf);
	switch (end) {
	case 's':
		mul = 1;
		break;
	case 'm':
		mul = 60;
		break;
	case 'h':
		mul = 60 * 60;
		break;
	case 'd':
		mul = 60 * 60 * 24;
		break;
	case 'w':
		mul = 60 * 60 * 24 * 7;
		break;
	default:
		break;
	}

    ret = chula_atol ((char *)buf->buf, &val);
    if (unlikely (ret != ret_ok)) return ret;

	return val * mul;
}


ret_t
chula_syslog (int priority, chula_buffer_t *buf)
{
	uint8_t *p;
	uint8_t *nl, *end;

	if (chula_buffer_is_empty(buf))
		return ret_ok;

	p   = buf->buf;
	end = buf->buf + buf->len;

	do {
		nl = (uint8_t *) strchr ((char *)p, '\n');
		if (nl != NULL)
			*nl = (uint8_t)'\0';

		syslog (priority, "%s", (char *)p);

		if (nl == NULL) {
			break;
		}

		*nl = (uint8_t)'\n';
		p = nl + 1;
	} while (p < end);

	return ret_ok;
}


ret_t
chula_buf_add_backtrace (chula_buffer_t *buf,
                         int             n_skip,
                         const char     *new_line,
                         const char     *line_pre)
{
#ifdef HAVE_BACKTRACE
	void    *array[128];
	size_t   size;
	char   **strings;
	size_t   i;
	int      line_pre_len;
	int      new_line_len;

	line_pre_len = strlen (line_pre);
	new_line_len = strlen (new_line);

	size = backtrace (array, 128);
	strings = backtrace_symbols (array, size);

	for (i=n_skip; i < size; i++) {
		if (line_pre_len > 0) {
			chula_buffer_add (buf, line_pre, line_pre_len);
		}
		chula_buffer_add (buf, strings[i], strlen(strings[i]));
		chula_buffer_add (buf, new_line, new_line_len);
	}

	free (strings);
	return ret_ok;
#else
	return ret_no_sys;
#endif
}


ret_t
chula_wait_pid (pid_t pid, int *retcode)
{
	int re;
	int exitcode;

	TRACE(ENTRIES",signal", "Handling SIGCHLD, waiting PID %d\n", pid);

	while (true) {
		re = waitpid (pid, &exitcode, 0);
		if (re > 0) {
			if (WIFEXITED(exitcode) && (retcode != NULL)) {
				*retcode = WEXITSTATUS(exitcode);
			}
			return ret_ok;
		}

		else if (errno == ECHILD) {
			return ret_not_found;
		}

		else if (errno == EINTR) {
			continue;

		} else {
			chula_log_error ("ERROR: waiting PID %d, error %d\n", pid, errno);
			return ret_error;
		}
	}

	SHOULDNT_HAPPEN;
	return ret_error;
}


ret_t
chula_reset_signals (void)
{
	int              i;
	struct sigaction sig_action;

	/* Reset signal handlers */
	sig_action.sa_handler = SIG_DFL;
	sig_action.sa_flags   = 0;
	sigemptyset (&sig_action.sa_mask);

	for (i=0 ; i < NSIG ; i++) {
		sigaction (i, &sig_action, NULL);
	}

	return ret_ok;
}


ret_t
chula_get_shell (const char **shell, const char **binary)
{
	char *t1, *t2;

	/* Set the shell path
	 */
#ifdef _WIN32
	*shell = getenv("ComSpec");
#else
	*shell = "/bin/sh";
#endif

	/* Find the binary
	 */
	t1 = strrchr (*shell, '\\');
	t2 = strrchr (*shell, '/');

	t1 = chula_max_str (t1, t2);
	if (t1 == NULL) return ret_error;

	*binary = &t1[1];

	return ret_ok;
}


ret_t
chula_tmp_dir_copy (chula_buffer_t *buffer)
{
    ret_t       ret;
	const char *p;

	/* Read a custom Http2d variable
	 */
	p = getenv("CHULA_TMPDIR");
	if (p != NULL) {
		ret = chula_buffer_add (buffer, p, strlen(p));
        if (unlikely (ret != ret_ok)) return ret;

		return ret_ok;
	}

	/* Read the system variable
	 */
#ifdef _WIN32
	p = getenv("TEMP");
#else
	p = getenv("TMPDIR");
#endif
#ifdef P_tmpdir
    if (p == NULL) {
        p = P_tmpdir;
    }
#endif
#ifdef _PATH_TMP
    if (p == NULL) {
        p = _PATH_TMP;
    }
#endif
	if (p != NULL) {
		ret = chula_buffer_add (buffer, p, strlen(p));
        if (unlikely (ret != ret_ok)) return ret;

		return ret_ok;
	}

	/* /tmp will be our safety-net
	 */
	ret = chula_buffer_add_str (buffer, "/tmp");
    if (unlikely (ret != ret_ok)) return ret;

	return ret_ok;
}


ret_t
chula_malloc_size (const void *ptr, size_t *size)
{
#ifdef HAVE_MALLOC_SIZE
    *size = malloc_size (ptr);
    return ret_ok;

#elif defined(HAVE_MALLOC_USABLE_SIZE)
    *size = malloc_usable_size (ptr);
    return ret_ok;

#else
# error "Neither malloc_size() nor malloc_usable_size()"
#endif
    return ret_error;
}
