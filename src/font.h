#pragma once
#include <glad/glad.h>
#include "vendor/stb_truetype.h"

typedef struct rendered_font_data_t
{
    GLuint texture;
    stbtt_bakedchar *char_data;
    uint32_t tex_size;
} rendered_font_data_t;

typedef struct rendered_font_entry_t
{
    float key;
    rendered_font_data_t value;
} rendered_font_entry_t;

typedef struct font_t
{
    uint8_t *buffer;
    size_t buffer_length;
    rendered_font_entry_t *hm_rendered;
} font_t;

font_t font_load(const char *path);
void font_cleanup(font_t *font);

const rendered_font_data_t *const get_font_render_data(font_t *font, float size);