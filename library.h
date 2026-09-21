#ifndef ELIAS_SDL_GRAPHICS_LIBRARY_H
#define ELIAS_SDL_GRAPHICS_LIBRARY_H

#include <glad/glad.h>
#include <SDL3/SDL.h>

#define ARRAYCOUNT(arr) sizeof(arr)/sizeof(*(arr))


#define RED (vec3){1.0, 0.0, 0.0}
#define GREEN (vec3){0.0, 1.0, 0.0}
#define BLUE (vec3){0.0, 0.0, 1.0}
#define WHITE (vec3){1.0, 1.0, 1.0}
#define MAPGREEN1 (vec3){0.22, 0.67, 0.35}
#define MAPGREEN2 (vec3){0.14, 0.53, 0.24}


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

static OpenGL_Context context;

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


void DrawRectangle(vec2 pos, vec2 size, vec3 color);


#endif // ELIAS_SDL_GRAPHICS_LIBRARY_H
