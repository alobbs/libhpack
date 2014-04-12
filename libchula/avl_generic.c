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
#include "avl_generic.h"
#include "log.h"

#define MAX_HEIGHT 45

static chula_avl_generic_node_t *node_first  (chula_avl_generic_t *avl);
static chula_avl_generic_node_t *node_prev   (chula_avl_generic_node_t *node);
static chula_avl_generic_node_t *node_next   (chula_avl_generic_node_t *node);
static ret_t                     node_free   (chula_avl_generic_node_t *node, chula_avl_generic_t *avl);
static int32_t                   node_height (chula_avl_generic_node_t *node);


/* AVL Tree Node
 */

ret_t
chula_avl_generic_node_init (chula_avl_generic_node_t *node)
{
	node->balance     = 0;
	node->left        = NULL;
	node->right       = NULL;
	node->left_child  = false;
	node->right_child = false;
	node->value       = NULL;

	return ret_ok;
}


/* AVL Tree
 */

ret_t
chula_avl_generic_init (chula_avl_generic_t *avl)
{
	avl->root = NULL;

	avl->node_mrproper = NULL;
	avl->node_cmp      = NULL;
	avl->node_is_empty = NULL;

	return ret_ok;
}


/* Util
 */

static ret_t
node_free (chula_avl_generic_node_t *node,
           chula_avl_generic_t      *avl)
{
	avl->node_mrproper (node);
	free (node);
	return ret_ok;
}


static chula_avl_generic_node_t *
node_first (chula_avl_generic_t *avl)
{
	chula_avl_generic_node_t *tmp;

	if (!avl->root)
		return NULL;

	tmp = avl->root;

	while (tmp->left_child)
		tmp = tmp->left;

	return tmp;
}

static chula_avl_generic_node_t *
node_next (chula_avl_generic_node_t *node)
{
	chula_avl_generic_node_t *tmp = node->right;

	if (node->right_child)
		while (tmp->left_child)
			tmp = tmp->left;
	return tmp;
}

static chula_avl_generic_node_t *
node_prev (chula_avl_generic_node_t *node)
{
	chula_avl_generic_node_t *tmp = node->left;

	if (node->left_child)
		while (tmp->right_child)
			tmp = tmp->right;
	return tmp;
}


static chula_avl_generic_node_t *
node_rotate_left (chula_avl_generic_node_t *node)
{
	chula_avl_generic_node_t *right;
	int32_t                   a_bal;
	int32_t                   b_bal;

	right = node->right;

	if (right->left_child)
		node->right = right->left;
	else {
		node->right_child = false;
		node->right = right;
		right->left_child = true;
	}
	right->left = node;

	a_bal = node->balance;
	b_bal = right->balance;

	if (b_bal <= 0) {
		if (a_bal >= 1)
			right->balance = b_bal - 1;
		else
			right->balance = a_bal + b_bal - 2;
		node->balance = a_bal - 1;
	} else {
		if (a_bal <= b_bal)
			right->balance = a_bal - 2;
		else
			right->balance = b_bal - 1;
		node->balance = a_bal - b_bal - 1;
	}

	return right;
}

static chula_avl_generic_node_t *
node_rotate_right (chula_avl_generic_node_t *node)
{
	chula_avl_generic_node_t *left;
	int32_t                   a_bal;
	int32_t                   b_bal;

	left = node->left;

	if (left->right_child)
		node->left = left->right;
	else {
		node->left_child = false;
		node->left = left;
		left->right_child = true;
	}
	left->right = node;

	a_bal = node->balance;
	b_bal = left->balance;

	if (b_bal <= 0) {
		if (b_bal > a_bal)
			left->balance = b_bal + 1;
		else
			left->balance = a_bal + 2;
		node->balance = a_bal - b_bal + 1;
	} else {
		if (a_bal <= -1)
			left->balance = b_bal + 1;
		else
			left->balance = a_bal + b_bal + 2;
		node->balance = a_bal + 1;
	}

	return left;
}


static chula_avl_generic_node_t *
node_balance (chula_avl_generic_node_t *node)
{
	if (node->balance < -1) {
		if (node->left->balance > 0)
			node->left = node_rotate_left (node->left);
		node = node_rotate_right (node);

	} else if (node->balance > 1) {
		if (node->right->balance < 0)
			node->right = node_rotate_right (node->right);
		node = node_rotate_left (node);
	}

	return node;
}


