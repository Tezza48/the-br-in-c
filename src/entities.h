#pragma once
#include <stdint.h>

#include <SDL2/SDL.h>
#include <glad/glad.h>

#include "asset_cache.h"
#include "sprite_batch.h"
#include "sprite.h"
#include "transform.h"
#include "text.h"
#include "camera.h"

typedef struct entity_t entity_t;
typedef struct app_t
{

    SDL_Window *window;
    int window_width, window_height;
    char window_title[256];

    SDL_GLContext context;

    int32_t keyboard_state_length;
    const uint8_t *keyboard_state;
    uint8_t *last_keyboard_state;

    entity_t *root;
    entity_t **entities;
    asset_cache_t *asset_cache;
    sprite_batch_t *sprite_batch;

    uint8_t is_running;
} app_t;

typedef enum render_type_e
{
    RENDER_TYPE_NONE = 0,
    RENDER_TYPE_SPRITE,
    RENDER_TYPE_TEXT,
} render_type_e;

typedef struct entity_t
{
    entity_t *parent;
    entity_t **children;

    transform_t transform;

    struct
    {
        render_type_e render_type;

        union
        {
            sprite_t sprite;
            text_t text;
        };
    };

    uint8_t has_camera;
    camera_t camera;

} entity_t;

app_t *app_new();
void app_free(app_t *app);

entity_t *entity_new(app_t *app);
void entity_free(app_t *world, entity_t *app);

entity_t *set_parent(entity_t *entity, entity_t *parent);

#if UNIT_TEST
#include <assert.h>

static void entities_unit_tests_set_parent()
{
    app_t *app = calloc(1, sizeof(app_t));

    entity_t *parent = entity_new(app);

    entity_t *child = entity_new(app);

    entity_t *old_expect_0 = set_parent(child, parent);

    assert(parent->children[0] == child);
    assert(child->parent == parent);
    assert(old_expect_0 == 0);
}

static int entities_unit_tests(void)
{
    entities_unit_tests_set_parent();

    return 1;
}
#endif
