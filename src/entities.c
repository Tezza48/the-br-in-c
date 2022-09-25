#include "entities.h"
#include "vendor/stb_ds.h"

void free_all_components(world_t *world, entity_t *entity);

world_t *world_new()
{
    world_t *val = (world_t *)calloc(1, sizeof(world_t));
    assert(val);
    return val;
}
void world_free(world_t *world)
{
    for (size_t i = 0; i < arrlen(world->arr_entities); i++)
    {
        // DO NOT USE -> entity_delete(world, &world->arr_entities[i]) here.
        // Directly free all components and free the entity's hashmap to avoid individually deleting each entity.
        free_all_components(world, &world->arr_entities[i]);
        shfree(world->arr_entities[i].sh_components);
    }
    // free entire entity array now that they're all cleaned up.
    arrfree(world->arr_entities);

    free_all_components(world, &world->resources);

    free(world);
}
entity_t *world_get_entities(world_t *world)
{
    return world->arr_entities;
}

void *world_create_resource_impl(world_t *world, const char *typename, size_t typesize)
{
    return entity_create_component_impl(&world->resources, typename, typesize);
}

void *world_get_resource_impl(world_t *world, const char *typename, size_t typesize)
{
    return entity_get_component_impl(&world->resources, typename, typesize);
}
void world_register_free_impl(world_t *world, const char *typename, custom_free_fn free_fn)
{
    shput(world->sh_free_fns, typename, free_fn);
}

entity_t *entity_new(world_t *world)
{
    entity_t val = {
        0};

    arrput(world->arr_entities, val);

    return &world->arr_entities[arrlen(world->arr_entities) - 1];
}

void free_all_components(world_t *world, entity_t *entity)
{
    for (size_t i = 0; i < shlen(entity->sh_components); i++)
    {
        custom_free_fn free_fn = shget(world->sh_free_fns, entity->sh_components[i].key);
        if (free_fn)
            free_fn(entity->sh_components[i].value);
        else
            free(entity->sh_components[i].value);
    }
}

void entity_delete(world_t *world, entity_t *entity)
{
    free_all_components(world, entity);

    shfree(entity->sh_components);
    int64_t index = (entity - world->arr_entities) / sizeof(entity);
    arrdelswap(world->arr_entities, index);
}

void *entity_create_component_impl(entity_t *entity, const char *typename, size_t typesize)
{
    void *val = calloc(1, typesize);
    assert(val);

    shput(entity->sh_components, typename, val);

    return val;
}
void *entity_get_component_impl(entity_t *entity, const char *typename, size_t typesize)
{
    // TODO WT: Grabbing the index anyway to see if it's got that component. should store that and use it instead of calling shget on the next line.
    if (shgeti(entity->sh_components, typename) != -1)
        return shget(entity->sh_components, typename);

    return 0;
}
