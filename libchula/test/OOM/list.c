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

#include "oom.h"

int
list_add (void)
{
    ret_t        ret;
    size_t       s1, s2;
    chula_list_t list    = LIST_HEAD_INIT(list);

    for (int i=0; i<10; i++) {
        /* Add a new list entry */
        chula_list_get_len (&list, &s1);
        ret = chula_list_content_add (&list, INT_TO_POINTER(i));
        chula_list_get_len (&list, &s2);

        /* Check return code */
        assert (((ret == ret_ok) && (s2 > s1)) ||
                ((ret == ret_nomem) && (s2 <= s1)));
    }

    chula_list_content_free (&list, NULL);

    for (int i=0; i<10; i++) {
        /* Add a new list entry */
        chula_list_get_len (&list, &s1);
        ret = chula_list_content_add_tail (&list, INT_TO_POINTER(100 + i));
        chula_list_get_len (&list, &s2);

        /* Check return code */
        assert (((ret == ret_ok) && (s2 > s1)) ||
                ((ret == ret_nomem) && (s2 <= s1)));
    }

    chula_list_content_free (&list, NULL);

    return 0;
}
