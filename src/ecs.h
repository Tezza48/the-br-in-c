#pragma once
#include <stdint.h>
#include "vec.h"

typedef struct rect_t
{

} rect_t;

// typedef struct grid_position_t
// {
//     int x, y;
// } grid_position_t;

// typedef struct user_t
// {
//     char *name;
//     size_t id;
// } user_t;

// Define X macro calls for the names of all components
#define ECS_COMPONENTS \
    // X(grid_position)   \
    // X(user)

// Declare entity type which has a ptr to any of all components
typedef struct entity_t
{
    size_t dead;
#define X(name) name##_t *name;
    ECS_COMPONENTS
#undef X
} entity_t;

VEC_TYPED_DECL(entity_t, entity_t);

typedef struct ecs_t
{
    vec_entity_t entities;
} ecs_t;

ecs_t ecs_init();

entity_t *ecs_spawn(ecs_t *ecs);
void ecs_despawn(ecs_t *ecs, entity_t *entity);
