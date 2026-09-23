#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>

#include <glad/glad.h>
#include <SDL3/SDL.h>

#include "library.h"

#define BASE_WIDTH 1920
#define BASE_HEIGHT 1080

#define MAP_SIZE (vec2){17,15}
#define TILE_SIZE 40

#define MAP_BORDER (vec4){124, 119, 62, 1.0}
#define MAPGREEN1 (vec4){67, 160, 71, 1.0}
#define MAPGREEN2 (vec4){69, 170, 85, 1.0}
#define BACKGROUND_COLOR (vec4){73, 154, 213, 1.0}
#define SNAKE_COLOR (vec4){27, 118, 255, 1.0};
#define APPLE_COLOR (vec4){217, 0, 0, 1.0};




bool HandleEvents(SDL_Window *window, SDL_Event event)
{
    static bool vsync = true;
    static bool wired = false;
    switch (event.type)
    {
        case SDL_EVENT_QUIT:
            WindowShouldClose = true;
            return false;

        case SDL_EVENT_KEY_DOWN:
            if (event.key.repeat)
                break;

            switch (event.key.key)
            {
                case SDLK_ESCAPE:
                    WindowShouldClose = true;
                    return false;

                case SDLK_E:
                    vsync = !vsync;
                    SDL_GL_SetSwapInterval(vsync ? 1 : 0);
                    break;

                case SDLK_SPACE:
                    wired = !wired;
                    glPolygonMode(GL_FRONT_AND_BACK, wired ? GL_LINE : GL_FILL);
                    break;
            }
            break;

        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        {
            int width, height;
            SDL_GetWindowSizeInPixels(window, &width, &height);

            glViewport(0, 0, width, height);
            check_scale((float)height, (float)width, BASE_WIDTH, BASE_HEIGHT);
        } break;
    }

    return true; // keep running
}

int main() {

    SDL_Window* window = CreateWindow();
    SDL_GLContext contextGL = CreateOpenGLContext();

    Shader vertexShader =
        CreateShader(GL_VERTEX_SHADER, "shaders/basic.vert");
    Shader fragmentShader =
        CreateShader(GL_FRAGMENT_SHADER, "shaders/basic.frag");

    // Create Shader Program
    Shader shaders[2] = {
        vertexShader,
        fragmentShader
    };

    setEventCallback(window, HandleEvents);

    CreateShaderProgram(shaders, ARRAYCOUNT(shaders));

    // Delete shaders - these aren't needed anymore since they exist within the program
    DeleteShader(vertexShader);
    DeleteShader(fragmentShader);

    // Generate buffers
    glGenVertexArrays(1, &context.VAO);
    glGenBuffers(1, &context.VBO);
    glGenBuffers(1, &context.EBO);

    // Bind VAO FIRST so EBO/VBO bindings are captured inside it
    glBindVertexArray(context.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, context.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(context.vertices), NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(context.indices), NULL, GL_DYNAMIC_DRAW);

    glBindVertexArray(0); // Unbind

    // Enable vsync
    SDL_GL_SetSwapInterval(1);



    while (!WindowShouldClose) {
        StartFrame(window);
        context.rectCount = 0;

        // Render here
        Clear(BACKGROUND_COLOR);

        vec2 tileOffset = {
            BASE_WIDTH/2-(MAP_SIZE.x+2)*TILE_SIZE/2,
            BASE_HEIGHT/2-(MAP_SIZE.y+2)*TILE_SIZE/2

        };

        for (int y = -1; y < MAP_SIZE.y+1; y++)
        {
            for (int x = -1; x < MAP_SIZE.x+1; x++)
            {
                vec4 color;
                if ((x == -1 || x == MAP_SIZE.x) || (y == -1 || y == MAP_SIZE.y)) {
                    color = MAP_BORDER;
                }
                else {
                    if ((x+y) % 2 == 0)
                        color = MAPGREEN1;
                    else
                        color = MAPGREEN2;
                }
            

                DrawRectangle(
                (vec2){
                    tileOffset.x+(float)x*(float)TILE_SIZE,
                    tileOffset.y+(float)y*(float)TILE_SIZE},
                (vec2){
                    (float)TILE_SIZE,
                    (float)TILE_SIZE },
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

        // Position attribute: 2 floats
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
            4,                  // vec4
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

        EndFrame(window);

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
    }

    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(contextGL);
    SDL_Quit();
}