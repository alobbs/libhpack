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

#ifndef CHULA_AVL_H
#define CHULA_AVL_H

#if !defined(CHULA_H_INSIDE) && !defined (CHULA_COMPILATION)
# error "Only <libchula/libchula.h> can be included directly."
#endif

#include <libchula/buffer.h>
#include <libchula/avl_generic.h>

/* AVL Tree Node
 */
typedef struct {
	chula_avl_generic_node_t base;
	chula_buffer_t           key;
} chula_avl_node_t;

/* AVL Tree
 */
typedef struct {
	chula_avl_generic_t base;
	bool                case_insensitive;
} chula_avl_t;

#define AVL(a) ((chula_avl_t *)(a))
#define AVL_NODE(a) ((chula_avl_node_t *)(a))


ret_t chula_avl_init     (chula_avl_t  *avl);
ret_t chula_avl_new      (chula_avl_t **avl);
ret_t chula_avl_set_case (chula_avl_t  *avl, bool case_insensitive);

ret_t chula_avl_add      (chula_avl_t *avl, chula_buffer_t *key, void  *value);
ret_t chula_avl_del      (chula_avl_t *avl, chula_buffer_t *key, void **value);
ret_t chula_avl_get      (chula_avl_t *avl, chula_buffer_t *key, void **value);

ret_t chula_avl_add_ptr  (chula_avl_t *avl, const char *key, void  *value);
ret_t chula_avl_del_ptr  (chula_avl_t *avl, const char *key, void **value);
ret_t chula_avl_get_ptr  (chula_avl_t *avl, const char *key, void **value);


typedef ret_t (* chula_avl_while_func_t) (chula_buffer_t *key, void *value, void *param);

ret_t chula_avl_while (chula_avl_generic_t       *avl,
                       chula_avl_while_func_t     func,
                       void                      *param,
                       chula_buffer_t           **key,
                       void                     **value);


#endif /* CHULA_AVL_H */
