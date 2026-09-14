
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>


#include "GLFW/glfw3.h"

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define BASE_WIDTH 256
#define BASE_HEIGHT 144
#define ARRAYCOUNT(arr) sizeof(arr)/sizeof(*(arr))

typedef struct
{
    float x,y;
} vec2;

typedef struct
{
    float x,y,z;
} vec3;

typedef struct {
    vec2 pos;
    vec3 color;
} Vertex;

typedef struct {
    vec2 pos;
    vec2 size;
    vec3 color;
} RectInstance;

typedef struct
{
    GLuint VBO;
    GLuint VAO;
    GLuint EBO;
    GLuint shaderProgram;
    Vertex vertices[1024*4];
    unsigned int indices[1024*6];
    unsigned int rectCount;
} OpenGL_Context;

#define RED (vec3){1.0, 0.0, 0.0}
#define GREEN (vec3){0.0, 1.0, 0.0}
#define BLUE (vec3){0.0, 0.0, 1.0}
#define WHITE (vec3){1.0, 1.0, 1.0}
#define MAPGREEN1 (vec3){0.22, 0.67, 0.35}
#define MAPGREEN2 (vec3){0.14, 0.53, 0.24}

static OpenGL_Context context;

int screenWidth, screenHeight = 0;
float scale = 1;

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

void DrawRectangle(vec2 pos, vec2 size, vec3 color)
{

    float x =
        (pos.x*scale) / (float)screenWidth * 2.0f - 1.0f;
    float y =
        1.0f - (pos.y*scale) / (float)screenHeight * 2.0f;

    float width  =
        (size.x*scale) / (float)screenWidth * 2.0f;
    float height =
        (size.y*scale) / (float)screenHeight * 2.0f;

    Vertex verticies[4] = {
        {x,         y,          color.x, color.y, color.z},
        {x + width, y,          color.x, color.y, color.z},
        {x,         y - height, color.x, color.y, color.z},
        {x + width, y - height, color.x, color.y, color.z}
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

void check_scale(float height, float width)
{
    float scaleX = width / BASE_WIDTH;
    float scaleY = height / BASE_HEIGHT;
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

    fread(buffer, 1, size, file);
    buffer[size] = '\0';

    fclose(file);

    return buffer;
}

unsigned int create_shader(int type, const char *path)
{
    unsigned int shader;
    int success;
    char infoLog[512];

    if (type != GL_FRAGMENT_SHADER &&
        type != GL_VERTEX_SHADER)
    {
        printf("Invalid shader type! Aborting shader creation\n");

        return 0;
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
    return shader;
}

unsigned int create_shader_program(unsigned int shaders[], int shaderCount)
{
    int success;
    char infoLog[512];
    unsigned int program =
        glCreateProgram();

    for (int i = 0; i < shaderCount; i++)
    {
        glAttachShader(program, shaders[i]);
    }
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("SHADER PROGRAM LINK ERROR:\n%s\n", infoLog);
    }
    return program;
}

void input_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    static bool vsync = true;
    static bool wired = false;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        vsync = !vsync;
        glfwSwapInterval(vsync);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        wired = !wired;
        if (wired)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

static SDL_Window *window = NULL;
static SDL_GLContext contextGL;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Snake Squared", "1.0", "com.eliasander.snakesquared");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
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

    check_scale((float)h, (float)w);

    if (!window)
        return -1;

    contextGL = SDL_GL_CreateContext(window);

    if (!contextGL)
        return -1;

    // GLAD must be initialized AFTER the SDL OpenGL context exists.
    if (!gladLoadGLLoader(
            (GLADloadproc)SDL_GL_GetProcAddress))
    {
        return -1;
    }


    // Create vertex shader
    GLuint vertexShader =
        create_shader(GL_VERTEX_SHADER, "shaders/basic.vert");

    // Create fragment shader
    GLuint fragmentShader =
        create_shader(GL_FRAGMENT_SHADER, "shaders/basic.frag");


    // Create Shader Program
    GLuint shaders[2] = {
        vertexShader,
        fragmentShader
    };


    context.shaderProgram =
        create_shader_program(shaders, ARRAYCOUNT(shaders));

    // Delete shaders - these aren't needed anymore since they exist within the program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // Generate buffers
    glGenBuffers(1, &context.VBO);
    glGenVertexArrays(1, &context.VAO);
    glGenBuffers(1, &context.EBO);


    glBindBuffer(GL_ARRAY_BUFFER, context.VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(context.vertices),
        NULL,
        GL_DYNAMIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context.EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(context.indices),
        NULL,
        GL_DYNAMIC_DRAW
    );




    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    static bool vsync = true;
    static bool wired = false;

    switch (event->type)
    {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;

    case SDL_EVENT_KEY_DOWN:
        {
            // Ignore key-repeat events.
            if (event->key.repeat)
                break;

            switch (event->key.key)
            {
            case SDLK_ESCAPE:
                return SDL_APP_SUCCESS;

            case SDLK_E:
                vsync = !vsync;

                SDL_GL_SetSwapInterval(vsync ? 1 : 0);
                break;

            case SDLK_SPACE:
                wired = !wired;

                if (wired)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                else
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                break;
            }
        } break;

    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        {
            int width;
            int height;

            SDL_GetWindowSizeInPixels(
                window,
                &width,
                &height
            );

            glViewport(0, 0, width, height);

            screenWidth = width;
            screenHeight = height;

            check_scale(
                (float)height,
                (float)width
            );
        } break;
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    // OpenGL code can now be used normally.
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    context.rectCount = 0;

    // Render here
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int tileSize = 6;
    for (int y = 0; y < 15; y++)
    {
        for (int x = 0; x < 17; x++)
        {
            vec3 color;
            if ((x+y) % 2 == 0)
                color = MAPGREEN1;
            else
                color = MAPGREEN2;

            DrawRectangle(
            (vec2){
                30.0f+(float)x*(float)tileSize,
                30.0f+(float)y*(float)tileSize},
            (vec2){
                (float)tileSize,
                (float)tileSize },
            color);
        }
    }

    // Bind VAO FIRST
    glBindVertexArray(context.VAO);

    // Upload vertices
    glBindBuffer(GL_ARRAY_BUFFER, context.VBO);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        (context.rectCount*4)*sizeof(*context.vertices),
        context.vertices
    );

    // Upload indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context.EBO);
    glBufferSubData(
        GL_ELEMENT_ARRAY_BUFFER,
        0,
        (context.rectCount*6)*sizeof(*context.indices),
        context.indices
    );

    // Position attribute: 3 floats
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, pos)
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,                  // location
        3,                  // vec3
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),  // stride
        (void*)offsetof(Vertex, color)
    );
    glEnableVertexAttribArray(1);

    // Shader
    glUseProgram(context.shaderProgram);

    // Draw rectangle (2 triangles = 6 indices)
    glDrawElements(
        GL_TRIANGLES,
        context.rectCount*6,
        GL_UNSIGNED_INT,
        0
    );
    // render...

    SDL_GL_SwapWindow(window);

    // region FPS
    static int frameCount = 0;
    static double previousTime = 0.0;

    frameCount++;

    double currentTime =
        (double)SDL_GetPerformanceCounter() /
        (double)SDL_GetPerformanceFrequency();

    if (currentTime - previousTime >= 1.0)
    {
        printf("FPS: %i\n", frameCount);

        frameCount = 0;
        previousTime = currentTime;
    }
    // endregion

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
    SDL_GL_DestroyContext(contextGL);
    SDL_DestroyWindow(window);
    SDL_Quit();
}