#include "sprite.h"
#include "engine/engine.h"
#include "vendor/stb_ds.h"

sprite_batch_t sprite_batch_new(GLuint program, size_t max_batch_size)
{
    sprite_batch_t result = {0};
    result.program = program;

    glCreateVertexArrays(1, &result.vertex_array);
    glBindVertexArray(result.vertex_array);

    glCreateBuffers(1, &result.vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, result.vertex_buffer);

    GL_CALL(glEnableVertexAttribArray(0));
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void *)0));
    GL_CALL(glEnableVertexAttribArray(1));
    GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void *)offsetof(vertex_t, uv)));
    GL_CALL(glEnableVertexAttribArray(2));
    GL_CALL(glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(vertex_t), (const void *)offsetof(vertex_t, color)));
    GL_CALL(glBindVertexArray(0));

    result.num_quads = 0;
    result.quads_vertices = calloc(max_batch_size, sizeof(sprite_quad_t));
    result.max_batch_size = max_batch_size;

    glBufferData(GL_ARRAY_BUFFER, max_batch_size * sizeof(sprite_quad_t), result.quads_vertices, GL_DYNAMIC_DRAW);

    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CALL(glCreateSamplers(1, &result.texture_sampler));
    // GL_CALL(glSamplerParameteri(result.texture_sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    // GL_CALL(glSamplerParameteri(result.texture_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    // GL_CALL(glSamplerParameteri(result.texture_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    // GL_CALL(glSamplerParameterf(result.texture_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
    // GL_CALL(glSamplerParameterf(result.texture_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    glObjectLabel(GL_BUFFER, result.vertex_buffer, -1, "VertexBuffer(sprite_batch_t)");

    return result;
}

void sprite_batch_free(sprite_batch_t *self)
{
    free(self->quads_vertices);
    glDeleteBuffers(1, &self->vertex_buffer);
    glDeleteVertexArrays(1, &self->vertex_array);

    *self = (sprite_batch_t){0};
}

void sprite_batch_flush(sprite_batch_t *self)
{
    if (self->num_quads == 0)
        return;

    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, self->vertex_buffer));
    glBufferSubData(GL_ARRAY_BUFFER, 0, self->num_quads * sizeof(sprite_quad_t), self->quads_vertices);
    // GL_CALL(glBufferData(GL_ARRAY_BUFFER, self->num_quads * sizeof(sprite_quad_t), self->quads_vertices, GL_DYNAMIC_DRAW));
    // glNamedBufferData(self->vertex_buffer, self->num_quads * sizeof(sprite_quad_t), self->quads_vertices, GL_STATIC_DRAW);

    GL_CALL(glActiveTexture(GL_TEXTURE0));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, self->current_texture_id));
    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL_CALL(glUseProgram(self->program));
    GL_CALL(glBindVertexArray(self->vertex_array));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, self->vertex_buffer));
    GL_CALL(glBindSampler(0, self->texture_sampler));
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, self->num_quads * 6));
    self->num_quads = 0;

    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

size_t sprite_batch_draw(sprite_batch_t *self, sprite_t *sprite)
{
    if (self->current_texture_id != sprite->texture->texture)
    {
        sprite_batch_flush(self);
    }

    self->current_texture_id = sprite->texture->texture;

    float x_anchor = sprite->anchor[0] * sprite->scale[0];
    float y_anchor = sprite->anchor[1] * sprite->scale[1];
    sprite_quad_t vertices = {
        {
            {
                sprite->pos[0] - x_anchor,
                sprite->pos[1] - y_anchor,
                sprite->pos[2],
            },
            {0.0, 0.0},
            {
                sprite->color[0],
                sprite->color[1],
                sprite->color[2],
                sprite->color[3],
            },
        },
        {
            {
                sprite->pos[0] - x_anchor,
                sprite->pos[1] + sprite->scale[1] - y_anchor,
                sprite->pos[2],
            },
            {0.0, 1.0},
            {
                sprite->color[0],
                sprite->color[1],
                sprite->color[2],
                sprite->color[3],
            },
        },
        {
            {
                sprite->pos[0] + sprite->scale[0] - x_anchor,
                sprite->pos[1] + sprite->scale[1] - y_anchor,
                sprite->pos[2],
            },
            {1.0, 1.0},
            {
                sprite->color[0],
                sprite->color[1],
                sprite->color[2],
                sprite->color[3],
            },
        },
        {
            {
                sprite->pos[0] - x_anchor,
                sprite->pos[1] - y_anchor,
                sprite->pos[2],
            },
            {0.0, 0.0},
            {
                sprite->color[0],
                sprite->color[1],
                sprite->color[2],
                sprite->color[3],
            },
        },
        {
            {
                sprite->pos[0] + sprite->scale[0] - x_anchor,
                sprite->pos[1] + sprite->scale[1] - y_anchor,
                sprite->pos[2],
            },
            {1.0, 1.0},
            {
                sprite->color[0],
                sprite->color[1],
                sprite->color[2],
                sprite->color[3],
            },
        },
        {
            {
                sprite->pos[0] + sprite->scale[0] - x_anchor,
                sprite->pos[1] - y_anchor,
                sprite->pos[2],
            },
            {1.0, 0.0},
            {
                sprite->color[0],
                sprite->color[1],
                sprite->color[2],
                sprite->color[3],
            },
        },
    };

    memcpy_s(&self->quads_vertices[self->num_quads++], sizeof(sprite_quad_t), vertices, sizeof(sprite_quad_t));

    if (self->num_quads >= self->max_batch_size)
    {
        sprite_batch_flush(self);
        return 1;
    }

    return 0;
}

#include "camera.h"

void draw_sprites(world_t *world)
{
    sprite_batch_t *sprite_batch = world_get_resource(world, sprite_batch_t);
    GL_CALL(glUseProgram(sprite_batch->program));

    camera_t *camera;
    entity_t *arr_entities = world_get_entities(world);
    for (size_t i = 0; i < arrlen(arr_entities); i++)
    {
        camera = entity_get_component(&arr_entities[i], camera_t);
        if (camera)
            break;
    }

    assert(camera);

    glUniformMatrix4fv(glGetUniformLocation(sprite_batch->program, "mat_view_proj"), 1, GL_FALSE, camera->view_proj[0]);

    glDisable(GL_DEPTH_TEST);

    // TODO WT: either sort by depth OR use parent hierarchy to draw back to front.
    size_t did_batcher_flush = 0;
    for (size_t i = 0; i < arrlen(world->arr_entities); i++)
    {
        entity_t *entity = &arr_entities[i];
        sprite_t *sprite = entity_get_component(entity, sprite_t);
        if (!sprite)
            continue;

        did_batcher_flush = sprite_batch_draw(sprite_batch, sprite);
    }

    if (!did_batcher_flush)
    {
        sprite_batch_flush(sprite_batch);
    }
}