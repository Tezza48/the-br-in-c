#pragma once

#include <glad/glad.h>

#define GL_CALL(x) \
    x;             \
    _gl_call_impl(__FILE__, __LINE__)

void _gl_call_impl(char *file, int line);

// TODO WT: Move these to somwhere specifically for shaders.
GLuint createAndCompileShader(const char *path, GLenum type);

GLuint create_program(const char *path, GLenum *p_shader_types, size_t num_shader_types);