#pragma once
#include "texture.h"
#include "vendor/linmath.h"
#include "entities.h"

typedef struct sprite_t sprite_t;
typedef struct sprite_batch_t sprite_batch_t;

typedef struct sprite_t
{
    vec3 pos;
    vec2 scale;
    vec2 anchor;
    vec4 color;
    texture_t *texture;
} sprite_t;

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
// void sprite_batch_flush(sprite_batch_t *self);
// size_t sprite_batch_draw(sprite_batch_t *self, sprite_t *sprite);
void draw_sprites(world_t *world);

void sprite_batch_flush(sprite_batch_t *self);