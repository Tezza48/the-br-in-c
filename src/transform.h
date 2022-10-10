#pragma once
#include "vendor/linmath.h"

typedef struct app_t app_t;

typedef struct transform_t
{
    mat4x4 global_matrix;
    mat4x4 local_matrix;

    uint8_t is_dirty;

    vec3 pos;
    vec2 scale;
} transform_t;

void set_pos(transform_t *transform, vec3 pos);

void set_scale(transform_t *transform, vec2 scale);

void update_local(transform_t *transform);

typedef struct app_t app_t;
// void sort_transforms(app_t *app);
void update_local_system(app_t *app);
void update_global_system(app_t *app);