#pragma once
#include <stdint.h>

typedef uint8_t lib_start_result;
lib_start_result lib_start();

#if UNIT_TEST
#include "entities.h"
#include "stdio.h"

static int lib_unit_tests()
{
    int32_t success = entities_unit_tests();

    if (success)
    {
        puts("All Lib Unit Tests passed successfully\n");
    }

    return success;
}
#endif
