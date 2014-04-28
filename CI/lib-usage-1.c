/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <libhpack/libhpack.h>

int
main()
{
    ret_t                 ret;
    hpack_header_table_t *table;

    ret = hpack_header_table_new (&table);
    if (ret != ret_ok) return 1;

    hpack_header_table_free (table);
    return 0;
}
