#pragma once
#include "stdint.h"
#include "assert.h"

// TODO WT: world_free_resource, entity_free_component.

typedef void *(*custom_free_fn)(void *);

typedef struct free_fn_map_t
{
    const char *key;
    custom_free_fn value;
} free_fn_map_t;

typedef struct component_map_t
{
    const char *key;
    void *value;
} component_map_t;

typedef struct entity_t
{
    component_map_t *sh_components;
} entity_t;

typedef struct world_t
{
    entity_t *arr_entities;
    entity_t resources;
    free_fn_map_t *sh_free_fns;
} world_t;

void *world_create_resource_impl(world_t *, const char *typename, size_t typesize);
void *world_get_resource_impl(world_t *, const char *, size_t);
void world_register_free_impl(world_t *world, const char *, custom_free_fn);

void *entity_create_component_impl(entity_t *, const char *typename, size_t typesize);
void *entity_get_component_impl(entity_t *, const char *, size_t);

world_t *world_new();
void world_free(world_t *);
entity_t *world_get_entities(world_t *world);
#define world_create_resource(p_world, type) (type *)world_create_resource_impl(p_world, #type, sizeof(type))
#define world_get_resource(p_world, type) (type *)world_get_resource_impl(p_world, #type, sizeof(type))
#define world_register_free(p_world, type, fn) world_register_free_impl(p_world, #type, fn)

entity_t *entity_new(world_t *);
void entity_delete(world_t *, entity_t *);
#define entity_create_component(p_entity, type) (type *)entity_create_component_impl(p_entity, #type, sizeof(type))
#define entity_get_component(p_entity, type) (type *)entity_get_component_impl(p_entity, #type, sizeof(type))