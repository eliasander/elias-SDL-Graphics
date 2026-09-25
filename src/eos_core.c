#include <eos_core.h>

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>

#include <SDL3/SDL.h>

#define ARRAYCOUNT(arr) sizeof(arr)/sizeof(*(arr))
bool WindowShouldClose = false;

int screenWidth, screenHeight = 0;
int baseWidth, baseHeight = 0;
int renderWidth, renderHeight = 0;
float scale = 1;

Viewport v = {0};

OpenGL_Context context = {0};

// Helper functions

vec4 colorConvert(Color color) {
    vec4 convertedColor = {
        (float)color.r/255,
        (float)color.g/255,
        (float)color.b/255,
        color.a
    };
    return convertedColor;
}

Color darken(Color color, float value) {
    color.r*=value;
    color.g*=value;
    color.b*=value;

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

void DrawRectangle(vec2 pos, vec2 size, Color color)
{
    if (context.rectCount > 1024) return;

    float x =
        (pos.x*scale) / (float)screenWidth * 2.0f - 1.0f;
    float y =
        1.0f - (pos.y*scale) / (float)screenHeight * 2.0f;

    float width  =
        (size.x*scale) / (float)screenWidth * 2.0f;
    float height =
        (size.y*scale) / (float)screenHeight * 2.0f;

    vec4 convertedColor = colorConvert(color);

    Vertex verticies[4] = {
        {x,         y,          convertedColor.x, convertedColor.y, convertedColor.z, convertedColor.w},
        {x + width, y,          convertedColor.x, convertedColor.y, convertedColor.z, convertedColor.w},
        {x,         y - height, convertedColor.x, convertedColor.y, convertedColor.z, convertedColor.w},
        {x + width, y - height, convertedColor.x, convertedColor.y, convertedColor.z, convertedColor.w}
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
            color.r, color.g, color.b, color.a
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

float check_scale(float height, float width, float base_width, float base_height)
{
    float scaleX = width / base_width;
    float scaleY = height / base_height;
    screenWidth = width;
    screenHeight = height;
    if (scaleX < scaleY)
        return scaleX;
    else
        return scaleY;
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
int flagCount = 0;

void SetWindowFlag(unsigned long long flag) {
    context.flags[flagCount++] = flag;
}

bool HasFlag(unsigned long long flag) {
    for (int i = 0; i < flagCount; i++) {
        if (context.flags[i] == flag)
            return true;
    }
    return false;
}

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
        switch (event.type)
        {
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            {
                int width, height;
                SDL_GetWindowSizeInPixels(window, &width, &height);
                compute_viewport(width, height, baseWidth, baseHeight);
                scale = check_scale(renderHeight, renderWidth, baseWidth, baseHeight);
                
            } break;
        }


        g_eventCallback(window, event);
    }
}

void StartFrame(SDL_Window* window) {
    pollEvents(window);

    return;
}

void Clear(Color color) {
    vec4 convertedColor = colorConvert(color);
    
    if (HasFlag(EOS_WINDOW_LETTERBOXING)) {
        glDisable(GL_SCISSOR_TEST);
        glViewport(0, 0, screenWidth, screenHeight);
        glClearColor(0,0,0,1);
        if (HasFlag(EOS_WINDOW_LETTERBOXING_COLOR_CLEAR))
            glClearColor(convertedColor.x, convertedColor.y, convertedColor.z, convertedColor.w);
        glClear(GL_COLOR_BUFFER_BIT);

        glViewport(v.x, v.y, v.w, v.h);
        glEnable(GL_SCISSOR_TEST);
        glScissor(v.x, v.y, v.w, v.h);
    }
    glClearColor(convertedColor.x, convertedColor.y, convertedColor.z, convertedColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
}



void EndFrame(SDL_Window* window) {
    SDL_GL_SwapWindow(window);

    return;
}

Viewport compute_viewport(float win_w, float win_h, float game_w, float game_h) {
    float scale = fminf(win_w / game_w, win_h / game_h);
    screenWidth = win_w;
    screenHeight = win_h;
    baseWidth = game_w;
    baseHeight = game_h;
    v.scale = scale;
    v.w = game_w * scale;
    v.h = game_h * scale;

    v.x = (win_w - v.w) * 0.5f;
    v.y = (win_h - v.h) * 0.5f;

    renderWidth = v.w;
    renderHeight = v.h;

    return v;
}