#include "lib.h"
#include <stdio.h>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "vec.h"

#define GL_CALL(x)                                                                                                          \
    x;                                                                                                                      \
    {                                                                                                                       \
        GLenum dump_gl_errors_error;                                                                                        \
        while ((dump_gl_errors_error = glGetError()) != GL_NO_ERROR)                                                        \
        {                                                                                                                   \
            char *dump_gl_errors_error_string;                                                                              \
            switch (dump_gl_errors_error)                                                                                   \
            {                                                                                                               \
            case GL_NO_ERROR:                                                                                               \
                dump_gl_errors_error_string = "GL_NO_ERROR";                                                                \
                break;                                                                                                      \
            case GL_INVALID_ENUM:                                                                                           \
                dump_gl_errors_error_string = "GL_INVALID_ENUM";                                                            \
                break;                                                                                                      \
            case GL_INVALID_VALUE:                                                                                          \
                dump_gl_errors_error_string = "GL_INVALID_VALUE";                                                           \
                break;                                                                                                      \
            case GL_INVALID_OPERATION:                                                                                      \
                dump_gl_errors_error_string = "GL_INVALID_OPERATION";                                                       \
                break;                                                                                                      \
            case GL_INVALID_FRAMEBUFFER_OPERATION:                                                                          \
                dump_gl_errors_error_string = "GL_INVALID_FRAMEBUFFER_OPERATION";                                           \
                break;                                                                                                      \
            case GL_OUT_OF_MEMORY:                                                                                          \
                dump_gl_errors_error_string = "GL_OUT_OF_MEMORY";                                                           \
                break;                                                                                                      \
            case GL_STACK_UNDERFLOW:                                                                                        \
                dump_gl_errors_error_string = "GL_STACK_UNDERFLOW";                                                         \
                break;                                                                                                      \
            case GL_STACK_OVERFLOW:                                                                                         \
                dump_gl_errors_error_string = "GL_STACK_OVERFLOW";                                                          \
                break;                                                                                                      \
            }                                                                                                               \
            printf("GL ERROR:\n\t%s:%d: 0x%x %s\n", __FILE__, __LINE__, dump_gl_errors_error, dump_gl_errors_error_string); \
        }                                                                                                                   \
    }

char *readFileToString(const char *path)
{
    FILE *file = fopen(path, "r");

    assert(file != 0);

    fseek(file, 0, SEEK_END);

    int64_t numBytes = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *text = calloc(numBytes, sizeof(char));

    fread(text, sizeof(char), numBytes, file);

    fclose(file);

    return text;
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

lib_start_result lib_start()
{
    SDL_Window *window = SDL_CreateWindow("Hello, SDL!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    if (!window)
    {
        return 0;
    }

    SDL_ShowWindow(window);

    const uint8_t *scancode_to_state_map = SDL_GetKeyboardState(0);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    assert(context != 0);

    if (!gladLoadGLLoader(&SDL_GL_GetProcAddress))
    {
        return 0;
    }

    GLuint vao;
    {
        GL_CALL(glGenVertexArrays(1, &vao));
        GL_CALL(glBindVertexArray(vao));
    }

    GLuint vBuffer;
    {
        {
            GL_CALL(glCreateBuffers(1, &vBuffer));

            char *vBuffName = "Quad(VertexBuffer)";
            GL_CALL(glObjectLabel(GL_BUFFER, vBuffer, -1, vBuffName));

            float vertices[6][5] = {
                {-0.5, -0.5, 0.0, 0.0, 0.0},
                {-0.5, 0.5, 0.0, 0.0, 1.0},
                {0.5, 0.5, 0.0, 1.0, 1.0},

                {-0.5, -0.5, 0.0, 0.0, 0.0},
                {0.5, 0.5, 0.0, 1.0, 1.0},
                {0.5, -0.5, 0.0, 1.0, 0.0},
            };

            GL_CALL(glNamedBufferData(vBuffer, sizeof(vertices), vertices, GL_STATIC_DRAW));
        }

        GL_CALL(glBindVertexArray(vao));
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vBuffer));

        GL_CALL(glEnableVertexAttribArray(0));
        GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0));
        GL_CALL(glEnableVertexAttribArray(1));
        GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void *)(sizeof(float) * 3)));
        GL_CALL(glBindVertexArray(0));
    }

    GLenum shader_types[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
    GLuint program = create_program("./shader/shader.glsl", shader_types, 2);

    uint8_t running = 1;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            default:
                break;
            }
        }

        if (scancode_to_state_map[SDL_SCANCODE_ESCAPE])
            running = 0;

        GL_CALL(glClearColor(0.5, 0.5, 0.5, 1.0));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

        GL_CALL(glUseProgram(program));

        GL_CALL(glBindVertexArray(vao));
        GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));

        GL_CALL(glBindVertexArray(0));

        SDL_GL_SwapWindow(window);
    }

    GL_CALL(glDeleteBuffers(1, &vBuffer));
    GL_CALL(glDeleteProgram(program));

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 1;
}