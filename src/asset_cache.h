#pragma once
#include <glad/glad.h>
#include "texture.h"
#include "text.h"

// TODO WT: cache font textures/implementation within the font code.
// ! If you want fonts at a size other than 32px, touch luck

#define FOR_EACH_CACHE_TYPE                                                   \
    X(texture_cache_entry_t, texture_t, sh_textures, texture_cleanup)         \
    X(program_cache_entry_t, GLuint, sh_programs, asset_cache_delete_program) \
    X(baked_font_32px_entry_t, baked_font_t, sh_baked_fonts_32px, baked_font_cleanup)

#define X(kv_struct, type, ...) \
    typedef struct kv_struct    \
    {                           \
        const char *key;        \
        type value;             \
    } kv_struct;
FOR_EACH_CACHE_TYPE
#undef X

typedef struct font_size_to_baked_font_entry_t
{
    float key;
    baked_font_t value;
} font_size_to_baked_font_entry_t;

typedef struct asset_cache_t
{
#define X(kv_struct, type, var, ...) kv_struct *var;
    FOR_EACH_CACHE_TYPE
#undef X
} asset_cache_t;

void asset_cache_free(asset_cache_t *p_cache);

/// @brief Delete the opengl program stored at this pointer, only for use with asset_cache_t.
/// @param program Ptr to an OpenGL program.
void asset_cache_delete_program(GLuint *program);