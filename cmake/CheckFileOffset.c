#include <sys/types.h>

#define  B ((off_t) 1)
#define KB ((off_t) 1024 * B)
#define MB ((off_t) 1024 * KB)
#define GB ((off_t) 1024 * MB)

int testArray [(((off_t)4*GB) == (int64_t)4ULL*GB) ? 1 : -1];

int main()
{
    testArray [0] = 1;
    return 0;
}

