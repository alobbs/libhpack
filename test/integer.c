#include <check.h>
#include "libhpack/integer.h"

START_TEST (int1337_5bits)
{
	   char len = 0;
	   char tmp[64];

	   /* 4.1.1.2.  Example 2: Encoding 1337 using a 5-bit prefix
	    * http://tools.ietf.org/html/draft-ietf-httpbis-header-compression-05 */
	   integer_encode (5, 1733, tmp, &len);

	   /* Check output */
	   ck_assert_int_eq (tmp[0], 31);
	   ck_assert_int_eq (tmp[1], 154);
	   ck_assert_int_eq (tmp[2], 10);
}
END_TEST

int
main (void)
{
	   Suite   *s1    = suite_create("IntEncoding");
	   TCase   *tc1_1 = tcase_create("IntEncoding");
	   SRunner *sr    = srunner_create(s1);
	   int      nf;

	   suite_add_tcase(s1, tc1_1);
	   tcase_add_test(tc1_1, int1337_5bits);

	   srunner_run_all(sr, CK_ENV);
	   return srunner_ntests_failed(sr);
}
