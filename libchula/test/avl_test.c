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

#include <libchula-qa/libchula-qa.h>
#include <libchula-qa/testing_macros-internal.h>


static void
fake_free (void *p)
{
    UNUSED(p);
}

START_TEST (new_)
{
    ret_t        ret;
    chula_avl_t *avl = NULL;

    ret = chula_avl_new (&avl);
    ch_assert (avl != NULL);
    ch_assert (ret == ret_ok);

    ret = chula_avl_free (AVL_GENERIC(avl), NULL);
    ch_assert (ret == ret_ok);
}
END_TEST

START_TEST (empty)
{
    ret_t       ret;
    chula_avl_t avl;

    ret = chula_avl_init (&avl);
    ch_assert (ret == ret_ok);

    chula_avl_mrproper (AVL_GENERIC(&avl), NULL);
}
END_TEST

START_TEST (_ptr)
{
    ret_t        ret;
    void        *p;
    chula_avl_t  avl;

    chula_avl_init (&avl);

    ret = chula_avl_add_ptr (&avl, "uno", INT_TO_POINTER(1));
    ch_assert (ret == ret_ok);
    ret = chula_avl_add_ptr (&avl, "dos", INT_TO_POINTER(2));
    ch_assert (ret == ret_ok);
    ret = chula_avl_add_ptr (&avl, "tres", INT_TO_POINTER(3));
    ch_assert (ret == ret_ok);

    chula_avl_get_ptr (&avl, "dos", &p);
    ch_assert (POINTER_TO_INT(p) == 2);
    chula_avl_get_ptr (&avl, "tres", &p);
    ch_assert (POINTER_TO_INT(p) == 3);
    chula_avl_get_ptr (&avl, "uno", &p);
    ch_assert (POINTER_TO_INT(p) == 1);

    chula_avl_set_case (&avl, true);
    chula_avl_get_ptr (&avl, "TrEs", &p);
    ch_assert (POINTER_TO_INT(p) == 3);
    chula_avl_set_case (&avl, false);

    chula_avl_del_ptr (&avl, "dos", NULL);
    chula_avl_get_ptr (&avl, "uno", &p);
    ch_assert (POINTER_TO_INT(p) == 1);

    p = NULL;
    ret = chula_avl_get_ptr (&avl, "dos", &p);
    ch_assert (ret != ret_ok);
    ch_assert (p == NULL);

    ret = chula_avl_check (AVL_GENERIC(&avl));
    ch_assert (ret == ret_ok);

    chula_avl_mrproper (AVL_GENERIC(&avl), NULL);
}
END_TEST

START_TEST (_buf)
{
    ret_t           ret;
    void           *p;
    chula_avl_t     avl;
    chula_buffer_t  uno  = CHULA_BUF_INIT;
    chula_buffer_t  dos  = CHULA_BUF_INIT;
    chula_buffer_t  tres = CHULA_BUF_INIT;

    chula_avl_init (&avl);
    chula_buffer_add_str (&uno,  "uno");
    chula_buffer_add_str (&dos,  "dos");
    chula_buffer_add_str (&tres, "tres");

    ret = chula_avl_add (&avl, &uno, INT_TO_POINTER(1));
    ch_assert (ret == ret_ok);
    ret = chula_avl_add (&avl, &dos, INT_TO_POINTER(2));
    ch_assert (ret == ret_ok);
    ret = chula_avl_add (&avl, &tres, INT_TO_POINTER(3));
    ch_assert (ret == ret_ok);

    chula_avl_get (&avl, &dos, &p);
    ch_assert (POINTER_TO_INT(p) == 2);
    chula_avl_get (&avl, &tres, &p);
    ch_assert (POINTER_TO_INT(p) == 3);
    chula_avl_get (&avl, &uno, &p);
    ch_assert (POINTER_TO_INT(p) == 1);

    chula_avl_del (&avl, &dos, NULL);
    chula_avl_get (&avl, &uno, &p);
    ch_assert (POINTER_TO_INT(p) == 1);

    p = NULL;
    ret = chula_avl_get (&avl, &dos, &p);
    ch_assert (ret != ret_ok);
    ch_assert (p == NULL);

    ret = chula_avl_check (AVL_GENERIC(&avl));
    ch_assert (ret == ret_ok);

    chula_buffer_mrproper (&uno);
    chula_buffer_mrproper (&dos);
    chula_buffer_mrproper (&tres);
    chula_avl_mrproper (AVL_GENERIC(&avl), NULL);
}
END_TEST

