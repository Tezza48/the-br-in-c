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

#include "vendor/stb_truetype.h"

#include "asset_cache.h"

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
    // Tell the ecs to use a particular proc to free certain types.
    world_register_free(world, sprite_batch_t, (custom_free_fn)sprite_batch_free);
    world_register_free(world, texture_t, (custom_free_fn)texture_free);
    world_register_free(world, gl_program_t, (custom_free_fn)gl_program_free);
    world_register_free(world, asset_cache_t, (custom_free_fn)asset_cache_free);

    asset_cache_t *asset_cache = world_create_resource(world, asset_cache_t);

    {
        // ? Should sprite_batch_t own this entirely?
        GLenum shader_types[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
        const char *batched_sprite_shader_src_path = "./shader/shader.glsl";
        shput(asset_cache->sh_programs, batched_sprite_shader_src_path, create_program(batched_sprite_shader_src_path, shader_types, 2));
        GLuint *program = &shget(asset_cache->sh_programs, batched_sprite_shader_src_path);

        sprite_batch_t *p_sprite_batch = world_create_resource(world, sprite_batch_t);
        // TODO WT: Refactor sprite batch instantiation to "set up" an existing ptr.
        sprite_batch_t sprite_batch = sprite_batch_new(*program, 1000);
        memcpy_s(p_sprite_batch, sizeof(sprite_batch_t), &sprite_batch, sizeof(sprite_batch_t));
    }

    {
        entity_t *cam_entity = entity_new(world);
        camera_t *camera = entity_create_component(cam_entity, camera_t);

        vec2 pos = {0.0f, 0.0f};
        memcpy_s(camera->pos, sizeof(vec2), pos, sizeof(vec2));

        app_context_t *app_context = world_get_resource(world, app_context_t);

        camera->aspect = (float)app_context->window_width / (float)app_context->window_height;
        camera->size = app_context->window_width;

        float hw = app_context->window_width / 2, hh = app_context->window_height / 2;
        mat4x4_ortho(camera->view_proj, -hw, hw, -hh, hh, 0, 100);
    }

    {
        app_context_t *app_context = world_get_resource(world, app_context_t);

        const size_t num_sprites = 1000;

        const vec2 size = {app_context->window_width, app_context->window_height};
        const vec2 min_max_scale = {10, 100};

        // TODO WT: Not a great idea storing an individual texture on some random entity, create a texture cache resource.
        const char *banana_texture_path = "./images/fruit_banana.png";
        shput(asset_cache->sh_textures, banana_texture_path, texture_new_load_entire(banana_texture_path));
        texture_t *tex = &shget(asset_cache->sh_textures, banana_texture_path);

        for (size_t i = 0; i < num_sprites; i++)
        {
            entity_t *e = entity_new(world);
            sprite_t *p_sprite = entity_create_component(e, sprite_t);
            float scale = min_max_scale[0] + ((float)rand() / RAND_MAX) * min_max_scale[1];
            sprite_t sprite = {
                .pos = {
                    ((float)rand() / RAND_MAX) * size[0] - size[0] / 2,
                    ((float)rand() / RAND_MAX) * size[1] - size[1] / 2,
                    -1.0,
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
    {
        const char *constan_font_path = "./font/CONSTAN.TTF";
        shput(asset_cache->sh_baked_fonts_32px, constan_font_path, font_load(constan_font_path, 32.0));

        baked_font_t *constan_32 = &shget(asset_cache->sh_baked_fonts_32px, constan_font_path);

        sprite_t *debug_entire_font_texture_sprite = entity_create_component(entity_new(world), sprite_t);
        vec3 pos = {0.0f, 0.0f, 0.0f};
        vec2 scale = {4.0f, 4.0f};
        vec2 anchor = {0.5f, 0.5f};
        vec4 color = {1.0, 1.0, 1.0, 1.0};

        memcpy_s(debug_entire_font_texture_sprite->pos, sizeof(vec3), pos, sizeof(vec3));
        memcpy_s(debug_entire_font_texture_sprite->scale, sizeof(vec2), scale, sizeof(vec2));
        memcpy_s(debug_entire_font_texture_sprite->anchor, sizeof(vec2), anchor, sizeof(vec2));
        memcpy_s(debug_entire_font_texture_sprite->color, sizeof(vec4), color, sizeof(vec4));

        debug_entire_font_texture_sprite->texture = &constan_32->texture;
    }
}

#include <SDL2/SDL_opengl.h>

void tick(world_t *world)
{
    draw_sprites(world);

    asset_cache_t *assets = world_get_resource(world, asset_cache_t);
    baked_font_t *font = &shget(assets->sh_baked_fonts_32px, "./font/CONSTAN.TTF");
    GLuint ftex = font->texture.texture;
    const char *text = "Hello, World!\0";
    stbtt_bakedchar *cdata = font->char_data;

    float x = 0, y = 0;

    // void (*glBegin)() = SDL_GL_GetProcAddress("glBegin");
    // void (*glTexCoord2f)() = SDL_GL_GetProcAddress("glTexCoord2f");
    // void (*glVertex2f)() = SDL_GL_GetProcAddress("glVertex2f");
    // void (*glEnd)() = SDL_GL_GetProcAddress("glEnd");

    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    // GL_CALL(glEnable(GL_TEXTURE_2D));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, ftex));

    sprite_batch_t *sprite_batch = world_get_resource(world, sprite_batch_t);

    GL_CALL(glUseProgram(sprite_batch->program));

    camera_t *camera;
    entity_t *arr_entities = world_get_entities(world);
    for (size_t i = 0; i < arrlen(arr_entities); i++)
    {
        camera = entity_get_component(&arr_entities[i], camera_t);
        if (camera)
            break;
    }

    assert(camera);

    glUniformMatrix4fv(glGetUniformLocation(sprite_batch->program, "mat_view_proj"), 1, GL_FALSE, camera->view_proj[0]);

    glDisable(GL_DEPTH_TEST);

    sprite_batch->current_texture_id = ftex;

    int did_flush = 0;

    // GL_CALL(glBegin(GL_TRIANGLES));
    while (*text)
    {
        if (*text >= 32 && *text < 128)
        {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512, 512, *text - 32, &x, &y, &q, 1); // 1=opengl & d3d10+,0=d3d9

            sprite_quad_t vertices = {
                {.uv = {q.s0, q.t0}, .pos = {q.x0, -q.y0, 0.0}, .color = {1.0, 1.0, 1.0, 1.0}},
                {.uv = {q.s1, q.t0}, .pos = {q.x1, -q.y0, 0.0}, .color = {1.0, 1.0, 1.0, 1.0}},
                {.uv = {q.s1, q.t1}, .pos = {q.x1, -q.y1, 0.0}, .color = {1.0, 1.0, 1.0, 1.0}},
                {.uv = {q.s0, q.t0}, .pos = {q.x0, -q.y0, 0.0}, .color = {1.0, 1.0, 1.0, 1.0}},
                {.uv = {q.s1, q.t1}, .pos = {q.x1, -q.y1, 0.0}, .color = {1.0, 1.0, 1.0, 1.0}},
                {.uv = {q.s0, q.t1}, .pos = {q.x0, -q.y1, 0.0}, .color = {1.0, 1.0, 1.0, 1.0}},
            };

            memcpy_s(&sprite_batch->quads_vertices[sprite_batch->num_quads++], sizeof(sprite_quad_t), vertices, sizeof(sprite_quad_t));

            // sprite_batch_flush(sprite_batch);
            if (sprite_batch->num_quads >= sprite_batch->max_batch_size)
            {
                sprite_batch_flush(sprite_batch);

                did_flush = 1;
            }
            else
            {
                did_flush = 0;
            }

            // GL_CALL(glTexCoord2f(q.s0, q.t0));
            // GL_CALL(glVertex2f(q.x0, q.y0));

            // GL_CALL(glTexCoord2f(q.s1, q.t0));
            // GL_CALL(glVertex2f(q.x1, q.y0));

            // GL_CALL(glTexCoord2f(q.s1, q.t1));
            // GL_CALL(glVertex2f(q.x1, q.y1));

            // GL_CALL(glTexCoord2f(q.s0, q.t0));
            // GL_CALL(glVertex2f(q.x0, q.y0));

            // GL_CALL(glTexCoord2f(q.s1, q.t1));
            // GL_CALL(glVertex2f(q.x1, q.y1));

            // GL_CALL(glTexCoord2f(q.s0, q.t1));
            // GL_CALL(glVertex2f(q.x0, q.y1));
        }
        ++text;
    }

    if (!did_flush)
    {
        sprite_batch_flush(sprite_batch);
    }
}

lib_start_result lib_start()
{
    float window_width = 1280, window_height = 720;
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
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
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
        GL_CALL(glClearDepthf(1));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

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