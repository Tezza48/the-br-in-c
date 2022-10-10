#pragma once
#include "vendor/linmath.h"
#include "font.h"

typedef struct text_t
{
    char *text;
    font_t *font;
    float font_size;
} text_t;