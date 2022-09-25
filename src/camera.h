#pragma once
#include "vendor/linmath.h"

typedef struct camera_t
{
    vec2 pos;
    float aspect;
    float size;
    mat4x4 view_proj;
} camera_t;