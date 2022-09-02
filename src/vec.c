#include "vec.h"
#include <stdlib.h>
#include <string.h>

vec_result vec_new(uint32_t element_size)
{
    vec result = {0};
    result.length = 0;
    result.capacity = 1;
    result.element_size = element_size;
    result.data = calloc(result.capacity, element_size);

    if (result.data)
    {
        return (vec_result){
            .type = VEC_RESULT_SUCCESS,
            .new_result = result,
        };
    }
    return (vec_result){
        .type = VEC_RESULT_FAILED,
        .failed = "Failed to allocate capacity in vec_new",
    };
}

vec_result vec_expand(vec *vec_ptr, uint32_t count)
{
    uint32_t new_capacity = vec_ptr->capacity + count;
    void *new_data = realloc(vec_ptr->data, new_capacity * vec_ptr->element_size);
    if (!new_data)
    {
        return (vec_result){
            .type = VEC_RESULT_FAILED,
            .failed = "Failed to reallocate capacity in vec_push",
        };
    }

    vec_ptr->capacity = new_capacity;
    vec_ptr->data = new_data;

    return (vec_result){
        .type = VEC_RESULT_SUCCESS};
}

vec_result vec_push(vec *vec_ptr, void *element)
{
    if (vec_ptr->length >= vec_ptr->capacity)
    {
        vec_result result = vec_expand(vec_ptr, 1);
        if (result.type != VEC_RESULT_SUCCESS)
        {
            return result;
        }
    }

    if (memcpy_s(
            vec_ptr->data + vec_ptr->length * vec_ptr->element_size,
            vec_ptr->element_size,
            element,
            vec_ptr->element_size) != 0)
    {
        return (vec_result){
            .type = VEC_RESULT_FAILED,
            .failed = "Failed to copy element into vec_ptr->data with memcpy_s",
        };
    }
    vec_ptr->length += 1;

    return (vec_result){
        .type = VEC_RESULT_SUCCESS,
    };
}

vec_result vec_pop(vec *vec_ptr, void *popped)
{
    errno_t result = memcpy_s(
        popped,
        vec_ptr->element_size,
        vec_ptr->data + (vec_ptr->length - 1) * vec_ptr->element_size,
        vec_ptr->element_size);

    if (result != 0)
    {
        return (vec_result){
            .type = VEC_RESULT_FAILED,
            .failed = "Failed to copy last element in vec_ptr->data to popped",
        };
    }

    vec_ptr->length -= 1;

    return (vec_result){
        .type = VEC_RESULT_SUCCESS,
    };
}

void vec_free(void *vec_ptr)
{
    free(((vec *)vec_ptr)->data);
    *(vec *)vec_ptr = (vec){0};
}