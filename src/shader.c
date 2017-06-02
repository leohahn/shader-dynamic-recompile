#include <stdlib.h>
#include <stdio.h>

#include "glad/glad.h"

#include "shader.h"
#include "lt.h"

typedef struct Shader {
    GLuint program;
} Shader;

static const char *resources_path = "/home/lhahn/dev/c/shader-loader/resources/";
static Shader g_shaders[ShaderKind_Count] = {{0}};

Shader shader_get(ShaderKind kind) {
    LT_ASSERT(kind != ShaderKind_Count);
    return g_shaders[kind];
}

GLuint shader_make_program(const char* shader_name) {
    // Fetch source codes from each shader
    String *shader_src_path = string_make(resources_path);
    string_append(shader_src_path, shader_name);

    FileContents *shader_src = file_read_contents(shader_src_path->data);

    LT_ASSERT(shader_src != NULL);

    if (shader_src->error != FileError_None) {
        fprintf(stderr, "Error reading shader source from %s\n", shader_src_path->data);
        string_free(shader_src_path);
        file_free_contents(shader_src);
        return 0;
    }

    String *shader_string = string_make_ptrs((u8*)shader_src->data,
                                             (u8*)shader_src->data + shader_src->size - 1);

    file_free_contents(shader_src);

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    if (vertex_shader == 0 || fragment_shader == 0) {
        fprintf(stderr, "Error creating shaders (glCreateShader)\n");
        string_free(shader_src_path);
        file_free_contents(shader_src);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return 0;
    }

    GLuint program = 0;

    // Stores information about the compilation, so we can print it,
    GLchar info[512] = {0};
    GLint success;

    {
        const char *vertex_string[3] = {
            "#version 330 core\n",
            "#define COMPILING_VERTEX\n",
            shader_string->data
        };
        glShaderSource(vertex_shader, 3, &vertex_string[0], NULL);
    }

    {
        const char *fragment_string[3] = {
            "#version 330 core\n",
            "#define COMPILING_FRAGMENT\n",
            shader_string->data
        };
        glShaderSource(fragment_shader, 3, &fragment_string[0], NULL);
    }

    //printf("CODE:\n%s\n", shader_string->data);

    /* Debug("Compiling vertex shader ... "); */
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info);
        printf("ERROR: Vertex shader compilation failed:\n");
        printf("%s\n", info);
        goto error_cleanup;
    }
    /* printf("done\n"); */

    /* Debug("Compiling fragment shader ... "); */
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info);
        printf("ERROR: Fragment shader compilation failed:\n");
        printf("%s\n", info);
        goto error_cleanup;
    }
    /* printf("done\n"); */

    program = glCreateProgram();
    /* Debug("Linking shader program ... "); */
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info);
        printf("ERROR: Shader linking failed:\n");
        printf("%s\n", info);
        goto error_cleanup;
    }
    /* printf("done\n\n"); */

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    string_free(shader_string);
    string_free(shader_src_path);
    return program;

error_cleanup:
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    string_free(shader_string);
    string_free(shader_src_path);
    return 0;
}

GLuint shader_get_program(ShaderKind kind) {
    return g_shaders[kind].program;
}

void shader_recompile(ShaderKind kind) {
    switch (kind) {
    case ShaderKind_Basic: {
        printf("Recompiling Basic Shader\n");
        GLuint old_program = g_shaders[kind].program;
        GLuint new_program = shader_make_program("basic.glsl");

        if (new_program == 0) {
            return;
        }

        g_shaders[kind].program = new_program;
        glDeleteProgram(old_program);
    } break;

    default:
        LT_ASSERT(false);
    }
}

void shader_initialize() {
    Shader basic_shader;
    basic_shader.program = shader_make_program("basic.glsl");

    g_shaders[ShaderKind_Basic] = basic_shader;
}
