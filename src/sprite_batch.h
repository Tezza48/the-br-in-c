#pragma once
#include <glad/glad.h>
#include "vendor/linmath.h"

typedef struct app_t app_t;

typedef struct vertex_t
{
    vec3 pos;
    vec2 uv;
    vec4 color;
} vertex_t;

typedef vertex_t sprite_quad_t[6];

typedef struct sprite_batch_t
{
    GLuint vertex_array;
    GLuint vertex_buffer;
    sprite_quad_t *quads_vertices;
    size_t num_quads;
    GLuint program;
    size_t max_batch_size;
    GLuint current_texture_id;
    GLuint texture_sampler;
} sprite_batch_t;

sprite_batch_t sprite_batch_new(GLuint program, size_t max_batch_size);

void sprite_batch_free(sprite_batch_t *self);

void sprite_batch_render_system(app_t *app);

uint8_t sprite_batch_submit_quad(sprite_batch_t *batch, sprite_quad_t quad, GLuint texture);

void sprite_batch_flush(sprite_batch_t *self);