static ret_t
node_add (chula_avl_generic_t *avl, chula_avl_generic_node_t *child)
{
	short                     re;
	bool                      is_left;
	chula_avl_generic_node_t *path[MAX_HEIGHT];
	chula_avl_generic_node_t *node   = avl->root;
	chula_avl_generic_node_t *parent = NULL;
	int32_t                   idx    = 1;

	path[0] = NULL;

	/* If the tree is empty..
	 */
	if (unlikely (avl->root == NULL)) {
		avl->root = child;
		return ret_ok;
	}

	/* Insert the node
	 */
	while (true) {
		re = avl->node_cmp (child, node, avl);

		if (re < 0) {
			/* Insert it on the left */
			if (node->left_child) {
				path[idx++] = node;
				node        = node->left;
			} else {
				child->left      = node->left;
				child->right     = node;
				node->left       = child;
				node->left_child = true;
				node->balance   -= 1;
				break;
			}

		} else if (re > 0) {
			/* Insert it on the left */
			if (node->right_child) {
				path[idx++] = node;
				node        = node->right;
			} else {
				child->right      = node->right;
				child->left       = node;
				node->right       = child;
				node->right_child = true;
				node->balance    += 1;
				break;
			}

		} else {
			node_free (child, avl);
			return ret_error;
		}
	}

	/* Rebalance the tree
	 */
	while (true) {
		parent = path[--idx];
		is_left = (parent && (parent->left == node));

		if ((node->balance < -1) ||
		    (node->balance >  1))
		{
			node = node_balance (node);
			if (parent == NULL)
				avl->root = node;
			else if (is_left)
				parent->left = node;
			else
				parent->right = node;
		}

		if ((node->balance == 0) || (parent == NULL))
			break;

		if (is_left)
			parent->balance -= 1;
		else
			parent->balance += 1;

		node = parent;
	}

	return ret_ok;
}


static int32_t
node_height (chula_avl_generic_node_t *node)
{
	int32_t left_height;
	int32_t right_height;

	if (node) {
		left_height = 0;
		right_height = 0;

		if (node->left_child)
			left_height = node_height (node->left);

		if (node->right_child)
			right_height = node_height (node->right);

		return MAX (left_height, right_height) + 1;
	}
	return 0;
}


static ret_t
node_check (chula_avl_generic_node_t *node)
{
	int32_t                   left_height;
	int32_t                   right_height;
	int32_t                   balance;
	chula_avl_generic_node_t *tmp;

	if (node == NULL)
		return ret_ok;

	if (node->left_child) {
		tmp = node_prev (node);
		if (tmp->right != node) {
            chula_log_error ("AVL Tree inconsistency: Right child");
			return ret_error;
		}
	}

	if (node->right_child) {
		tmp = node_next (node);
		if (tmp->left != node) {
            chula_log_error ("AVL Tree inconsistency: Left child");
			return ret_error;
		}
	}

	left_height  = 0;
	right_height = 0;

	if (node->left_child)
		left_height  = node_height (node->left);
	if (node->right_child)
		right_height = node_height (node->right);

	balance = right_height - left_height;
	if (balance != node->balance) {
        chula_log_error ("AVL Tree inconsistency: Balance");
		return ret_error;
	}

	if (node->left_child)
		node_check (node->left);
	if (node->right_child)
		node_check (node->right);

	return ret_ok;
}


ret_t
chula_avl_generic_add (chula_avl_generic_t *avl, chula_avl_generic_node_t *key, void *value)
{
	ret_t ret;

	if (unlikely (avl->node_is_empty (key)))
		return ret_error;

	/* Store the node value
	 */
	key->value = value;

	/* Add th node to the tree
	 */
	ret = node_add (avl, key);
	if (unlikely (ret != ret_ok)) {
		return ret;
	}

	return ret_ok;
}


