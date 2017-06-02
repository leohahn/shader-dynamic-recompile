#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h"

typedef enum ShaderKind {
    ShaderKind_Basic,
    ShaderKind_Count
} ShaderKind;

void   shader_initialize();
GLuint shader_get_program(ShaderKind kind);
void   shader_recompile(ShaderKind kind);

#endif // SHADER_H
