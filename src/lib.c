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
#include "sprite_batch.h"
#include "font.h"

#include "vendor/stb_truetype.h"

#include "asset_cache.h"
#include "transform.h"

// void rect_to_uv_matrix(vec4 rect, mat4x4 matrix)
// {
//     mat4x4_identity(matrix);
//     matrix[0][0] = rect[2];
//     matrix[1][1] = rect[3];

//     mat4x4_translate(matrix, rect[0], rect[1], 0);
// }

void startup(app_t *app)
{
    asset_cache_t *asset_cache = app->asset_cache;
    {
        entity_t *cam_entity = entity_new(app);
        cam_entity->has_camera = 1;
        camera_t *camera = &cam_entity->camera;

        set_parent(cam_entity, app->root);

        // TODO WT: Consolidate all the individual components with position/scale/etc...
        vec2 pos = {0.0f, 0.0f};
        memcpy_s(camera->pos, sizeof(vec2), pos, sizeof(vec2));

        camera->aspect = (float)app->window_width / (float)app->window_height;
        camera->size = app->window_width;

        float hw = app->window_width / 2, hh = app->window_height / 2;
        mat4x4_ortho(camera->view_proj, -hw, hw, -hh, hh, -1, 100);
    }

    {
        const size_t num_sprites = 500;

        const vec2 size = {app->window_width, app->window_height};
        const vec2 min_max_scale = {10, 100};

        const char *banana_texture_path = "./images/fruit_banana.png";
        shput(asset_cache->sh_textures, banana_texture_path, texture_new_load_entire(banana_texture_path));
        texture_t *tex = &shget(asset_cache->sh_textures, banana_texture_path);

        for (size_t i = 0; i < num_sprites; i++)
        {
            entity_t *e = entity_new(app);
            set_parent(e, app->root);

            e->render_type = RENDER_TYPE_SPRITE;
            float scale = min_max_scale[0] + ((float)rand() / RAND_MAX) * min_max_scale[1];
            vec3 pos = {
                ((float)rand() / RAND_MAX) * size[0] - size[0] / 2,
                ((float)rand() / RAND_MAX) * size[1] - size[1] / 2,
                1.0,
            };
            memcpy_s(e->transform.pos, sizeof(vec3), pos, sizeof(vec3));
            vec2 scaleVec = {scale, scale};
            memcpy_s(e->transform.scale, sizeof(vec2), scaleVec, sizeof(vec2));

            vec2 anchor = {0.5, 0.5};
            memcpy_s(e->sprite.anchor, sizeof(vec2), anchor, sizeof(vec2));
            vec4 color = {
                0xff / 255.0, // ((float)rand() / RAND_MAX),
                0,            // ((float)rand() / RAND_MAX),
                0xff / 255.0, // ((float)rand() / RAND_MAX),
                1.0,
            };
            memcpy_s(e->sprite.color, sizeof(vec4), color, sizeof(vec4));
            e->sprite.texture = tex;
        }
    }
    {
        const char *constan_font_path = "./font/CONSTAN.TTF";
        shput(asset_cache->sh_fonts, constan_font_path, font_load(constan_font_path));

        font_t *constan = &shget(asset_cache->sh_fonts, constan_font_path);

        entity_t *e = entity_new(app);
        e->render_type = RENDER_TYPE_TEXT;
        text_t *hello_text = &e->text;
        set_parent(e, app->root);

        hello_text->font = constan;
        hello_text->font_size = 30;
        vec2 scale = {1.0, 1.0};
        memcpy_s(e->transform.scale, sizeof(vec2), scale, sizeof(vec2));

        hello_text->text = "Hello, World!";
    }

    {
        const size_t num_sprites = 500;

        const vec2 size = {app->window_width, app->window_height};
        const vec2 min_max_scale = {10, 100};

        char *banana_texture_path = "./images/fruit_banana.png";
        texture_t *tex = &shget(asset_cache->sh_textures, banana_texture_path);

        for (size_t i = 0; i < num_sprites; i++)
        {
            entity_t *e = entity_new(app);
            set_parent(e, app->root);
            e->render_type = RENDER_TYPE_SPRITE;

            float scale = min_max_scale[0] + ((float)rand() / RAND_MAX) * min_max_scale[1];
            vec3 pos = {
                ((float)rand() / RAND_MAX) * size[0] - size[0] / 2,
                ((float)rand() / RAND_MAX) * size[1] - size[1] / 2,
                1.0,
            };
            memcpy_s(e->transform.pos, sizeof(vec3), pos, sizeof(vec3));
            vec2 scaleVec = {scale, scale};
            memcpy_s(e->transform.scale, sizeof(vec2), scaleVec, sizeof(vec2));

            vec2 anchor = {0.5, 0.5};
            memcpy_s(e->sprite.anchor, sizeof(vec2), anchor, sizeof(vec2));
            vec4 color = {
                1.0, //((float)rand() / RAND_MAX),
                1.0, //((float)rand() / RAND_MAX),
                1.0, //((float)rand() / RAND_MAX),
                0.8,
            };
            memcpy_s(e->sprite.color, sizeof(vec4), color, sizeof(vec4));
            e->sprite.texture = tex;
        }
    }
}

void tick(app_t *app)
{

    sprite_batch_render_system(app);
}

lib_start_result lib_start()
{
    app_t *app = app_new();

    startup(app);

    uint64_t last_time;
    uint64_t this_time = SDL_GetTicks64();

    uint64_t elapsed_times[60];
    size_t curr_frame_time = 0;
    while (app->is_running)
    {
        memcpy_s(
            app->last_keyboard_state,
            sizeof(uint8_t) * app->keyboard_state_length,
            app->keyboard_state,
            sizeof(uint8_t) * app->keyboard_state_length);

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

        sprintf(app->window_title, "Hello, Sprite Batching | %.1f FPS", 1.0 / delta_seconds);
        SDL_SetWindowTitle(app->window, app->window_title);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                app->is_running = 0;
                break;
            default:
                break;
            }
        }

        if (app->keyboard_state[SDL_SCANCODE_ESCAPE])
            app->is_running = 0;

        GL_CALL(glClearColor(0.5, 0.5, 0.5, 1.0));
        GL_CALL(glClearDepthf(1));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        tick(app);

        SDL_GL_SwapWindow(app->window);
    }

    app_free(app);

    return 1;
}