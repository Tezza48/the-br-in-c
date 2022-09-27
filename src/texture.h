#pragma once
#include <glad/glad.h>
#include "vendor/linmath.h"

typedef struct texture_t
{
    const char *name;
    GLuint texture;
    mat4x4 uv_matrix;
} texture_t;

texture_t texture_new_load_entire(const char *path);

void texture_cleanup(texture_t *self);

void texture_free(texture_t *self);
