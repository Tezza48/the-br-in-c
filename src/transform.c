#include "transform.h"
#include "vendor/stb_ds.h"
#include "entities.h"
#include "search.h"

void set_pos(transform_t *transform, vec3 pos)
{
    memcpy_s(transform->pos, sizeof(vec3), pos, sizeof(vec3));
    transform->is_dirty = 1;
}

void set_scale(transform_t *transform, vec2 scale)
{
    memcpy_s(transform->scale, sizeof(vec2), scale, sizeof(vec2));
    transform->is_dirty = 1;
}

void update_local(transform_t *transform)
{
    if (!transform->is_dirty)
        return;

    mat4x4_identity(transform->local_matrix);
    mat4x4_translate(transform->local_matrix, transform->pos[0], transform->pos[1], transform->pos[2]);
    transform->local_matrix[0][0] = transform->scale[0];
    transform->local_matrix[1][1] = transform->scale[1];

    transform->is_dirty = 0;
}

// void sort_transforms(app_t *app)
// {
//     for (size_t i = 0; i < arrlen(app->entities); i++)
//     {
//         uint32_t curr_i = lfind(app->entities[i], )
//         app->entities
//     }
// }

void update_local_system(app_t *app)
{
    for (size_t i = 0; i < arrlen(app->entities); i++)
    {
        update_local(&app->entities[i]->transform);
    }
}

void traverse_and_push_children(entity_t *node, entity_t **dest)
{
    arrput(dest, node);

    for (size_t i = 0; i < arrlen(node->children); i++)
    {
        traverse_and_push_children(node->children[i], dest);
    }
}

void update_global_system(app_t *app)
{
    // Make all orphaned entities a direct child of root.
    for (size_t i = 0; i < arrlen(app->entities); i++)
    {
        if (!app->entities[i]->parent && app->entities[i] != app->root)
            arrput(app->root->children, app->entities[i]);
    }

    entity_t **arr_sorted = 0;
    arrsetcap(arr_sorted, arrlen(app->entities));

    // Traverse the tree and push all children into arr_sorted.
    traverse_and_push_children(app->root, arr_sorted);

    arrfree(app->entities);

    // Entities list is now a sorted tree.
    app->entities = arr_sorted;
}