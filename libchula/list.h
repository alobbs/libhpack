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

#ifndef CHULA_LIST_H
#define CHULA_LIST_H

#if !defined(CHULA_H_INSIDE) && !defined (CHULA_COMPILATION)
# error "Only <libchula/libchula.h> can be included directly."
#endif

#include <libchula/common.h>

struct list_entry {
	struct list_entry *next;
	struct list_entry *prev;
};

typedef struct list_entry chula_list_t;
typedef struct list_entry chula_list_entry_t;


#define LIST(l) ((chula_list_t *)(l))

#define INIT_LIST_HEAD(ptr) do {                \
		(ptr)->next = (ptr);                    \
		(ptr)->prev = (ptr);                    \
	} while (0)

#define LIST_HEAD_INIT(ptr) { &(ptr), &(ptr) }

#define list_entry(ptr, type, member)                               \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#define list_for_each(pos, head)                                \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head)                                \
	for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

#define list_for_each_entry(pos, head, member)                          \
    for (pos = list_entry((head)->next, typeof(*pos), member);          \
         &pos->member != (head);                                        \
         pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_next_circular(list,item)                           \
	(((item)->next == (list)) ? (list)->next : (item)->next)

static inline int
chula_list_empty (chula_list_t *list)
{
	return (list->next == list);
}

static inline void
chula_list_add (chula_list_t *new_entry, chula_list_t *head)
{
	new_entry->next  = head->next;
	new_entry->prev  = head;
	head->next->prev = new_entry;
	head->next       = new_entry;
}

static inline void
chula_list_add_tail (chula_list_t *new_entry, chula_list_t *head)
{
	new_entry->next  = head;
	new_entry->prev  = head->prev;
	head->prev->next = new_entry;
	head->prev       = new_entry;
}

static inline void
chula_list_del (chula_list_t *entry)
{
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
}

static inline void
chula_list_reparent (chula_list_t *list, chula_list_t *new_entry)
{
	if (chula_list_empty(list))
		return;

	new_entry->next = list->next;
	new_entry->prev = list->prev;
	new_entry->prev->next = new_entry;
	new_entry->next->prev = new_entry;
}

void  chula_list_sort    (chula_list_t *head, int (*cmp)(chula_list_t *a, chula_list_t *b));
ret_t chula_list_get_len (chula_list_t *head, size_t *len);
ret_t chula_list_invert  (chula_list_t *head);


/* Methods for non list elements
 */

typedef void (*chula_list_free_func) (void *);

typedef struct {
	chula_list_entry_t  list;
	void               *info;
} chula_list_item_t;

#define LIST_ITEM(i)      ((chula_list_item_t *)(i))
#define LIST_ITEM_INFO(i) (LIST_ITEM(i)->info)

ret_t chula_list_content_add              (chula_list_t *head, void *item);
ret_t chula_list_content_add_tail         (chula_list_t *head, void *item);

ret_t chula_list_content_free             (chula_list_t *head, chula_list_free_func free_func);
ret_t chula_list_content_free_item        (chula_list_t *head, chula_list_free_func free_func);
ret_t chula_list_content_free_item_simple (chula_list_t *head);

#endif /* CHULA_LIST_H */
