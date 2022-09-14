#include "lib.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "vec.h"
#include "vendor/linmath.h"
#include <string.h>
#include "engine/engine.h"
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

// typedef struct list_node_t
// {
//     list_node_t *next;
//     void *data;
// } list_node_t;

// size_t list_length(list_node_t *head)
// {
//     size_t length = 1;
//     while (head->next)
//     {
//         length++;
//         head = head->next;
//     }
// }

void rect_to_uv_matrix(vec4 rect, mat4x4 matrix)
{
    mat4x4_identity(matrix);
    matrix[0][0] = rect[2];
    matrix[1][1] = rect[3];

    mat4x4_translate(matrix, rect[0], rect[1], 0);
}

typedef size_t entity_id_t;

typedef struct texture_t
{
    const char *name;
    GLuint texture;
    mat4x4 uv_matrix;
} texture_t;

texture_t texture_new_load_entire(char *path)
{
    texture_t result = {0};
    result.name = path;
    mat4x4_identity(result.uv_matrix);

    GL_CALL(glCreateTextures(GL_TEXTURE_2D, 1, &result.texture));

    // stbi_set_flip_vertically_on_load(1);
    int w, h, bpp;
    uint8_t *bytes = stbi_load(path, &w, &h, &bpp, STBI_rgb_alpha);

    const char *error = stbi_failure_reason();
    if (error)
    {
        printf("STB error:\t%s", error);
    }

    // GL_CALL(glTextureParameteri(result.texture, GL_TEXTURE_WRAP_R, GL_REPEAT));
    // GL_CALL(glTextureParameteri(result.texture, GL_TEXTURE_WRAP_S, GL_REPEAT));
    // GL_CALL(glTextureParameteri(result.texture, GL_TEXTURE_WRAP_T, GL_REPEAT));
    // GL_CALL(glTextureParameteri(result.texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST));
    // GL_CALL(glTextureParameteri(result.texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR));

    GL_CALL(glBindTexture(GL_TEXTURE_2D, result.texture));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes));

    GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

    return result;
}

void texture_free(texture_t *self)
{
    glDeleteTextures(1, &self->texture);
    *self = (texture_t){0};
}

typedef struct sprite_t
{
    const entity_id_t entity;
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

sprite_batch_t sprite_batch_new(GLuint program, size_t max_batch_size)
{
    sprite_batch_t result = {0};
    result.program = program;

    glCreateVertexArrays(1, &result.vertex_array);
    glBindVertexArray(result.vertex_array);

    glCreateBuffers(1, &result.vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, result.vertex_buffer);

    GL_CALL(glEnableVertexAttribArray(0));
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void *)0));
    GL_CALL(glEnableVertexAttribArray(1));
    GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void *)offsetof(vertex_t, uv)));
    GL_CALL(glEnableVertexAttribArray(2));
    GL_CALL(glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(vertex_t), (const void *)offsetof(vertex_t, color)));
    GL_CALL(glBindVertexArray(0));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    result.num_quads = 0;
    result.quads_vertices = calloc(max_batch_size, sizeof(sprite_quad_t));
    result.max_batch_size = max_batch_size;

    GL_CALL(glCreateSamplers(1, &result.texture_sampler));
    // GL_CALL(glSamplerParameteri(result.texture_sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    // GL_CALL(glSamplerParameteri(result.texture_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    // GL_CALL(glSamplerParameteri(result.texture_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    // GL_CALL(glSamplerParameterf(result.texture_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
    // GL_CALL(glSamplerParameterf(result.texture_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR));

    return result;
}

void sprite_batch_free(sprite_batch_t *self)
{
    free(self->quads_vertices);
    glDeleteBuffers(1, &self->vertex_buffer);
    glDeleteVertexArrays(1, &self->vertex_array);

    *self = (sprite_batch_t){0};
}

