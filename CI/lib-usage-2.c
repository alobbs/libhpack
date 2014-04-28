/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <libchula/libchula.h>

int
main()
{
    ret_t          ret;
    chula_buffer_t buf = CHULA_BUF_INIT;

    ret = chula_buffer_add_str (&buf, "test");
    if (ret != ret_ok) return 1;

    chula_buffer_mrproper(&buf);
    return 0;
}
