#include "entities.h"
#include <stdlib.h>
#include "vendor/stb_ds.h"
#include <assert.h>
#include "engine/engine.h"
#include "search.h"

app_t *app_new()
{
    app_t *app = calloc(1, sizeof(app_t));

    app->window_width = 1280;
    app->window_height = 720;
    strcpy_s(app->window_title, sizeof(app->window_title), "Hello, SDL2!");
    {
        SDL_Window *window = SDL_CreateWindow(
            app->window_title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            app->window_width,
            app->window_height,
            SDL_WINDOW_OPENGL);

        assert(window);
        app->window = window;
    }

    SDL_ShowWindow(app->window);
    SDL_GL_SetSwapInterval(0);

    app->keyboard_state = SDL_GetKeyboardState(&app->keyboard_state_length);
    app->last_keyboard_state = calloc(app->keyboard_state_length, sizeof(uint8_t));
    assert(app->keyboard_state && app->last_keyboard_state);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    app->context = SDL_GL_CreateContext(app->window);
    assert(app->context);

    if (!gladLoadGLLoader(&SDL_GL_GetProcAddress))
    {
        return 0;
    }

    app->entities = 0;
    app->root = entity_new(app);

    app->asset_cache = calloc(1, sizeof(asset_cache_t));

    // Malloc instead of calloc as we're going to memcpy to this address
    app->sprite_batch = malloc(sizeof(sprite_batch_t));
    assert(app->asset_cache && app->sprite_batch);
    {
        GLenum shader_types[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
        const char *batched_sprite_shader_src_path = "./shader/shader.glsl";
        shput(app->asset_cache->sh_programs, batched_sprite_shader_src_path, create_program(batched_sprite_shader_src_path, shader_types, 2));
        GLuint program = shget(app->asset_cache->sh_programs, batched_sprite_shader_src_path);

        sprite_batch_t temp_sprite_batch = sprite_batch_new(program, 1000);
        memcpy_s(app->sprite_batch, sizeof(sprite_batch_t), &temp_sprite_batch, sizeof(sprite_batch_t));
    }

    app->is_running = 1;

    return app;
}

void app_free(app_t *app)
{
    sprite_batch_free(app->sprite_batch);
    asset_cache_free(app->asset_cache);

    for (size_t i = 0; i < arrlen(app->entities); i++)
        entity_free(app, app->entities[i]);

    arrfree(app->entities);

    free(app->last_keyboard_state);

    SDL_GL_DeleteContext(app->context);
    SDL_DestroyWindow(app->window);
    SDL_Quit();

    free(app);
}

entity_t *entity_new(app_t *app)
{
    entity_t *entity = calloc(1, sizeof(entity_t));
    arrput(app->entities, entity);

    return entity;
}
void entity_free(app_t *app, entity_t *entity)
{
    size_t index = -1;
    for (size_t i = 0; i < arrlen(app->entities); i++)
    {
        if (app->entities[i] == entity)
        {
            index = i;
            break;
        }
    }

    assert(index != -1);

    free(entity);
    arrdelswap(app->entities, index);
}

int32_t compare_entities(const entity_t *a, const entity_t *b)
{
    return a == b;
}

entity_t *set_parent(entity_t *entity, entity_t *parent)
{
    // TODO WT: Setting the parent should move the entity to the correct sorted position in app->entities.
    entity_t *old_parent = entity->parent;

    if (old_parent)
    {
        uint32_t num_children = arrlenu(old_parent->children);
        entity_t **found = lfind(
            entity,
            old_parent->children,
            &num_children,
            sizeof(entity_t *),
            (int (*)(const void *, const void *))compare_entities);

        if (found)
        {
            size_t index = (found - old_parent->children) / sizeof(entity_t);
            arrdel(old_parent->children, index);
        }
    }

    arrput(parent->children, entity);
    entity->parent = parent;

    return old_parent;
}
