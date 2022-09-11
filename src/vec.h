#pragma once

#include <stdint.h>

typedef struct vec
{
    void *data;
    uint32_t length;
    uint32_t capacity;
    uint32_t element_size;
} vec;

typedef enum vec_result_type
{
    VEC_RESULT_SUCCESS,
    VEC_RESULT_FAILED,
} vec_result_type;

typedef struct vec_result
{
    vec_result_type type;
    union
    {
        vec new_result;
        char *failed;
    };
} vec_result;

vec_result vec_new(uint32_t element_size);

vec_result vec_expand(vec *vec_ptr, uint32_t count);

vec_result vec_push(vec *vec_ptr, void *element);

vec_result vec_pop(vec *vec_ptr, void *popped);

void vec_free(vec *vec_ptr);

#define VEC_TYPED_DECL(TYPE, USER_TYPE_NAME)                                                                    \
    typedef struct vec_##USER_TYPE_NAME                                                                         \
    {                                                                                                           \
        TYPE *data;                                                                                             \
        uint32_t length;                                                                                        \
        uint32_t capacity;                                                                                      \
        uint32_t element_size;                                                                                  \
    } vec_##USER_TYPE_NAME;                                                                                     \
    typedef struct vec_##USER_TYPE_NAME##_result                                                                \
    {                                                                                                           \
        vec_result_type type;                                                                                   \
        union                                                                                                   \
        {                                                                                                       \
            vec_##USER_TYPE_NAME new_result;                                                                    \
            char *failed;                                                                                       \
            TYPE pop_result;                                                                                    \
        };                                                                                                      \
    } vec_##USER_TYPE_NAME##_result;                                                                            \
    vec_##USER_TYPE_NAME##_result vec_##USER_TYPE_NAME##_new();                                                 \
    vec_##USER_TYPE_NAME##_result vec_##USER_TYPE_NAME##_expand(vec_##USER_TYPE_NAME *vec_ptr, uint32_t count); \
    vec_##USER_TYPE_NAME##_result vec_##USER_TYPE_NAME##_push(vec_##USER_TYPE_NAME *vec_ptr, TYPE element);     \
    vec_##USER_TYPE_NAME##_result vec_##USER_TYPE_NAME##_pop(vec_##USER_TYPE_NAME *vec_ptr);

#define VEC_TYPED_IMPL(TYPE, USER_TYPE_NAME)                                                                   \
    vec_##USER_TYPE_NAME##_result vec_##USER_TYPE_NAME##_new()                                                 \
    {                                                                                                          \
        vec_result result = vec_new(sizeof(TYPE));                                                             \
        if (result.type != VEC_RESULT_SUCCESS)                                                                 \
        {                                                                                                      \
            return (vec_##USER_TYPE_NAME##_result){                                                            \
                .type = result.type,                                                                           \
                .failed = result.failed,                                                                       \
            };                                                                                                 \
        }                                                                                                      \
        vec vec = result.new_result;                                                                           \
        vec_##USER_TYPE_NAME typed_vec = {                                                                     \
            .length = vec.length,                                                                              \
            .capacity = vec.capacity,                                                                          \
            .element_size = vec.element_size,                                                                  \
            .data = (TYPE *)vec.data,                                                                          \
        };                                                                                                     \
        return (vec_##USER_TYPE_NAME##_result){                                                                \
            .type = result.type,                                                                               \
            .new_result = typed_vec,                                                                           \
        };                                                                                                     \
    }                                                                                                          \
    vec_##USER_TYPE_NAME##_result vec_##USER_TYPE_NAME##_expand(vec_##USER_TYPE_NAME *vec_ptr, uint32_t count) \
    {                                                                                                          \
        vec_result result = vec_expand((vec *)vec_ptr, count);                                                 \
        if (result.type != VEC_RESULT_SUCCESS)                                                                 \
        {                                                                                                      \
            return (vec_##USER_TYPE_NAME##_result){                                                            \
                .type = result.type,                                                                           \
                .failed = result.failed,                                                                       \
            };                                                                                                 \
        }                                                                                                      \
        return (vec_##USER_TYPE_NAME##_result){.type = result.type};                                           \
    }                                                                                                          \
    vec_##USER_TYPE_NAME##_result vec_##USER_TYPE_NAME##_push(vec_##USER_TYPE_NAME *vec_ptr, TYPE element)     \
    {                                                                                                          \
        vec_result result = vec_push((vec *)vec_ptr, &element);                                                \
        if (result.type != VEC_RESULT_SUCCESS)                                                                 \
        {                                                                                                      \
            return (vec_##USER_TYPE_NAME##_result){                                                            \
                .type = result.type,                                                                           \
                .failed = result.failed,                                                                       \
            };                                                                                                 \
        }                                                                                                      \
        return (vec_##USER_TYPE_NAME##_result){.type = result.type};                                           \
    }                                                                                                          \
    vec_##USER_TYPE_NAME##_result vec_##USER_TYPE_NAME##_pop(vec_##USER_TYPE_NAME *vec_ptr)                    \
    {                                                                                                          \
        TYPE pop = {0};                                                                                        \
        vec_result result = vec_pop((vec *)vec_ptr, &pop);                                                     \
        if (result.type != VEC_RESULT_SUCCESS)                                                                 \
        {                                                                                                      \
            return (vec_##USER_TYPE_NAME##_result){                                                            \
                .type = result.type,                                                                           \
                .failed = result.failed,                                                                       \
            };                                                                                                 \
        }                                                                                                      \
        return (vec_##USER_TYPE_NAME##_result){.type = result.type, .pop_result = pop};                        \
    }
