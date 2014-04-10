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

#include "libchula/testing_macros.h"

typedef struct {
    int foo;
} test_obj1_t;

static ret_t
test_obj1_new (test_obj1_t **obj, int val)
{
    *obj = INT_TO_POINTER(val);
    return ret_ok;
}

static ret_t
gen_new_obj_w_ret (void)
{
    test_obj1_t *obj = CHULA_GEN_NEW_OBJ(test,obj1,1234);
    ch_assert (POINTER_TO_INT(obj) == 1234);
    return ret_ok;
}

static ret_t
gen_new_obj_3_w_ret (void)
{
    test_obj1_t *a = CHULA_GEN_NEW_OBJ(test,obj1,123);
    test_obj1_t *b = CHULA_GEN_NEW_OBJ(test,obj1,456);
    test_obj1_t *c = CHULA_GEN_NEW_OBJ(test,obj1,789);
    ch_assert (POINTER_TO_INT(a) == 123);
    ch_assert (POINTER_TO_INT(b) == 456);
    ch_assert (POINTER_TO_INT(c) == 789);
    return ret_ok;
}


START_TEST (gen_new_obj) {
    gen_new_obj_w_ret();
}
END_TEST
START_TEST (gen_new_obj_3) {
    gen_new_obj_3_w_ret();
}
END_TEST

int
macro_tests (void)
{
    Suite *s1 = suite_create("Macros");
    check_add (s1, gen_new_obj);
    check_add (s1, gen_new_obj_3);
    run_test (s1);
}
