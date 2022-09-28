#include "text.h"
#include "stdio.h"
#include "asset_cache.h"
#include "engine/engine.h"

baked_font_t font_load(const char *path, float size)
{
    uint8_t *bytes;
    FILE *file = fopen(path, "r");

    fseek(file, 0, SEEK_END);
    int64_t num_bytes = ftell(file);

    rewind(file);

    bytes = calloc(num_bytes, sizeof(uint8_t));

    fread(bytes, sizeof(uint8_t), num_bytes, file);

    fclose(file);

    uint32_t tex_width = 512;
    uint8_t bitmap[tex_width * tex_width];
    stbtt_bakedchar *cdata = calloc(96, sizeof(stbtt_bakedchar));

    stbtt_BakeFontBitmap(bytes, 0, size, bitmap, tex_width, tex_width, 32, 96, cdata);

    // uint8_t y_flip[tex_width * tex_width];
    // for (size_t y = 0; y < tex_width; y++)
    // {
    //     for (size_t x = 0; x < tex_width; x++)
    //     {
    //         y_flip[(tex_width - y) * tex_width + x] = bitmap[y * tex_width + x];
    //     }
    // }

    uint8_t *rgba_bitmap = calloc(tex_width * tex_width * 4, sizeof(uint8_t));

    for (size_t i = 0; i < tex_width * tex_width; i++)
    {
        size_t start_index = i * 4;
        rgba_bitmap[start_index + 0] = bitmap[i]; // y_flip[i];
        rgba_bitmap[start_index + 1] = bitmap[i]; // y_flip[i];
        rgba_bitmap[start_index + 2] = bitmap[i]; // y_flip[i];
        rgba_bitmap[start_index + 3] = bitmap[i]; // y_flip[i];
    }

    baked_font_t result = {0};
    result.char_data = cdata;

    result.texture.name = "FONT_TEXTURE";
    GL_CALL(glGenTextures(1, &result.texture.texture));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, result.texture.texture));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_width, tex_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_bitmap));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
    glObjectLabel(GL_TEXTURE, result.texture.texture, -1, result.texture.name);
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

    free(rgba_bitmap);

    mat4x4_identity(result.texture.uv_matrix);

    result.size = size;

    return result;
}

void baked_font_cleanup(baked_font_t *font)
{
    texture_cleanup(&font->texture);
    free(font->char_data);
}
