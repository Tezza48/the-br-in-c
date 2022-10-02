#include "font.h"
#include <stdio.h>
#include "vendor/stb_ds.h"
#include "engine/engine.h"

font_t font_load(const char *path)
{
    uint8_t *bytes;
    FILE *file = fopen(path, "r");

    fseek(file, 0, SEEK_END);
    int64_t num_bytes = ftell(file);

    rewind(file);

    bytes = calloc(num_bytes, sizeof(uint8_t));

    fread(bytes, sizeof(uint8_t), num_bytes, file);

    fclose(file);

    return (font_t){bytes, num_bytes, 0};
}

/// @brief Get the rendered font data for the specified size, Bakes a new texture if that size isn't already baked.
/// @param font
/// @param size
/// @return
const rendered_font_data_t *const get_font_render_data(font_t *font, float size)
{
    if (hmgeti(font->hm_rendered, size) != -1)
    {
        return &hmget(font->hm_rendered, size);
    }

    const int32_t tex_size = size * 8;

    uint8_t bitmap[tex_size * tex_size];
    stbtt_bakedchar *cdata = calloc(96, sizeof(stbtt_bakedchar));

    stbtt_BakeFontBitmap(font->buffer, 0, size, bitmap, tex_size, tex_size, 32, 96, cdata);

    uint8_t *rgba_bitmap = calloc(tex_size * tex_size * 4, sizeof(uint8_t));

    for (size_t i = 0; i < tex_size * tex_size; i++)
    {
        size_t start_index = i * 4;
        rgba_bitmap[start_index + 0] = bitmap[i];
        rgba_bitmap[start_index + 1] = bitmap[i];
        rgba_bitmap[start_index + 2] = bitmap[i];
        rgba_bitmap[start_index + 3] = bitmap[i];
    }

    rendered_font_data_t result = {0};
    result.tex_size = tex_size;
    result.char_data = cdata;
    GL_CALL(glGenTextures(1, &result.texture));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, result.texture));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_size, tex_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_bitmap));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

    free(rgba_bitmap);

    hmput(font->hm_rendered, size, result);
    return &hmget(font->hm_rendered, size);
}

void font_cleanup(font_t *font)
{
    for (size_t i = 0; i < hmlen(font->hm_rendered); i++)
    {
        rendered_font_data_t *data = &font->hm_rendered[i].value;
        glDeleteTextures(1, &data->texture);
        free(data->char_data);
    }

    free(font->buffer);
    font->buffer_length = 0;
}
