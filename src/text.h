#pragma once
#include "entities.h"
#include "texture.h"
#include "vendor/stb_truetype.h"
#include "vendor/stb_ds.h"

typedef struct baked_font_t
{
    texture_t texture;
    stbtt_bakedchar *char_data;
    float size;
} baked_font_t;

baked_font_t font_load(const char *path, float size);

void baked_font_cleanup(baked_font_t *font);

typedef struct text_t
{
    char *text;
    baked_font_t *font;
} text_t;

void draw_texts(world_t *world);