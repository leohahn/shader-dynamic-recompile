#include <stdio.h>
#include <pthread.h>
#include <linux/inotify.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#define LT_IMPLEMENTATION
#include "lt.h"
#include "watcher.h"
#include "shader.h"

bool g_keyboard[1024] = {0};

void framebuffer_size_callback(GLFWwindow *w, i32 width, i32 height) {
    LT_UNUSED(w);
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *w) {
    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        printf("Setting window to close\n");
        glfwSetWindowShouldClose(w, true);
    }

    g_keyboard[GLFW_KEY_W] = glfwGetKey(w, GLFW_KEY_W);
}

void process_watcher_events() {
    WatcherEvent *ev;
    while((ev = watcher_peek_event()) != NULL) {
        printf("Consuming Event...\n");

        if (ev->inotify_mask & IN_ISDIR) {
            printf("Something happened with directory %s, IGNORING.\n", ev->name->data);
        } else if (ev->inotify_mask & IN_MODIFY) {
            shader_recompile(ShaderKind_Basic);
        } else {
            printf("Event on %s is not relevant, IGNORING.\n", ev->name->data);
        }

        // Notify the watcher that the event was consumed.
        watcher_event_peeked();
    }
}


GLFWwindow *create_window_and_set_context(const char *title, i32 width, i32 height) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);

    if (window == NULL) {
        glfwTerminate();
        LT_FAIL("Failed to create glfw window.\n");
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return window;
}

int main(void) {
    const i32 WINDOW_WIDTH = 800;
    const i32 WINDOW_HEIGHT = 600;

    glfwInit();

    GLFWwindow *window = create_window_and_set_context("Hot Shader Loader", WINDOW_WIDTH, WINDOW_HEIGHT);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LT_FAIL("Failed to initialize GLAD\n");
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

#ifdef DEV_ENV
    pthread_t watcher_thread;
    pthread_create(&watcher_thread, NULL, watcher_start, NULL);
#endif

    shader_initialize();

    GLfloat vertices[] = {
        0.0f, 1.0f,
        1.0f, 0.0f,
        -1.0f, 0.0f,
    };

    GLuint vao, vbo;
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
        glEnableVertexAttribArray(0);
    }

    bool running = true;
    while (running) {
        process_input(window);
        process_watcher_events();

        if (glfwWindowShouldClose(window)) {
            running = false;
#ifdef DEV_ENV
            watcher_stop();
#endif
            continue;
        }

        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader_get_program(ShaderKind_Basic));
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glUseProgram(0);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
#ifdef DEV_ENV
    pthread_join(watcher_thread, NULL);
#endif
}
