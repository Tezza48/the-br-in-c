#pragma once
#include "vendor/linmath.h"
#include "font.h"

typedef struct text_t
{
    char *text;
    font_t *font;
    float font_size;
    vec3 pos;
    vec2 scale;
} text_t;