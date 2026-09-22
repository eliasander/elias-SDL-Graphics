#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>

#include <glad/glad.h>
#include <SDL3/SDL.h>

#include "library.h"

#define BASE_WIDTH 256
#define BASE_HEIGHT 144

bool HandleEvents(SDL_Window *window)
{
    static bool vsync = true;
    static bool wired = false;
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_QUIT:
                return false; // signal "stop running"

            case SDL_EVENT_KEY_DOWN:
                if (event.key.repeat)
                    break;

                switch (event.key.key)
                {
                    case SDLK_ESCAPE:
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

                /*
                glViewport(0, 0, width, height);
                screenWidth = width;
                screenHeight = height;

                check_scale((float)height, (float)width, BASE_WIDTH, BASE_HEIGHT);
                */
            } break;
        }
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

    bool running = true;

    while (running) {
        running = HandleEvents(window);
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
    }

    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(contextGL);
    SDL_Quit();
}