ret_t
chula_avl_generic_del (chula_avl_generic_t *avl, chula_avl_generic_node_t *key, void **value)
{
	short                     re;
	bool                      is_left;
	chula_avl_generic_node_t *path[MAX_HEIGHT];
	chula_avl_generic_node_t *parent;
	chula_avl_generic_node_t *pbalance;
	chula_avl_generic_node_t *node      = avl->root;
	int32_t                   idx       = 1;

	if (unlikely (avl->node_is_empty (key)))
		return ret_error;

	if (avl->root == NULL)
		return ret_not_found;

	path[0] = NULL;

	while (true) {
		re = avl->node_cmp (key, node, avl);
		if (re == 0) {
			if (value)
				*value = node->value;
			break;
		} else if (re < 0) {
			if (!node->left_child)
				return ret_not_found;
			path[idx++] = node;
			node = node->left;
		} else {
			if (!node->right_child)
				return ret_not_found;
			path[idx++] = node;
			node = node->right;
		}
	}

	pbalance = path[idx-1];
	parent   = pbalance;
	idx     -= 1;
	is_left  = (parent && (node == parent->left));

	if (!node->left_child) {
		if (!node->right_child) {
			if (!parent)
				avl->root = NULL;
			else if (is_left) {
				parent->left_child  = false;
				parent->left        = node->left;
				parent->balance   += 1;
			} else {
				parent->right_child = false;
				parent->right       = node->right;
				parent->balance     -= 1;
			}

		} else { /* right child */
			chula_avl_generic_node_t *tmp = node_next (node);
			tmp->left = node->left;

			if (!parent)
				avl->root = node->right;
			else if (is_left) {
				parent->left     = node->right;
				parent->balance += 1;
			} else {
				parent->right    = node->right;
				parent->balance -= 1;
			}
		}

	} else { /* left child */
		if (!node->right_child) {
			chula_avl_generic_node_t *tmp = node_prev(node);
			tmp->right = node->right;

			if (parent == NULL)
				avl->root = node->left;
			else if (is_left) {
				parent->left     = node->left;
				parent->balance += 1;
			} else {
				parent->right    = node->left;
				parent->balance -= 1;
			}
		} else { /* both children */
			chula_avl_generic_node_t *prev    = node->left;
			chula_avl_generic_node_t *next    = node->right;
			chula_avl_generic_node_t *nextp   = node;
			int32_t                   old_idx = idx + 1;
			idx++;

			/* find the immediately next node (and its parent) */
			while (next->left_child) {
				path[++idx] = nextp = next;
				next = next->left;
			}

			path[old_idx] = next;
			pbalance      = path[idx];

			/* remove 'next' from the tree */
			if (nextp != node) {
				if (next->right_child)
					nextp->left = next->right;
				else
					nextp->left_child = false;

				nextp->balance    += 1;
				next->right_child  = true;
				next->right        = node->right;

			} else {
				node->balance -= 1;
			}

			/* set the prev to point to the right place */
			while (prev->right_child)
				prev = prev->right;
			prev->right = next;

			/* prepare 'next' to replace 'node' */
			next->left_child = true;
			next->left = node->left;
			next->balance = node->balance;

			if (!parent)
				avl->root = next;
			else if (is_left)
				parent->left = next;
			else
				parent->right = next;
		}
	}

	/* restore balance */
	if (pbalance) {
		while (true) {
			chula_avl_generic_node_t *bparent = path[--idx];
			is_left = (bparent && (pbalance == bparent->left));

			if(pbalance->balance < -1 || pbalance->balance > 1) {
				pbalance = node_balance (pbalance);
				if (!bparent)
					avl->root = pbalance;
				else if (is_left)
					bparent->left = pbalance;
				else
					bparent->right = pbalance;
			}

			if (pbalance->balance != 0 || !bparent)
				break;

			if (is_left)
				bparent->balance += 1;
			else
				bparent->balance -= 1;

			pbalance = bparent;
		}
	}

	node_free (node, avl);
	return ret_ok;
}


ret_t
chula_avl_generic_get (chula_avl_generic_t *avl, chula_avl_generic_node_t *key, void **value)
{
	short                     re;
	chula_avl_generic_node_t *node;

	if (unlikely (avl->node_is_empty (key)))
		return ret_error;

	node = avl->root;
	if (!node)
		return ret_not_found;

	while (true) {
		re = avl->node_cmp (key, node, avl);
		if (re == 0) {
			if (value)
				*value = node->value;
			return ret_ok;

		} else if (re < 0) {
			if (!node->left_child)
				return ret_not_found;
			node = node->left;

		} else {
			if (!node->right_child)
				return ret_not_found;
			node = node->right;
		}
	}

	SHOULDNT_HAPPEN;
	return ret_error;
}


ret_t
chula_avl_check (chula_avl_generic_t *avl)
{
	return node_check (avl->root);
}


ret_t
chula_avl_generic_while (chula_avl_generic_t             *avl,
                         chula_avl_generic_while_func_t   func,
                         void                            *param,
                         chula_avl_generic_node_t       **key,
                         void                           **value)
{
	ret_t                     ret;
	chula_avl_generic_node_t *node = avl->root;

	if (avl->root == NULL) {
		return ret_ok;
	}

	node = node_first (avl);
	while (node) {
		if (key)
			*key = node;
		if (value)
			*value = node->value;

		ret = func (node, node->value, param);
		if (ret != ret_ok) return ret;

		node = node_next (node);
	}

	return ret_ok;
}


static ret_t
func_len_each (chula_avl_generic_node_t *key, void *value, void *param)
{
	UNUSED(key);
	UNUSED(value);

	*((size_t *)param) += 1;
	return ret_ok;
}


ret_t
chula_avl_len (chula_avl_generic_t *avl, size_t *len)
{
	*len = 0;
	return chula_avl_generic_while (avl, func_len_each, len, NULL, NULL);
}


ret_t
chula_avl_mrproper (chula_avl_generic_t *avl,
                    chula_func_free_t    free_func)
{
	chula_avl_generic_node_t *node;
	chula_avl_generic_node_t *next;

	if (unlikely (avl == NULL))
		return ret_ok;

	node = node_first (avl);

	while (node) {
		next = node_next (node);

		/* Node content */
		if (free_func) {
			free_func (node->value);
		}

		/* Node itself */
		node_free (node, avl);

		node = next;
	}

	return ret_ok;
}

ret_t
chula_avl_free (chula_avl_generic_t *avl,
                chula_func_free_t    free_func)
{
	ret_t ret;
	ret = chula_avl_mrproper (avl, free_func);

	free (avl);
	return ret;
}


int
chula_avl_is_empty (chula_avl_generic_t *avl)
{
	return (avl->root == NULL);
}
