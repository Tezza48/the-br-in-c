#pragma once
#include "texture.h"
#include "vendor/linmath.h"

typedef struct sprite_t
{
    vec2 anchor;
    vec4 color;
    texture_t *texture;
} sprite_t;