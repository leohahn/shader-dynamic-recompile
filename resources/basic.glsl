/* ====================================
 *
 *   Vertex Shader
 *
 * ==================================== */
#ifdef COMPILING_VERTEX

layout (location = 0) in vec2 position;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
}

#endif

/* ====================================
 *
 *   Fragment Shader
 *
 * ==================================== */
#ifdef COMPILING_FRAGMENT

out vec4 color;

void main() {
    color = vec4(0.8f, 1.0f, 0.3f, 1.0f);
}

#endif
