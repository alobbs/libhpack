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
#include "cstrings.h"
#include <string.h>
#include <strings.h>


char *
chula_strsep (char **str, const char *delims)
{
#ifndef HAVE_STRSEP
	char* token;

	if (*str == NULL) {
		/* No more tokens
		 */
		return NULL;
	}

	token = *str;
	while (**str!='\0') {
		if (strchr(delims,**str)!=NULL) {
			**str='\0';
			(*str)++;
			return token;
		}
		(*str)++;
	}

	/* There is no other token
	 */
	*str=NULL;
	return token;
#else
    return strsep (str, delims);
#endif
}

char *
chula_strnstr (const char *s, const char *find, size_t slen)
{
#ifndef HAVE_STRNSTR
	char c, sc;
	size_t len;

	if ((c = *find++) != '\0') {
		len = strlen(find);
		do {
			do {
				if (slen-- < 1 || (sc = *s++) == '\0')
					return (NULL);
			} while (sc != c);
			if (len > slen)
				return (NULL);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
#else
    return strnstr (s, find, slen);
#endif
}

char *
chula_strcasestr (const char *s, const char *find)
{
#ifndef HAVE_STRCASESTR
	register char c, sc;
	register size_t len;

	if ((c = *find++) != 0) {
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return NULL;
			} while (sc != c);
		} while (strncasecmp(s, find, len) != 0);
		s--;
	}
	return ((char *) s);
#else
    return strcasestr (s, find);
#endif
}

char *
chula_strncasestrn (const char *s, size_t slen, const char *find, size_t findlen)
{
    char c;
	char sc;

	if (unlikely (find == NULL) || (findlen == 0))
		return (char *)s;

	if (unlikely (*find == '\0'))
		return (char *)s;

	c = *find;
	find++;
	findlen--;

	do {
		do {
			if (slen-- < 1 || (sc = *s++) == '\0')
				return NULL;
		} while (CHULA_CHAR_TO_LOWER(sc) != CHULA_CHAR_TO_LOWER(c));
		if (findlen > slen) {
			return NULL;
		}
	} while (strncasecmp (s, find, findlen) != 0);

	s--;
    return (char *)s;
}

char *
chula_strncasestr (const char *s, const char *find, size_t slen)
{
	return chula_strncasestrn (s, slen, find, strlen(find));
}


/* Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t
chula_strlcat (char *dst, const char *src, size_t siz)
{
#ifndef HAVE_STRLCAT
    /* The following few lines has been copy and pasted from Todd
     * C. Miller <Todd.Miller@courtesan.com> code. BSD licensed.
     */
	register char       *d     = dst;
    register const char *s     = src;
    register size_t      n     = siz;
    size_t               dlen;

    /* Find the end of dst and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != '\0')
        d++;
    dlen = d - dst;
    n = siz - dlen;

    if (n == 0)
        return(dlen + strlen(s));
    while (*s != '\0') {
        if (n != 1) {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';

    /* count does not include NUL */
    return (dlen + (s - src));
#else
    return strlcat (dst, src, siz);
#endif
}
