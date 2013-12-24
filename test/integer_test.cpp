#include <gtest/gtest.h>
#include "lib_hpack/integer.h"

TEST(Encoding, Integer) {
	   char len = 0;
	   char tmp[64];
	   integer_encode (5, 1733, tmp, &len);
}

int
main (int argc, char **argv)
{
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}
