#include "asset_cache.h"
#include <stdlib.h>
#include "vendor/stb_ds.h"

void asset_cache_delete_program(GLuint *program)
{
    glDeleteProgram(*program);
}

void asset_cache_free(asset_cache_t *p_cache)
{
#define X(kv_struct, type, var, cleanup)             \
    for (size_t i = 0; i < shlen(p_cache->var); i++) \
    {                                                \
        cleanup(&p_cache->var[i].value);             \
    }                                                \
    shfree(p_cache->var);

    FOR_EACH_CACHE_TYPE

#undef X

    free(p_cache);
}