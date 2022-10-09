#pragma once
#include "entities.h"
#include "vendor/linmath.h"

typedef struct transform_t
{
    mat4x4 global_matrix;
    mat4x4 local_matrix;

    uint8_t is_dirty;

    vec3 pos;
    vec2 scale;

} transform_t;