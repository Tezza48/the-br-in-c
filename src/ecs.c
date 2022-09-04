#include "ecs.h"

VEC_TYPED_IMPL(entity_t, entity_t);

ecs_t ecs_init()
{
    return (ecs_t){
        .entities = vec_entity_t_new().new_result,
    };
}

entity_t *ecs_spawn(ecs_t *ecs)
{
    entity_t *result = 0;
    for (size_t i = 0; i < ecs->entities.length; i++)
    {
        if (ecs->entities.data[i].dead)
        {
            result = &ecs->entities.data[i];
        }
    }

    if (!result)
    {
        vec_entity_t_push(&ecs->entities, (entity_t){0});
        result = &ecs->entities.data[ecs->entities.length - 1];
    }

    result->dead = 0;

    return result;
}

void ecs_despawn(ecs_t *ecs, entity_t *entity)
{
#define X(name)             \
    if (entity->name)       \
    {                       \
        free(entity->name); \
        entity->name = 0;   \
    }

    ECS_COMPONENTS

#undef X
    entity->dead = 1;
}