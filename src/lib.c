#include "lib.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <math.h>

// ! Ideally engine wouldn't be included in the user code unless they're implementing extensions/plugins.
#include "engine/engine.h"

#include "vendor/linmath.h"
#include "vendor/stb_image.h"
#include "vendor/stb_ds.h"

#include "entities.h"
#include "sprite.h"
#include "camera.h"

void rect_to_uv_matrix(vec4 rect, mat4x4 matrix)
{
    mat4x4_identity(matrix);
    matrix[0][0] = rect[2];
    matrix[1][1] = rect[3];

    mat4x4_translate(matrix, rect[0], rect[1], 0);
}

typedef struct app_context_t
{
    uint32_t window_width;
    uint32_t window_height;
} app_context_t;

typedef struct gl_program_t
{
    GLuint program;
} gl_program_t;
void gl_program_free(gl_program_t *program)
{
    glDeleteProgram(program->program);
    free(program);
}

void startup(world_t *world)
{
    world_register_free(world, sprite_batch_t, (custom_free_fn)&sprite_batch_free);
    world_register_free(world, texture_t, (custom_free_fn)&texture_free);
    world_register_free(world, gl_program_t, (custom_free_fn)&gl_program_free);

    entity_t *random_stuff_entity = entity_new(world);
    {
        // TODO WT: Not a great idea storing an individual program on some random entity, create a program cache resource.
        gl_program_t *program = entity_create_component(random_stuff_entity, gl_program_t);

        // ? Should sprite_batch_t own this entirely?
        GLenum shader_types[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
        program->program = create_program("./shader/shader.glsl", shader_types, 2);

        sprite_batch_t *p_sprite_batch = world_create_resource(world, sprite_batch_t);
        // TODO WT: Refactor sprite batch instantiation to "set up" an existing ptr.
        sprite_batch_t sprite_batch = sprite_batch_new(program->program, 1000);
        memcpy_s(p_sprite_batch, sizeof(sprite_batch_t), &sprite_batch, sizeof(sprite_batch_t));
    }

    {
        entity_t *cam_entity = entity_new(world);
        camera_t *camera = entity_create_component(cam_entity, camera_t);
        camera->pos[0] = 0;
        camera->pos[1] = 1;

        app_context_t *app_context = world_get_resource(world, app_context_t);
        camera->aspect = (float)app_context->window_width / (float)app_context->window_height;

        camera->size = 10;

        float w = camera->size / 2, h = (camera->size / 2) / camera->aspect;
        mat4x4_ortho(camera->view_proj, -w, w, -h, h, -0, 100);
    }

    {
        const size_t num_sprites = 1000;

        const vec2 size = {8.0, 8.0};
        const vec2 min_max_scale = {0.1, 0.5};

        // TODO WT: Not a great idea storing an individual texture on some random entity, create a texture cache resource.
        texture_t *tex = entity_create_component(random_stuff_entity, texture_t);
        *tex = texture_new_load_entire("./images/fruit_banana.png");

        for (size_t i = 0; i < num_sprites; i++)
        {
            entity_t *e = entity_new(world);
            sprite_t *p_sprite = entity_create_component(e, sprite_t);
            float scale = min_max_scale[0] + ((float)rand() / RAND_MAX) * min_max_scale[1];
            sprite_t sprite = {
                .pos = {
                    ((float)rand() / RAND_MAX) * size[0] - size[0] / 2,
                    ((float)rand() / RAND_MAX) * size[1] - size[1] / 2,
                    0.0,
                },
                .scale = {scale, scale},
                .anchor = {0.5, 0.5},
                .color = {
                    ((float)rand() / RAND_MAX),
                    ((float)rand() / RAND_MAX),
                    ((float)rand() / RAND_MAX),
                    1.0,
                },
                .texture = tex,
            };
            memcpy_s(p_sprite, sizeof(sprite_t), &sprite, sizeof(sprite_t));
        }
    }
}

void tick(world_t *world)
{
    draw_sprites(world);
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

    world_t *world = world_new();

    app_context_t *app_context = world_create_resource(world, app_context_t);
    app_context->window_width = window_width;
    app_context->window_height = window_height;

    startup(world);

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

        tick(world);

        SDL_GL_SwapWindow(window);
    }

    // * These sould be freed by the custom free fns registered with world_t.
    // texture_free(&tex);
    // glDeleteProgram(program);

    // TODO WT: world_free is horribly inefficent and should be optimized. It happens at shutdown though so not massive.
    last_time = SDL_GetTicks64();
    world_free(world);
    this_time = SDL_GetTicks64();

    printf("world_free took %lldms", this_time - last_time);

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 1;
}