START_TEST (len)
{
    ret_t       ret;
    size_t      len;
    chula_avl_t avl;

    /* None */
    chula_avl_init (&avl);
    ch_assert (chula_avl_is_empty (AVL_GENERIC(&avl)));

    ret = chula_avl_len (AVL_GENERIC(&avl), &len);
    ch_assert (ret == ret_ok);
    ch_assert (len == 0);

    /* Single */
    chula_avl_add_ptr (&avl, "uno", INT_TO_POINTER(1));
    ch_assert (ret == ret_ok);
    ch_assert (! chula_avl_is_empty (AVL_GENERIC(&avl)));

    chula_avl_len (AVL_GENERIC(&avl), &len);
    ch_assert (len == 1);

    /* Multiple */
    chula_avl_add_ptr (&avl, "dos", INT_TO_POINTER(2));
    ch_assert (ret == ret_ok);
    ch_assert (! chula_avl_is_empty (AVL_GENERIC(&avl)));

    chula_avl_len (AVL_GENERIC(&avl), &len);
    ch_assert (len == 2);

    chula_avl_mrproper (AVL_GENERIC(&avl), NULL);
}
END_TEST

START_TEST (stress)
{
    ret_t       ret;
    chula_avl_t avl;
    int         pi[] = {3, 14, 15, 92, 65, 35, 89, 79, 32, 38, 46, 26, 43, 38, 32, 79, 50};

    ret = chula_avl_init (&avl);
    ch_assert (ret == ret_ok);

    /* Add */
    for (int i=0; i<16; i++) {
        char tmp[10];
        snprintf (tmp, 10, "%d", pi[i]);
        chula_avl_add_ptr (&avl, tmp, NULL);
    }

    /* Remove */
    for (int i=15; i>=0; i--) {
        char tmp[10];
        snprintf (tmp, 10, "%d", pi[i]);
        chula_avl_del_ptr (&avl, tmp, NULL);
    }

    ret = chula_avl_mrproper (AVL_GENERIC(&avl), fake_free);
    ch_assert (ret == ret_ok);
}
END_TEST

START_TEST (mrproper)
{
    ret_t       ret;
    chula_avl_t avl;

    chula_avl_init (&avl);

    for (int i=0; i<16; i++) {
        char tmp[3];
        snprintf (tmp, 3, "%d", i);
        chula_avl_add_ptr (&avl, tmp, NULL);
    }

    ret = chula_avl_mrproper (AVL_GENERIC(&avl), fake_free);
    ch_assert (ret == ret_ok);
}
END_TEST

static ret_t
while_func (chula_buffer_t *key,
            void           *value,
            void           *param)
{
    UNUSED(key);

    *((int *)param) += POINTER_TO_INT(value);
    return ret_ok;
}

START_TEST (_while)
{
    ret_t           ret;
    chula_avl_t     avl;
    int             total    = 0;
    int             expected = 0;
    chula_buffer_t *key   = NULL;
    void           *value = NULL;

    chula_avl_init (&avl);

    for (int i=0; i<1000; i++) {
        char tmp[5];
        expected += i;
        snprintf (tmp, 5, "%d", i);
        chula_avl_add_ptr (&avl, tmp, INT_TO_POINTER(i));
    }

    ret = chula_avl_while (AVL_GENERIC(&avl), while_func, &total, &key, &value);
    ch_assert (ret == ret_ok);
    ch_assert (total == expected);

    chula_avl_mrproper (AVL_GENERIC(&avl), NULL);
}
END_TEST


int
avl_tests (void)
{
    Suite *s1 = suite_create("AVL");

    check_add (s1, new_);
    check_add (s1, empty);
    check_add (s1, _ptr);
    check_add (s1, _buf);
    check_add (s1, len);
    check_add (s1, stress);
    check_add (s1, mrproper);
    check_add (s1, _while);
    run_test (s1);
}
