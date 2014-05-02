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

#ifndef CHULA_BUFFER_H
#define CHULA_BUFFER_H

#if !defined(CHULA_H_INSIDE) && !defined (CHULA_COMPILATION)
# error "Only <libchula/libchula.h> can be included directly."
#endif

#include <libchula/common.h>
#include <libchula/crc32.h>

#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

typedef struct {
	uint8_t  *buf;        /**< Memory chunk           */
	uint32_t  size;       /**< Total amount of memory */
	uint32_t  len;        /**< Length of the string   */
} chula_buffer_t;

#define BUF(x) ((chula_buffer_t *)(x))
#define CHULA_BUF_INIT               {NULL, 0, 0}
#define CHULA_BUF_INIT_FAKE(s)       {(uint8_t *)(s), sizeof(s), sizeof(s)-1}
#define CHULA_BUF_INIT_FAKE_LEN(s,l) {(uint8_t *)(s), l, l}
#define CHULA_BUF_SLIDE_NONE         INT_MIN

#define chula_buffer_is_empty(b)        (BUF(b)->len == 0)
#define chula_buffer_add_str(b,s)       chula_buffer_add      (b, s, sizeof(s)-1)
#define chula_buffer_prepend_str(b,s)   chula_buffer_prepend  (b, s, sizeof(s)-1)
#define chula_buffer_prepend_buf(b,s)   chula_buffer_prepend  (b, (char *)(s)->buf, (s)->len)
#define chula_buffer_cmp_str(b,s)       chula_buffer_cmp      (b, (char *)(s), sizeof(s)-1)
#define chula_buffer_case_cmp_str(b,s)  chula_buffer_case_cmp (b, (char *)(s), sizeof(s)-1)
#define chula_buffer_fake_str(b,s)      chula_buffer_fake     (b, s, sizeof(s)-1)

ret_t chula_buffer_new                  (chula_buffer_t **buf);
ret_t chula_buffer_free                 (chula_buffer_t  *buf);
ret_t chula_buffer_init                 (chula_buffer_t  *buf);
ret_t chula_buffer_mrproper             (chula_buffer_t  *buf);
void  chula_buffer_fake                 (chula_buffer_t  *buf, const char *str, uint32_t len);
ret_t chula_buffer_import               (chula_buffer_t  *buf, char *str, uint32_t len);

void  chula_buffer_clean                (chula_buffer_t  *buf);
ret_t chula_buffer_dup                  (chula_buffer_t  *buf, chula_buffer_t **dup);
void  chula_buffer_swap_buffers         (chula_buffer_t  *buf, chula_buffer_t *second);

ret_t chula_buffer_add                  (chula_buffer_t  *buf, const char *txt, size_t size);
ret_t chula_buffer_add_long10           (chula_buffer_t  *buf, int32_t lNum);
ret_t chula_buffer_add_llong10          (chula_buffer_t  *buf, int64_t lNum);
ret_t chula_buffer_add_ulong10          (chula_buffer_t  *buf, uint32_t ulNum);
ret_t chula_buffer_add_ullong10         (chula_buffer_t  *buf, uint64_t ulNum);
ret_t chula_buffer_add_ulong16          (chula_buffer_t  *buf, uint32_t ulNum);
ret_t chula_buffer_add_ullong16         (chula_buffer_t  *buf, uint64_t ulNum);
ret_t chula_buffer_add_uint16be         (chula_buffer_t  *buf, uint16_t n);
ret_t chula_buffer_add_uint32be         (chula_buffer_t  *buf, uint32_t n);
ret_t chula_buffer_add_va               (chula_buffer_t  *buf, const char *format, ...);
ret_t chula_buffer_add_va_fixed         (chula_buffer_t  *buf, const char *format, ...);
ret_t chula_buffer_add_va_list          (chula_buffer_t  *buf, const char *format, va_list args);
ret_t chula_buffer_add_char             (chula_buffer_t  *buf, char c);
ret_t chula_buffer_add_char_n           (chula_buffer_t  *buf, char c, int n);
ret_t chula_buffer_add_buffer           (chula_buffer_t  *buf, chula_buffer_t *buf2);
ret_t chula_buffer_add_buffer_slice     (chula_buffer_t  *buf, chula_buffer_t *buf2, ssize_t begin, ssize_t end);
ret_t chula_buffer_add_fsize            (chula_buffer_t  *buf, CST_OFFSET size);

ret_t chula_buffer_prepend              (chula_buffer_t  *buf, const char *txt, size_t size);

int32_t chula_buffer_cmp                 (chula_buffer_t  *buf, char *txt, uint32_t txt_len);
int32_t chula_buffer_cmp_buf             (chula_buffer_t  *buf, chula_buffer_t *buf2);
int32_t chula_buffer_case_cmp            (chula_buffer_t  *buf, char *txt, uint32_t txt_len);
int32_t chula_buffer_case_cmp_buf        (chula_buffer_t  *buf, chula_buffer_t *buf2);

