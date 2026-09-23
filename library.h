#ifndef ELIAS_SDL_GRAPHICS_LIBRARY_H
#define ELIAS_SDL_GRAPHICS_LIBRARY_H

#include <glad/glad.h>
#include <SDL3/SDL.h>

#define ARRAYCOUNT(arr) sizeof(arr)/sizeof(*(arr))
extern bool WindowShouldClose;


#define RED (vec3){255, 0, 0}
#define GREEN (vec3){0, 255, 0}
#define BLUE (vec3){0, 0, 255}
#define WHITE (vec3){255, 255, 255}



typedef bool (*EventCallback)(SDL_Window *window, SDL_Event event);
bool setEventCallback(SDL_Window *window, EventCallback callback);

// 2. Global storage for the callback function pointer
extern EventCallback g_eventCallback;


typedef struct
{
    float x,y;
} vec2;

typedef struct
{
    float x,y,z;
} vec3;

typedef struct {
    float x,y,z,w;
} vec4;

typedef struct {
    vec2 pos;
    vec4 color;
} Vertex;

typedef struct {
    vec2 pos;
    vec2 size;
    vec4 color;
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

extern OpenGL_Context context;

SDL_Window* CreateWindow();
SDL_GLContext CreateOpenGLContext();

// GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
// GLuint containing shader
typedef struct {
    int type;
    GLuint shader;
} Shader;

// GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
Shader CreateShader(int type, const char *path);
int CreateShaderProgram(Shader shaders[], int shaderCount);

void DeleteShader(Shader shader);


void DrawRectangle(vec2 pos, vec2 size, vec4 color);

void StartFrame(SDL_Window* window);
void Clear(vec4 color);

void EndFrame(SDL_Window* window);


#endif // ELIAS_SDL_GRAPHICS_LIBRARY_H

void check_scale(float height, float width, float base_width, float base_height);