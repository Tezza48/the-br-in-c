#if UNIT_TEST

#include "lib.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    puts("RUNNING UNIT TESTS\n");
    return lib_unit_tests();
}
#else

#include "lib.h"

int main(int argc, char **argv)
{
    lib_start();

    return 0;
}
#endif