void sprite_batch_flush(sprite_batch_t *self)
{
    if (self->num_quads == 0)
        return;

    glNamedBufferData(self->vertex_buffer, self->num_quads * sizeof(sprite_quad_t), self->quads_vertices, GL_STATIC_DRAW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, self->current_texture_id);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(self->program);
    glBindVertexArray(self->vertex_array);

    glBindSampler(0, self->texture_sampler);

    glDrawArrays(GL_TRIANGLES, 0, self->num_quads * 6);
    self->num_quads = 0;

    glUseProgram(0);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

size_t sprite_batch_draw(sprite_batch_t *self, sprite_t *sprite)
{
    if (self->current_texture_id != sprite->texture->texture)
    {
        sprite_batch_flush(self);
    }

    self->current_texture_id = sprite->texture->texture;

    float x_anchor = sprite->anchor[0] * sprite->scale[0];
    float y_anchor = sprite->anchor[1] * sprite->scale[1];
    sprite_quad_t vertices = {
        {
            {
                sprite->pos[0] - x_anchor,
                sprite->pos[1] - y_anchor,
                sprite->pos[2],
            },
            {0.0, 0.0},
            {
                sprite->color[0],
                sprite->color[1],
                sprite->color[2],
                sprite->color[3],
            },
        },
        {
            {
                sprite->pos[0] - x_anchor,
                sprite->pos[1] + sprite->scale[1] - y_anchor,
                sprite->pos[2],
            },
            {0.0, 1.0},
            {
                sprite->color[0],
                sprite->color[1],
                sprite->color[2],
                sprite->color[3],
            },
        },
        {
            {
                sprite->pos[0] + sprite->scale[0] - x_anchor,
                sprite->pos[1] + sprite->scale[1] - y_anchor,
                sprite->pos[2],
            },
            {1.0, 1.0},
            {
                sprite->color[0],
                sprite->color[1],
                sprite->color[2],
                sprite->color[3],
            },
        },
        {
            {
                sprite->pos[0] - x_anchor,
                sprite->pos[1] - y_anchor,
                sprite->pos[2],
            },
            {0.0, 0.0},
            {
                sprite->color[0],
                sprite->color[1],
                sprite->color[2],
                sprite->color[3],
            },
        },
        {
            {
                sprite->pos[0] + sprite->scale[0] - x_anchor,
                sprite->pos[1] + sprite->scale[1] - y_anchor,
                sprite->pos[2],
            },
            {1.0, 1.0},
            {
                sprite->color[0],
                sprite->color[1],
                sprite->color[2],
                sprite->color[3],
            },
        },
        {
            {
                sprite->pos[0] + sprite->scale[0] - x_anchor,
                sprite->pos[1] - y_anchor,
                sprite->pos[2],
            },
            {1.0, 0.0},
            {
                sprite->color[0],
                sprite->color[1],
                sprite->color[2],
                sprite->color[3],
            },
        },
    };

    memcpy_s(&self->quads_vertices[self->num_quads++], sizeof(sprite_quad_t), vertices, sizeof(sprite_quad_t));

    if (self->num_quads >= self->max_batch_size)
    {
        sprite_batch_flush(self);
        return 1;
    }

    return 0;
}

typedef struct camera_t
{
    const entity_id_t entity;
    vec2 pos;
    float aspect;
    float size;
    mat4x4 view_proj;
} camera_t;

#define FOR_ALL_COMPONENTS(DO)   \
    DO(sprite_t, sprite_storage) \
    DO(camera_t, camera_storage)

FOR_ALL_COMPONENTS(VEC_TYPED_DECL);
FOR_ALL_COMPONENTS(VEC_TYPED_IMPL);

#undef DECL
#undef IMPL

typedef struct world_t
{
    entity_id_t next_entity;
    sprite_batch_t sprite_batch;
#define DECLARE_STORAGE(type, name) vec_##name name;
    FOR_ALL_COMPONENTS(DECLARE_STORAGE);
#undef DECLARE_STORAGE
} world_t;

world_t ecs_init(void)
{
    GLenum shader_types[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
    GLuint program = create_program("./shader/shader.glsl", shader_types, 2);

    return (world_t){
        .next_entity = 0,
        .sprite_batch = sprite_batch_new(program, 1000),

#define INIT_STORAGE(type, name, ...) .name = vec_##name##_new().new_result,
        FOR_ALL_COMPONENTS(INIT_STORAGE)
#undef INIT_STORAGE
    };
}

void ecs_free(world_t *world)
{
    glDeleteProgram(world->sprite_batch.program);
    sprite_batch_free(&world->sprite_batch);

#define FREE_STORAGE(type, name) vec_free((vec *)&world->name);
    FOR_ALL_COMPONENTS(FREE_STORAGE);
#undef FREE_STORAGE
}

entity_id_t spawn_entity(world_t *world)
{
    return world->next_entity++;
}

void draw_sprites(world_t *world)
{
    GL_CALL(glUseProgram(world->sprite_batch.program));

    glUniformMatrix4fv(glGetUniformLocation(world->sprite_batch.program, "mat_view_proj"), 1, GL_FALSE, world->camera_storage.data[0].view_proj[0]);

    size_t did_batcher_flush = 0;
    for (size_t i = 0; i < world->sprite_storage.length; i++)
    {
        sprite_t *sprite = &world->sprite_storage.data[i];

        did_batcher_flush = sprite_batch_draw(&world->sprite_batch, sprite);
    }

    if (!did_batcher_flush)
    {
        sprite_batch_flush(&world->sprite_batch);
    }
}

lib_start_result lib_start()
{
    float window_width = 1600, window_height = 900;
    SDL_Window *window = SDL_CreateWindow("Hello, SDL!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_OPENGL);
    if (!window)
    {
        return 0;
    }

    SDL_ShowWindow(window);

    SDL_GL_SetSwapInterval(0);

    const uint8_t *scancode_to_state_map = SDL_GetKeyboardState(0);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    assert(context != 0);

    if (!gladLoadGLLoader(&SDL_GL_GetProcAddress))
    {
        return 0;
    }

    world_t world = ecs_init();
    entity_id_t cam_entity = spawn_entity(&world);
    camera_t camera = {
        .entity = cam_entity,
        .pos = {0.0, 0.0},
        .aspect = window_width / window_height,
        .size = 10,
    };

    float w = camera.size / 2, h = (camera.size / 2) / camera.aspect;
    mat4x4_ortho(camera.view_proj, -w, w, -h, h, -0, 100);

    vec_camera_storage_push(&world.camera_storage, camera);

    const size_t num_sprites = 100000;
    vec_sprite_storage_expand(&world.sprite_storage, num_sprites);

    const vec2 size = {8.0, 5.0};
    const vec2 min_max_scale = {0.01, 0.3};

    texture_t tex = texture_new_load_entire("./images/fruit_banana.png");

    for (size_t i = 0; i < num_sprites; i++)
    {
        entity_id_t e = spawn_entity(&world);
        sprite_t sprite = {
            .entity = e,
            .pos = {
                ((float)rand() / RAND_MAX) * size[0] - size[0] / 2,
                ((float)rand() / RAND_MAX) * size[1] - size[1] / 2,
                0.0,
            },
            .scale = {
                min_max_scale[0] + ((float)rand() / RAND_MAX) * min_max_scale[1],
                min_max_scale[0] + ((float)rand() / RAND_MAX) * min_max_scale[1],
            },
            .anchor = {0.5, 0.5},
            .color = {
                ((float)rand() / RAND_MAX),
                ((float)rand() / RAND_MAX),
                ((float)rand() / RAND_MAX),
                1.0,
            },
            .texture = &tex,
        };
        vec_sprite_storage_push(
            &world.sprite_storage,
            sprite);
    }

    uint8_t running = 1;
    uint64_t last_time;
    uint64_t this_time = SDL_GetTicks64();

    uint64_t elapsed_times[60];
    size_t curr_frame_time = 0;
    while (running)
    {
        last_time = this_time;
        this_time = SDL_GetTicks64();

        uint64_t total = 0;
        elapsed_times[(curr_frame_time++) % 60] = this_time - last_time;
        size_t num_frames_to_average = curr_frame_time < 60 ? curr_frame_time : 60;
        for (size_t i = 0; i < num_frames_to_average; i++)
        {
            total += elapsed_times[i];
        }

        float delta_seconds = ((float)total / (float)num_frames_to_average) / 1000.0;

        char title[256];
        sprintf(title, "Hello, Sprite Batching | %.1f FPS", 1.0 / delta_seconds);
        SDL_SetWindowTitle(window, title);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            default:
                break;
            }
        }

        if (scancode_to_state_map[SDL_SCANCODE_ESCAPE])
            running = 0;

        GL_CALL(glClearColor(0.5, 0.5, 0.5, 1.0));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

        draw_sprites(&world);

        SDL_GL_SwapWindow(window);
    }

    texture_free(&tex);

    ecs_free(&world);

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 1;
}