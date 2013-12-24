// http://tools.ietf.org/html/draft-ietf-httpbis-header-compression-05

#include <stdio.h>
#include <math.h>

void
integer_encode (int N, int value, char *mem, char *mem_len)
{
	   int i = 0;

	   /* N is always between 1 and 8 bits [4.1.1.]
	    */
	   const int limits[] = {0, 1, 3, 7, 15, 31, 63, 127, 255};
	   const int limit    = limits[N];

	   /*
	    */
	   if (value < limit) {
			 mem[i++] = (char)value;
			 *mem_len = i;
			 return;
	   } else {
			 mem[i++] = (char)limit;
			 value -= limit;
			 while (value >= 128) {
				    mem[i++] = value % 128 + 128;
				    value /= 128;
			 }
			 mem[i++] = (char)value;
			 *mem_len = i;
	   }

	   printf ("limit %d\n", limit);
}
