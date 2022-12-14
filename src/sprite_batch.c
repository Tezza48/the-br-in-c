#include "sprite_batch.h"
#include "engine/engine.h"
#include "sprite.h"
#include "camera.h"
#include "text.h"
#include "font.h"
#include <stdlib.h>
#include "vendor/stb_ds.h"
#include <assert.h>
#include "entities.h"

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

uint8_t submit_sprite(sprite_batch_t *self, sprite_t *sprite, transform_t *transform)
{
    float x_anchor = sprite->anchor[0] * transform->scale[0];
    float y_anchor = sprite->anchor[1] * transform->scale[1];
    sprite_quad_t vertices = {
        {
            {transform->pos[0] - x_anchor, transform->pos[1] - y_anchor, 0.0 /*-transform->pos[2]*/},
            {0.0, 0.0},
            {sprite->color[0], sprite->color[1], sprite->color[2], sprite->color[3]},
        },
        {
            {transform->pos[0] - x_anchor, transform->pos[1] + transform->scale[1] - y_anchor, 0.0 /*-transform->pos[2]*/},
            {0.0, 1.0},
            {sprite->color[0], sprite->color[1], sprite->color[2], sprite->color[3]},
        },
        {
            {transform->pos[0] + transform->scale[0] - x_anchor, transform->pos[1] + transform->scale[1] - y_anchor, 0.0 /*-transform->pos[2]*/},
            {1.0, 1.0},
            {sprite->color[0], sprite->color[1], sprite->color[2], sprite->color[3]},
        },
        {
            {transform->pos[0] - x_anchor, transform->pos[1] - y_anchor, 0.0 /*-transform->pos[2]*/},
            {0.0, 0.0},
            {sprite->color[0], sprite->color[1], sprite->color[2], sprite->color[3]},
        },
        {
            {transform->pos[0] + transform->scale[0] - x_anchor, transform->pos[1] + transform->scale[1] - y_anchor, 0.0 /*-transform->pos[2]*/},
            {1.0, 1.0},
            {sprite->color[0], sprite->color[1], sprite->color[2], sprite->color[3]},
        },
        {
            {transform->pos[0] + transform->scale[0] - x_anchor, transform->pos[1] - y_anchor, 0.0 /*-transform->pos[2]*/},
            {1.0, 0.0},
            {sprite->color[0], sprite->color[1], sprite->color[2], sprite->color[3]},
        },
    };

    return sprite_batch_submit_quad(self, vertices, sprite->texture->texture);
}

uint8_t submit_text(sprite_batch_t *batch, text_t *text, transform_t *transform)
{
    const rendered_font_data_t *const data = get_font_render_data(text->font, (float)text->font_size);
    stbtt_bakedchar *cdata = data->char_data;

    float x = transform->pos[0], y = transform->pos[1];

    char *t = text->text;

    uint8_t did_flush;

    while (*t)
    {
        if (*t >= 32 && *t < 128)
        {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, data->tex_size, data->tex_size, *t - 32, &x, &y, &q, 1); // 1=opengl & d3d10+,0=d3d9

            sprite_quad_t vertices = {
                {.uv = {q.s0, q.t0}, .pos = {q.x0, -q.y0, 0.0 /*-transform->pos[2]*/}, .color = {1.0, 1.0, 1.0, 1.0}},
                {.uv = {q.s1, q.t0}, .pos = {q.x1, -q.y0, 0.0 /*-transform->pos[2]*/}, .color = {1.0, 1.0, 1.0, 1.0}},
                {.uv = {q.s1, q.t1}, .pos = {q.x1, -q.y1, 0.0 /*-transform->pos[2]*/}, .color = {1.0, 1.0, 1.0, 1.0}},
                {.uv = {q.s0, q.t0}, .pos = {q.x0, -q.y0, 0.0 /*-transform->pos[2]*/}, .color = {1.0, 1.0, 1.0, 1.0}},
                {.uv = {q.s1, q.t1}, .pos = {q.x1, -q.y1, 0.0 /*-transform->pos[2]*/}, .color = {1.0, 1.0, 1.0, 1.0}},
                {.uv = {q.s0, q.t1}, .pos = {q.x0, -q.y1, 0.0 /*-transform->pos[2]*/}, .color = {1.0, 1.0, 1.0, 1.0}},
            };

            did_flush = sprite_batch_submit_quad(batch, vertices, data->texture);
        }

        ++t;
    }

    return did_flush;
}

void sprite_batch_render_system(app_t *app)
{
    sprite_batch_t *sprite_batch = app->sprite_batch;

    GL_CALL(glUseProgram(sprite_batch->program));

    camera_t *camera;
    entity_t **arr_entities = app->entities;
    // arr_entities is always sorted by the transform hierarchy, interate backwards to draw back to front.
    for (size_t i = arrlen(arr_entities) - 1; i >= 0; i--)
    {
        if (arr_entities[i]->has_camera)
        {
            camera = &arr_entities[i]->camera;
            break;
        }
    }

    assert(camera);

    glUniformMatrix4fv(glGetUniformLocation(sprite_batch->program, "mat_view_proj"), 1, GL_FALSE, camera->view_proj[0]);

    glDisable(GL_DEPTH_TEST);

    // TODO WT: either sort by depth OR use parent hierarchy to draw back to front.
    size_t did_batcher_flush = 0;
    for (size_t i = 0; i < arrlen(arr_entities); i++)
    {
        entity_t *entity = arr_entities[i];

        switch (entity->render_type)
        {
        case RENDER_TYPE_SPRITE:
        {
            did_batcher_flush = submit_sprite(sprite_batch, &entity->sprite, &entity->transform);
            break;
        }
        case RENDER_TYPE_TEXT:
        {
            did_batcher_flush = submit_text(sprite_batch, &entity->text, &entity->transform);
            break;
        }
        case RENDER_TYPE_NONE:
        default:
        {
            break;
        }
        }
    }

    if (!did_batcher_flush)
    {
        sprite_batch_flush(sprite_batch);
    }
}

uint8_t sprite_batch_submit_quad(sprite_batch_t *batch, sprite_quad_t quad, GLuint texture)
{
    if (texture != batch->current_texture_id)
    {
        sprite_batch_flush(batch);
    }

    batch->current_texture_id = texture;

    memcpy_s(&batch->quads_vertices[batch->num_quads++], sizeof(sprite_quad_t), quad, sizeof(sprite_quad_t));

    if (batch->num_quads >= batch->max_batch_size)
    {
        sprite_batch_flush(batch);
        return 1;
    }

    return 0;
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