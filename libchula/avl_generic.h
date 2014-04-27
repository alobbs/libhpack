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

#ifndef CHULA_AVL_GENERIC_H
#define CHULA_AVL_GENERIC_H

#if !defined(CHULA_H_INSIDE) && !defined (CHULA_COMPILATION)
# error "Only <libchula/libchula.h> can be included directly."
#endif

#include <libchula/buffer.h>

/* AVL Tree Node base
 */
typedef struct chula_avl_generic_node chula_avl_generic_node_t;

struct chula_avl_generic_node {
	short                          balance;
	struct chula_avl_generic_node *left;
	struct chula_avl_generic_node *right;
	bool                           left_child;
	bool                           right_child;
	void                          *value;
};

#define AVL_GENERIC_NODE(a) ((chula_avl_generic_node_t *)(a))

ret_t chula_avl_generic_node_init (chula_avl_generic_node_t *node);

/* AVL Tree
 */
typedef struct chula_avl_generic chula_avl_generic_t;

typedef ret_t (*avl_gen_node_mrproper_t) (chula_avl_generic_node_t *);
typedef ret_t (*avl_gen_node_cmp_t)      (chula_avl_generic_node_t *, chula_avl_generic_node_t *, chula_avl_generic_t *);
typedef ret_t (*avl_gen_node_is_empty_t) (chula_avl_generic_node_t *);

struct chula_avl_generic {
	chula_avl_generic_node_t *root;

	/* Virtual methods */
	avl_gen_node_mrproper_t node_mrproper;
	avl_gen_node_cmp_t      node_cmp;
	avl_gen_node_is_empty_t node_is_empty;
};

#define AVL_GENERIC(a) ((chula_avl_generic_t *)(a))

ret_t chula_avl_generic_init (chula_avl_generic_t *avl);


/* Public Methods
 */
typedef ret_t (* chula_avl_generic_while_func_t) (chula_avl_generic_node_t *key, void *value, void *param);

ret_t chula_avl_mrproper (chula_avl_generic_t *avl, chula_func_free_t free_func);
ret_t chula_avl_free     (chula_avl_generic_t *avl, chula_func_free_t free_func);
ret_t chula_avl_check    (chula_avl_generic_t *avl);
ret_t chula_avl_len      (chula_avl_generic_t *avl, size_t *len);
int   chula_avl_is_empty (chula_avl_generic_t *avl);

ret_t chula_avl_generic_add (chula_avl_generic_t *avl, chula_avl_generic_node_t *key, void  *value);
ret_t chula_avl_generic_del (chula_avl_generic_t *avl, chula_avl_generic_node_t *key, void **value);
ret_t chula_avl_generic_get (chula_avl_generic_t *avl, chula_avl_generic_node_t *key, void **value);

ret_t chula_avl_generic_while (chula_avl_generic_t             *avl,
                               chula_avl_generic_while_func_t   func,
                               void                            *param,
                               chula_avl_generic_node_t       **key,
                               void                           **value);

#endif /* CHULA_AVL_GENERIC_H */
