#include "lib.h"
#include <stdio.h>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "vec.h"
#include "vendor/linmath.h"

typedef struct vertex_t
{
    vec3 pos;
    vec2 uv;
} vertex_t;

typedef struct sprite_t
{
    vec3 pos;
    vec2 scale;
    vec4 color;
} sprite_t;

typedef vertex_t sprite_quad_t[6];

typedef struct sprite_batch_t
{
    GLuint vertex_array;
    GLuint vertex_buffer;
    sprite_quad_t *quads_vertices;
    size_t num_quads;
    GLuint program;
    size_t max_batch_size;
} sprite_batch_t;

sprite_batch_t sprite_batch_new(GLuint program, size_t max_batch_size)
{
    sprite_batch_t result = {0};
    result.program = program;

    glCreateVertexArrays(1, &result.vertex_array);
    glBindVertexArray(result.vertex_array);

    glCreateBuffers(1, &result.vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, result.vertex_buffer);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void *)offsetof(vertex_t, uv));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    result.num_quads = 0;
    result.quads_vertices = calloc(max_batch_size, sizeof(sprite_quad_t));

    return result;
}

void sprite_batch_free(sprite_batch_t *self)
{
    free(self->quads_vertices);
    glDeleteBuffers(1, &self->vertex_buffer);
    glDeleteVertexArrays(1, &self->vertex_array);

    *self = (sprite_batch_t){0};
}

void sprite_batch_flush(sprite_batch_t *self)
{
    glNamedBufferData(self->vertex_buffer, self->num_quads * sizeof(sprite_quad_t), self->quads_vertices, GL_STATIC_DRAW);

    glUseProgram(self->program);
    glBindVertexArray(self->vertex_array);

    glDrawArrays(GL_TRIANGLES, 0, self->num_quads * 6);
    self->num_quads = 0;
}

size_t sprite_batch_draw(sprite_batch_t *self, sprite_t *sprite)
{
    sprite_quad_t vertices = {
        {{
             sprite->pos[0],
             sprite->pos[1],
             sprite->pos[2],
         },
         {0.0, 0.0}},
        {{
             sprite->pos[0],
             sprite->pos[1] + sprite->scale[1],
             sprite->pos[2],
         },
         {0.0, 1.0}},
        {{
             sprite->pos[0] + sprite->scale[0],
             sprite->pos[1] + sprite->scale[1],
             sprite->pos[2],
         },
         {1.0, 1.0}},
        {{
             sprite->pos[0],
             sprite->pos[1],
             sprite->pos[2],
         },
         {0.0, 0.0}},
        {{
             sprite->pos[0] + sprite->scale[0],
             sprite->pos[1] + sprite->scale[1],
             sprite->pos[2],
         },
         {1.0, 1.0}},
        {{
             sprite->pos[0] + sprite->scale[0],
             sprite->pos[1],
             sprite->pos[2],
         },
         {1.0, 0.0}},
    };

    memcpy_s(&self->quads_vertices[self->num_quads++], sizeof(sprite_quad_t), vertices, sizeof(sprite_quad_t));

    if (self->num_quads == self->max_batch_size)
    {
        sprite_batch_flush(self);
        return 1;
    }

    return 0;
}

typedef struct camera_t
{
    vec2 pos;
    float aspect;
    float size;
    mat4x4 view_proj;
} camera_t;

void GL_CALL_IMPL(char *file, size_t line)
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

#define GL_CALL(x) \
    x;             \
    GL_CALL_IMPL(__FILE__, __LINE__)

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
    float window_width = 640, window_height = 360;
    SDL_Window *window = SDL_CreateWindow("Hello, SDL!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_OPENGL);
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

    // GLuint vao;
    // {
    //     GL_CALL(glGenVertexArrays(1, &vao));
    //     GL_CALL(glBindVertexArray(vao));
    // }

    // GLuint vBuffer;
    // {
    //     {
    //         GL_CALL(glCreateBuffers(1, &vBuffer));

    //         char *vBuffName = "Quad(VertexBuffer)";
    //         GL_CALL(glObjectLabel(GL_BUFFER, vBuffer, -1, vBuffName));

    //         vertex_t vertices[6] = {
    //             {{-0.5, -0.5, 0.0}, {0.0, 0.0}},
    //             {{-0.5, 0.5, 0.0}, {0.0, 1.0}},
    //             {{0.5, 0.5, 0.0}, {1.0, 1.0}},

    //             {{-0.5, -0.5, 0.0}, {0.0, 0.0}},
    //             {{0.5, 0.5, 0.0}, {1.0, 1.0}},
    //             {{0.5, -0.5, 0.0}, {1.0, 0.0}},
    //         };

    //         GL_CALL(glNamedBufferData(vBuffer, sizeof(vertices), (void *)vertices, GL_STATIC_DRAW));
    //     }

    //     GL_CALL(glBindVertexArray(vao));
    //     GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vBuffer));

    //     GL_CALL(glEnableVertexAttribArray(0));
    //     GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0));
    //     GL_CALL(glEnableVertexAttribArray(1));
    //     GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void *)offsetof(vertex_t, uv)));
    //     GL_CALL(glBindVertexArray(0));
    // }

    GLenum shader_types[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
    GLuint program = create_program("./shader/shader.glsl", shader_types, 2);
    sprite_batch_t sprite_batch = sprite_batch_new(program, 1024);

    camera_t camera = {
        .pos = {0.0, 0.0},
        .aspect = window_width / window_height,
        .size = 10,
    };

    float w = camera.size / 2, h = (camera.size / 2) / camera.aspect;
    mat4x4_ortho(camera.view_proj, -w, w, -h, h, -0, 100);

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

        glUniformMatrix4fv(glGetUniformLocation(program, "mat_view_proj"), 1, GL_FALSE, camera.view_proj[0]);

        sprite_t sprite = {
            {0.0, 0.0, 0.0},
            {1.0, 1.0},
            {1.0, 1.0, 1.0},
        };
        size_t did_batcher_flush = sprite_batch_draw(&sprite_batch, &sprite);

        if (!did_batcher_flush)
        {
            sprite_batch_flush(&sprite_batch);
        }

        SDL_GL_SwapWindow(window);
    }

    sprite_batch_free(&sprite_batch);
    GL_CALL(glDeleteProgram(program));

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 1;
}