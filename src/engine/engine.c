#include "engine.h"
#include "../util/fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void _gl_call_impl(char *file, size_t line)
{
#define X_OPENGL_ERRORS                 \
    X(GL_NO_ERROR)                      \
    X(GL_INVALID_ENUM)                  \
    X(GL_INVALID_VALUE)                 \
    X(GL_INVALID_OPERATION)             \
    X(GL_INVALID_FRAMEBUFFER_OPERATION) \
    X(GL_OUT_OF_MEMORY)                 \
    X(GL_STACK_UNDERFLOW)               \
    X(GL_STACK_OVERFLOW)
#define X(error)                              \
    case error:                               \
        dump_gl_errors_error_string = #error; \
        break;

    GLenum dump_gl_errors_error;
    while ((dump_gl_errors_error = glGetError()) != GL_NO_ERROR)
    {

        const char *dump_gl_errors_error_string;
        switch (dump_gl_errors_error)
        {

            X_OPENGL_ERRORS
        }

        printf("GL ERROR:\n\t%s:%d: 0x%x %s\n", __FILE__, __LINE__, dump_gl_errors_error, dump_gl_errors_error_string);
    }

#undef X
#undef X_OPENGL_ERRORS
}

GLuint createAndCompileShader(const char *path, GLenum type)
{

    GLuint shader = glCreateShader(type);
    const char *source = readFileToString(path);
    GL_CALL(glShaderSource(shader, 1, &source, 0));
    GL_CALL(glCompileShader(shader));

    GLint shaderCompiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);
    if (shaderCompiled != GL_TRUE)
    {
        GLsizei logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        char *log = calloc(logLength, sizeof(char));
        glGetShaderInfoLog(shader, logLength, 0, log);
        printf("Failed to compile shader\n\tFile:\t%s\n\tError:\t%s\n", path, log);
        assert(0);
    }

    free((void *)source);

    return shader;
}

GLuint create_program(const char *path, GLenum *p_shader_types, size_t num_shader_types)
{
    GLuint program = GL_CALL(glCreateProgram());

    GLuint *shaders = (GLuint *)alloca(num_shader_types * sizeof(GLenum));

    char *source = readFileToString(path);
    for (size_t shader_index = 0; shader_index < num_shader_types; shader_index++)
    {
        GLenum type = p_shader_types[shader_index];

        const char *preamble;
        switch (type)
        {
        case GL_VERTEX_SHADER:
        {
            preamble = "#define COMPILE_VERTEX_SHADER 1\n";
            break;
        }
        case GL_FRAGMENT_SHADER:
        {
            preamble = "#define COMPILE_FRAGMENT_SHADER 1\n";
            break;
        }
        }

        GLuint shader = GL_CALL(glCreateShader(type));
        shaders[shader_index] = shader;
        const char *sources[3] = {"#version 460\n", preamble, source};
        GL_CALL(glShaderSource(shader, 3, sources, 0));
        GL_CALL(glCompileShader(shader));

        GLint shaderCompiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);
        if (shaderCompiled != GL_TRUE)
        {
            GLsizei logLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
            char *log = calloc(logLength, sizeof(char));
            glGetShaderInfoLog(shader, logLength, 0, log);
            printf("Failed to compile shader\n\tFile:\t%s\n\tError:\t%s\n", path, log);
            assert(0);
        }

        glAttachShader(program, shader);
    }

    glLinkProgram(program);

    for (size_t i = 0; i < num_shader_types; i++)
    {
        GL_CALL(glDetachShader(program, shaders[i]));
        GL_CALL(glDeleteShader(shaders[i]));
    }

    free((void *)source);
    return program;
}