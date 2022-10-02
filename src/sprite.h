#pragma once
#include "texture.h"
#include "vendor/linmath.h"

typedef struct sprite_t
{
    vec3 pos;
    vec2 scale;
    vec2 anchor;
    vec4 color;
    texture_t *texture;
} sprite_t;