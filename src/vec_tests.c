#include "vec_tests.h"
#include <stdio.h>

void vec_test_vec_new()
{
    puts("\tvec_test_vec_new");
    vec_result sut_result = vec_new(sizeof(uint32_t));

    assert(sut_result.type == VEC_RESULT_SUCCESS);

    vec sut = sut_result.new_result;

    assert(sut.capacity == 1);
    assert(sut.length == 0);
}

void vec_test_vec_expand()
{
    puts("\tvec_test_vec_expand");
    vec sut = vec_new(sizeof(uint32_t)).new_result;

    vec_result expand_result = vec_expand(&sut, 2);
    assert(expand_result.type == VEC_RESULT_SUCCESS);

    assert(sut.capacity == 3);
    assert(sut.length == 0);
}

void vec_test_vec_push()
{
    puts("\tvec_test_vec_push");
    vec sut = vec_new(sizeof(uint32_t)).new_result;

    uint32_t val = 5;
    vec_result push_result = vec_push(&sut, &val);
    assert(push_result.type == VEC_RESULT_SUCCESS);

    assert(((uint32_t *)sut.data)[0] == 5);
    assert(sut.length == 1);
    assert(sut.capacity == 1);
}

void vec_test_vec_pop()
{
    puts("\tvec_test_vec_pop");
    vec sut = vec_new(sizeof(uint32_t)).new_result;

    uint32_t val = 5;
    vec_push(&sut, &val);

    uint32_t popped;
    vec_result pop_result = vec_pop(&sut, (void *)&popped);
    assert(pop_result.type == VEC_RESULT_SUCCESS);

    assert(popped == 5);
    assert(sut.length == 0);
    assert(sut.capacity == 1);
}

VEC_TYPED_DECL(int32_t, int32);
VEC_TYPED_IMPL(int32_t, int32);

void vec_test_vec_int32_new()
{
    puts("\tvec_test_vec_int32_new");
    vec_int32_result sut_result = vec_int32_new();

    assert(sut_result.type == VEC_RESULT_SUCCESS);

    vec_int32 sut = sut_result.new_result;

    assert(sut.capacity == 1);
    assert(sut.length == 0);
}

void vec_test_vec_int32_expand()
{
    puts("\tvec_test_vec_int32_expand");
    vec_int32 sut = vec_int32_new().new_result;

    vec_int32_result expand_result = vec_int32_expand(&sut, 2);
    assert(expand_result.type == VEC_RESULT_SUCCESS);

    assert(sut.capacity == 3);
    assert(sut.length == 0);
}

void vec_test_vec_int32_push()
{
    puts("\tvec_test_int32_vec_push");
    vec_int32 sut = vec_int32_new().new_result;

    vec_int32_result push_result = vec_int32_push(&sut, 5);
    assert(push_result.type == VEC_RESULT_SUCCESS);

    assert(sut.data[0] == 5);
    assert(sut.length == 1);
    assert(sut.capacity == 1);
}

void vec_test_vec_int32_pop()
{
    puts("\tvec_test_vec_int32_pop");
    vec_int32 sut = vec_int32_new().new_result;
    vec_int32_push(&sut, 5);

    vec_int32_result pop_result = vec_int32_pop(&sut);
    assert(pop_result.type == VEC_RESULT_SUCCESS);

    int32_t popped = pop_result.pop_result;

    assert(popped == 5);
    assert(sut.length == 0);
    assert(sut.capacity == 1);
}

void vec_test_run()
{
    puts("Running vec tests\n");

    vec_test_vec_new();
    vec_test_vec_expand();
    vec_test_vec_push();
    vec_test_vec_pop();

    vec_test_vec_int32_new();
    vec_test_vec_int32_expand();
    vec_test_vec_int32_push();
    vec_test_vec_int32_pop();

    puts("\n\tAll vec tests passed\n\n");
}