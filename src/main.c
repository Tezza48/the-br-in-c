#include "lib.h"

#include "vec_tests.h"

int main(int argc, char **argv)
{
    lib_start();

    return 0;
}

// #include <stdio.h>
// #include <SDL2/SDL.h>
// #include <unistd.h>

// enum main_args
// {
//     MAIN_ARGS_CWD = 0,
//     MAIN_ARGS_SET_WD = 1,
// };

// int main(int argc, char **argv)
// {
//     if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
//     {
//         printf("Error: %s", SDL_GetError());
//         return 0;
//     }

//     puts("Success");
//     SDL_Quit();

//     return 0;
// }
