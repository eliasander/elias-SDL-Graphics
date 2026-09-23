#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>

#include <SDL3/SDL.h>

#define ARRAYCOUNT(arr) sizeof(arr)/sizeof(*(arr))
bool WindowShouldClose = false;

int screenWidth, screenHeight = 0;
float scale = 1;

OpenGL_Context context = {0};

// Helper functions

void colorConvert(vec4 *color) {
    color->x/=255;
    color->y/=255;
    color->z/=255;
}

vec4 darken(vec4 color, float value) {
    color.x*=value;
    color.y*=value;
    color.z*=value;

    return color;
}


// Function callbacks
EventCallback g_eventCallback;
// 3. Setter function returning int (0 = Success, -1 = Error)
bool setEventCallback(SDL_Window *window, EventCallback callback) {
    if (!window) {
        return false; // Return error code if window is NULL
    }

    g_eventCallback = callback;
    return true; // Return true for success
}


void DrawTriangle(vec2 pos, vec2 size, vec3 color)
{
    pos.x *= scale;
    pos.y *= scale;

    size.x *= scale;
    size.y *= scale;

    // NDC = Normalized Device Coordinates (-1 - 1)

    float x =
        pos.x / (float)screenWidth * 2.0f - 1.0f;
    float y =
        1.0f - pos.y / (float)screenHeight * 2.0f;

    float width =
        size.x / (float)screenWidth * 2.0f;
    float height =
        size.y / (float)screenHeight * 2.0f;


    float verticies[] = {
        x,         y,          color.x, color.y, color.z,
        x + width, y,          color.x, color.y, color.z,
        x + width, y - height, color.x, color.y, color.z,
    };
    // 0: Bind Vertex Buffer Object and copy data into buffer data
    // 1. bind Vertex Array Object
    glBindVertexArray(context.VAO);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, context.VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(verticies),
        verticies,
        GL_STATIC_DRAW
    );
    // 3. then set our vertex attributes pointers
    glVertexAttribPointer(
    0,
    3,
    GL_FLOAT,
    GL_FALSE,
    3 * sizeof(float),
    (void*)0
    );
    glEnableVertexAttribArray(0);



    // 1: Tell OpenGL how to interpret the vertices which we have given.

    // 2: Use Shader Program
    glUseProgram(context.shaderProgram);

    // 3: Render our objects

    glUseProgram(context.shaderProgram);
    glBindVertexArray(context.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void DrawRectangle(vec2 pos, vec2 size, vec4 color)
{
    if (context.rectCount > 500) return;

    float x =
        (pos.x*scale) / (float)screenWidth * 2.0f - 1.0f;
    float y =
        1.0f - (pos.y*scale) / (float)screenHeight * 2.0f;

    float width  =
        (size.x*scale) / (float)screenWidth * 2.0f;
    float height =
        (size.y*scale) / (float)screenHeight * 2.0f;

    colorConvert(&color);

    Vertex verticies[4] = {
        {x,         y,          color.x, color.y, color.z, color.w},
        {x + width, y,          color.x, color.y, color.z, color.w},
        {x,         y - height, color.x, color.y, color.z, color.w},
        {x + width, y - height, color.x, color.y, color.z, color.w}
    };


    RectInstance instance = {
        .pos = {
            x, y
        },
        .size = {
            x+width, y-height
        },
        .color =
        {
            color.x, color.y, color.z, color.w
        }
    };

    for (int i = 0; i < 4; i++) {
        context.vertices[context.rectCount*4+i] = verticies[i];
    }

    unsigned int indices[] = {
        context.rectCount*4+0, context.rectCount*4+1, context.rectCount*4+2,
        context.rectCount*4+1, context.rectCount*4+2, context.rectCount*4+3,
    };

    for (int i = 0; i < 6; i++) {
        context.indices[context.rectCount*6+i] = indices[i];
    }

    context.rectCount++;
}

void check_scale(float height, float width, float base_width, float base_height)
{
    float scaleX = width / base_width;
    float scaleY = height / base_height;
    screenWidth = width;
    screenHeight = height;
    if (scaleX < scaleY)
        scale = scaleX;
    else
        scale = scaleY;
}

char *read_file(const char *path)
{
    char *buffer;
    long size;
    FILE *file;

    file = fopen(path, "rb");

    if (!file) {
        perror(path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    buffer = malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    (void)fread(buffer, 1, size, file);
    buffer[size] = '\0';

    fclose(file);

    return buffer;
}

static SDL_Window *window = NULL;
static SDL_GLContext contextGL;

SDL_Window* CreateWindow() {

    SDL_SetAppMetadata("Snake Squared", "1.0", "com.eliasander.snakesquared");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return NULL;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow(
        "My OpenGL App",
        1280,
        720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    int w, h;
    SDL_GetWindowSizeInPixels(window, &w, &h);

    screenWidth = w;
    screenHeight = h;

    check_scale((float)h, (float)w, 256, 144);

    if (!window)
        return NULL;
    return window;
}

SDL_GLContext CreateOpenGLContext() {
    contextGL = SDL_GL_CreateContext(window);

    if (!contextGL)
        return NULL;

    // GLAD must be initialized AFTER the SDL OpenGL context exists.
    if (!gladLoadGLLoader(
            (GLADloadproc)SDL_GL_GetProcAddress))
    {
        return NULL;
    }

    return contextGL;
}

Shader CreateShader(int type, const char *path) {
    unsigned int shader;
    int success;
    char infoLog[512];

    if (type != GL_FRAGMENT_SHADER &&
        type != GL_VERTEX_SHADER)
    {
        printf("Invalid shader type! Aborting shader creation\n");

        return (Shader){0};
    }

    const char *source = read_file(path);
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("SHADER ERROR: %s\n%s\n", path, infoLog);
    }
    return (Shader){type, shader};
}

int CreateShaderProgram(Shader shaders[], int shaderCount) {

    int success;
    char infoLog[512];
    unsigned int program =
        glCreateProgram();

    for (int i = 0; i < shaderCount; i++)
    {
        glAttachShader(program, shaders[i].shader);
    }
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("SHADER PROGRAM LINK ERROR:\n%s\n", infoLog);
    }

    context.shaderProgram = program;

    return 0;
}


void DeleteShader(Shader shader) {
    glDeleteShader(shader.shader);
}


void pollEvents(SDL_Window* window) {
    if (g_eventCallback == NULL) return;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        g_eventCallback(window, event);
    }
}

void StartFrame(SDL_Window* window) {
    pollEvents(window);

    return;
}

void Clear(vec4 color) {
    colorConvert(&color);
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
}



void EndFrame(SDL_Window* window) {
    SDL_GL_SwapWindow(window);

    return;
}