ret_t chula_buffer_read_file            (chula_buffer_t  *buf, char *filename);
ret_t chula_buffer_read_from_fd         (chula_buffer_t  *buf, int fd, size_t size, size_t *ret_size);

ret_t chula_buffer_move_to_begin        (chula_buffer_t  *buf, uint32_t pos);
ret_t chula_buffer_drop_ending          (chula_buffer_t  *buf, uint32_t num_chars);
ret_t chula_buffer_multiply             (chula_buffer_t  *buf, int num);
ret_t chula_buffer_swap_chars           (chula_buffer_t  *buf, char a, char b);
ret_t chula_buffer_remove_dups          (chula_buffer_t  *buf, char c);
ret_t chula_buffer_remove_string        (chula_buffer_t  *buf, char *string, int string_len);
ret_t chula_buffer_remove_chunk         (chula_buffer_t  *buf, uint32_t from, uint32_t len);
ret_t chula_buffer_replace_string       (chula_buffer_t  *buf, const char *subs, int subs_len, const char *repl, int repl_len);
ret_t chula_buffer_substitute_string    (chula_buffer_t  *bufsrc, chula_buffer_t *bufdst, char *subs, int subs_len, char *repl, int repl_len);
ret_t chula_buffer_trim                 (chula_buffer_t  *buf);
ret_t chula_buffer_insert               (chula_buffer_t  *buf, char *txt, size_t txt_len, size_t pos);
ret_t chula_buffer_insert_buffer        (chula_buffer_t  *buf, chula_buffer_t *src, size_t pos);

ret_t chula_buffer_get_utf8_len         (chula_buffer_t  *buf, uint32_t *len);
ret_t chula_buffer_ensure_addlen        (chula_buffer_t  *buf, size_t alen);
ret_t chula_buffer_ensure_size          (chula_buffer_t  *buf, size_t size);
ret_t chula_buffer_retract              (chula_buffer_t  *buf);

int    chula_buffer_is_ending           (chula_buffer_t  *buf, char c);
char   chula_buffer_end_char            (chula_buffer_t  *buf);
size_t chula_buffer_cnt_spn             (chula_buffer_t  *buf, uint32_t offset, const char *str);
size_t chula_buffer_cnt_cspn            (chula_buffer_t  *buf, uint32_t offset, const char *str);

crc_t chula_buffer_crc32                (chula_buffer_t  *buf);
ret_t chula_buffer_encode_base64        (chula_buffer_t  *buf, chula_buffer_t *encoded);
ret_t chula_buffer_decode_base64        (chula_buffer_t  *buf);
ret_t chula_buffer_encode_md5           (chula_buffer_t  *buf, chula_buffer_t *encoded);
ret_t chula_buffer_encode_md5_digest    (chula_buffer_t  *buf);
ret_t chula_buffer_encode_sha1          (chula_buffer_t  *buf, chula_buffer_t *encoded);
ret_t chula_buffer_encode_sha1_digest   (chula_buffer_t  *buf);
ret_t chula_buffer_encode_sha1_base64   (chula_buffer_t  *buf, chula_buffer_t *encoded);
ret_t chula_buffer_encode_sha512        (chula_buffer_t  *buf, chula_buffer_t *encoded);
ret_t chula_buffer_encode_sha512_digest (chula_buffer_t  *buf);
ret_t chula_buffer_encode_sha512_base64 (chula_buffer_t  *buf, chula_buffer_t *encoded);
ret_t chula_buffer_encode_hex           (chula_buffer_t  *buf, chula_buffer_t *encoded);
ret_t chula_buffer_decode_hex           (chula_buffer_t  *buf);
ret_t chula_buffer_unescape_uri         (chula_buffer_t  *buf);
ret_t chula_buffer_escape_uri           (chula_buffer_t  *buf, chula_buffer_t *src);
ret_t chula_buffer_escape_uri_delims    (chula_buffer_t  *buf, chula_buffer_t *src);
ret_t chula_buffer_escape_arg           (chula_buffer_t  *buf, chula_buffer_t *src);
ret_t chula_buffer_add_escape_html      (chula_buffer_t  *buf, chula_buffer_t *src);
ret_t chula_buffer_escape_html          (chula_buffer_t  *buf, chula_buffer_t *src);
ret_t chula_buffer_add_comma_marks      (chula_buffer_t  *buf);

ret_t chula_buffer_to_lowcase           (chula_buffer_t  *buf);
ret_t chula_buffer_split_lines          (chula_buffer_t  *buf, int columns, const char *indent);

ret_t chula_buffer_print_cstr           (chula_buffer_t  *buf);
void  chula_buffer_repr                 (chula_buffer_t  *buf, chula_buffer_t *output);

#endif /* CHULA_BUFFER_H */
