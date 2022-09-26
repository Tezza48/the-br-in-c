#include "texture.h"
#include "vendor/stb_image.h"
#include "engine/engine.h"

texture_t texture_new_load_entire(char *path)
{
    texture_t result = {0};
    result.name = path;
    mat4x4_identity(result.uv_matrix);

    GL_CALL(glCreateTextures(GL_TEXTURE_2D, 1, &result.texture));

    stbi_set_flip_vertically_on_load(1);
    int w, h, bpp;
    uint8_t *bytes = stbi_load(path, &w, &h, &bpp, STBI_rgb_alpha);

    const char *error = stbi_failure_reason();
    if (error)
    {
        printf("STB error:\t%s", error);
    }

    GL_CALL(glBindTexture(GL_TEXTURE_2D, result.texture));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes));

    stbi_image_free(bytes);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // GL_CALL(glTextureParameteri(result.texture, GL_TEXTURE_WRAP_R, GL_REPEAT));
    // GL_CALL(glTextureParameteri(result.texture, GL_TEXTURE_WRAP_S, GL_REPEAT));
    // GL_CALL(glTextureParameteri(result.texture, GL_TEXTURE_WRAP_T, GL_REPEAT));
    // GL_CALL(glTextureParameteri(result.texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST));
    // GL_CALL(glTextureParameteri(result.texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

    return result;
}

void texture_free(texture_t *self)
{
    glDeleteTextures(1, &self->texture);
    *self = (texture_t){0};
}