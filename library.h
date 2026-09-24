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

// FLAGS

#define EOS_WINDOW_LETTERBOXING (1ULL << 0)

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

typedef struct { 
    float x, y, w, h, scale; 
} Viewport;


typedef struct
{
    unsigned long long flags[32];
    GLuint VBO;
    GLuint VAO;
    GLuint EBO;
    GLuint shaderProgram;
    Vertex vertices[1024*4];
    unsigned int indices[1024*6];
    unsigned int rectCount;
} OpenGL_Context;

extern OpenGL_Context context;



// Helper functions

// Value 0.0-1.0, if set to 0.8 then it is darkened to 80% of what the original is.
vec4 darken(vec4 color, float value);
void SetWindowFlag(unsigned long long flag);






typedef bool (*EventCallback)(SDL_Window *window, SDL_Event event);
bool setEventCallback(SDL_Window *window, EventCallback callback);

// 2. Global storage for the callback function pointer
extern EventCallback g_eventCallback;




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


float check_scale(float height, float width, float base_width, float base_height);
Viewport compute_viewport(float win_w, float win_h, float game_w, float game_h);
#endif // ELIAS_SDL_GRAPHICS_LIBRARY_H