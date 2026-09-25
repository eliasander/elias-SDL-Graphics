#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <glad/glad.h>
#include <SDL3/SDL.h>

#include <eos_core.h>

#define BASE_WIDTH 1920
#define BASE_HEIGHT 1080

#define MAP_SIZE (vec2){10,9}
#define TILE_SIZE 80

#define MAP_BORDER (Color){122, 81, 61, 1.0}

#define MAPGREEN1 (Color){67, 160, 71, 1.0}
#define MAPGREEN2 (Color){69, 170, 85, 1.0}
#define BACKGROUND_COLOR (Color){73, 154, 213, 1.0}
#define SNAKE_COLOR (Color){27, 118, 255, 1.0};
#define APPLE_COLOR (Color){170, 50, 50, 1.0};

#define SNAKE_START_LENGTH 3
#define SNAKE_MAX_LENGTH 128
#define SNAKE_MOVE_DELAY 250 // ms
#define SNAKE_EFFECT_DARKEN true

#define FOOD_MAX_AMOUNT 5
#define SUBGAME_MAX_DEPTH 3


typedef struct {
    // Position on board, gos from -MAP_SIZE/2 to MAP_SIZE/2
    vec2 pos;

    // Direction
    // 0 - Up
    // 1 - Right
    // 2 - Down
    // 3 - Left
    int dir;
} SnakePart;

typedef struct {
    int length;
    SnakePart part[SNAKE_MAX_LENGTH];
    SnakePart *head;
    
} Snake;

typedef struct {
    bool exist;
} Food;




typedef struct Subgame {
    // The current depth
    int depth;
    // Pointers to the next depth of subgames

    // 0 = apple, 1 = subgame
    int type[FOOD_MAX_AMOUNT];
    bool exist[FOOD_MAX_AMOUNT];
    vec2 foodPos[FOOD_MAX_AMOUNT];
    int foodCount;
    vec2 pos;

    struct Subgame *parent;
    struct Subgame *subgame;

    // Snake for the subgame
    Snake snake;
} Subgame;

Subgame mainGame;
Subgame *currentGame;
int subGameCount;
bool dead = false;
bool block_snake_input = false;
bool exit_current_subgame = false;

Subgame CreateSubgame(int depth);
void DeleteSubgame(Subgame *game);


long long current_time_ms(void) {
    #ifdef _WIN32
    #include <windows.h>

    FILETIME ft;    
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    // FILETIME is 100-ns intervals since 1601; convert to Unix ms epoch
    return (long long)(ull.QuadPart / 10000ULL - 11644473600000ULL);

    #else
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

    #endif
}


void snakeInit(Snake *snake) {
    srand((unsigned)time(NULL)*10000);

    snake->length = SNAKE_START_LENGTH;
    for (int i = 0; i < SNAKE_MAX_LENGTH; i++) {
        snake->part[i].pos = (vec2){roundf(MAP_SIZE.x/2), roundf(MAP_SIZE.y/2)};
        snake->part[i].dir = -1; // Uninitalized direction
    };
    snake->head = &snake->part[0];
    snake->head->dir = 1;

}

void snakeTick(Snake *snake) {
    Snake next_snake = *snake;
    next_snake.head = &next_snake.part[0];

    long long snake_last_tick = current_time_ms();

    for (int i = 0; i < next_snake.length; i++) {
        if (i > 0 && i < 128) {
            next_snake.part[i] = snake->part[i-1];
        }
        else if (i == 0) {
            switch (next_snake.head->dir) {
                case 0:
                    next_snake.head->pos.y--;
                    break;
                case 1:
                    next_snake.head->pos.x++;
                    break;
                case 2:
                    next_snake.head->pos.y++;
                    break;
                case 3:
                    next_snake.head->pos.x--;
                    break;
                default:
                    break;
            }

            for (int j = 0; j < FOOD_MAX_AMOUNT; j++) {
                if (currentGame->exist[j] == false)
                    continue;

                if (next_snake.head->pos.x == currentGame->foodPos[j].x && 
                    next_snake.head->pos.y == currentGame->foodPos[j].y) {
                    if (currentGame->depth == SUBGAME_MAX_DEPTH) {
                        next_snake.part[next_snake.length] = snake->part[snake->length];
                        currentGame->exist[j] = false;
                        currentGame->foodCount--;
                        next_snake.length++;
                    }
                    else {
                        Subgame game = CreateSubgame(currentGame->depth+1);
                        game.pos = currentGame->foodPos[j];
                        game.parent = currentGame;
                        currentGame->subgame[j] = game;
                        currentGame = &currentGame->subgame[j];
                    }
                }
            }

            bool reset_death = false;

            for (int j = 1; j < next_snake.length; j++) {
                if (next_snake.head->pos.x == next_snake.part[j].pos.x && next_snake.head->pos.y == next_snake.part[j].pos.y) {
                    if (currentGame->depth != 0)
                        exit_current_subgame = true;
                    else
                        dead = true;
                }
                else if (next_snake.head->pos.x == -1 || next_snake.head->pos.x == MAP_SIZE.x || next_snake.head->pos.y == -1 || next_snake.head->pos.y == MAP_SIZE.y ) {
                    if (currentGame->depth != 0)
                        exit_current_subgame = true;
                    else
                        dead = true;
                }
            }
        }
    }
    
    block_snake_input = false;
    if (!dead) {
        memcpy(snake->part, next_snake.part, sizeof(SnakePart)*SNAKE_MAX_LENGTH);
        snake->head = &snake->part[0];
        snake->length = next_snake.length;
    }
    return;
}

void spawnFood() {
    int x;
    int y;
    int depth = currentGame->depth;

    bool cell_free;
    do {
        x = rand() % (int)MAP_SIZE.x;
        y = rand() % (int)MAP_SIZE.y;

        cell_free = true;
        for (int i = 0; i < currentGame->snake.length; i++) {
            if (x == currentGame->snake.part[i].pos.x && y == currentGame->snake.part[i].pos.y) {
                cell_free = false;
                break;
            }
        }
        for (int i = 0; i < currentGame->foodCount; i++)  {
            if (currentGame->exist[i]) {
                if (x == currentGame->foodPos[i].x && y == currentGame->foodPos[i].y) {
                    cell_free = false;
                    break;
                }
            }
        }
        
        if(cell_free == false)
            break;
    } while (!cell_free);

    if (cell_free == false)
        return;

    int i = -1;
    for (int j = 0; j < FOOD_MAX_AMOUNT; j++) {
        if (currentGame->exist[j] == false) {
            i = j;
            currentGame->exist[j] = true;
            break;
        }
    }

    if (i == -1)
        return;

    currentGame->foodPos[i].x = x;
    currentGame->foodPos[i].y = y;
    currentGame->foodCount++;
}

Subgame CreateSubgame(int depth){
    Subgame game;
    game.depth = depth;
    game.foodCount = 0;
    if (game.depth < SUBGAME_MAX_DEPTH) {
        game.subgame = malloc(sizeof *game.subgame * FOOD_MAX_AMOUNT);
    } 
    else {
        for (int i = 0; i < FOOD_MAX_AMOUNT; i++) {
            game.exist[i] = false;
        }
    }

    snakeInit(&game.snake);

    return game;
}

void DeleteSubgame(Subgame *game) {
    Subgame *parent = game->parent;
    int x,y;
    x = game->pos.x;
    y = game->pos.y;

    bool found;
    int i = -1;
    do {
        i++;
        if (parent->foodPos[i].x == x &&
            parent->foodPos[i].y == y) {

            found = true;
        }
    } while (!found);

    if (i == -1) {
        printf("Something went wrong, couldn't find subgame for deletion\n");
        return;
    }

    parent->subgame[i] = (Subgame){0};
    parent->exist[i] = false;
    currentGame->foodCount--;
    
}

bool HandleEvents(SDL_Window *window, SDL_Event event)
{
    static bool vsync = true;
    static bool wired = false;
    static bool fullscreen = false;
    switch (event.type)
    {
        case SDL_EVENT_QUIT:
            WindowShouldClose = true;
            return false;

        case SDL_EVENT_KEY_DOWN:
            if (event.key.repeat)
                break;

            int *dir = &currentGame->snake.head->dir;

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

                case SDLK_R:
                    if (dead) {
                        snakeInit(&mainGame.snake);
                        currentGame = &mainGame;
                        dead = false;
                    }
                    break;

                case SDLK_F:
                    SDL_SetWindowFullscreen(window, fullscreen);
                    fullscreen = !fullscreen;
                    break;

                case SDLK_W:
                    if (*dir != 2 && !block_snake_input) {
                        *dir = 0;
                        block_snake_input = true;
                    }
                    break;
                case SDLK_D:
                    if (*dir != 3 && !block_snake_input) {
                        *dir = 1;
                        block_snake_input = true;
                    }
                    break;
                case SDLK_S:
                    if (*dir != 0 && !block_snake_input) {
                        *dir = 2;
                        block_snake_input = true;
                    }
                    break;
                case SDLK_A:
                    if (*dir != 1 && !block_snake_input) {
                        *dir = 3;
                        block_snake_input = true;
                    }
                    break;
            }

            break;

        default:
            break;
    }

    return true; // keep running
}

int main() {

    SDL_Window* window = CreateWindow();
    SDL_GLContext contextGL = CreateOpenGLContext();

    SetWindowFlag(EOS_WINDOW_LETTERBOXING);
    SetWindowFlag(EOS_WINDOW_LETTERBOXING_COLOR_CLEAR);

    int w,h;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    compute_viewport(w, h, BASE_WIDTH, BASE_HEIGHT);

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

    mainGame = CreateSubgame(0);
    currentGame = &mainGame;    




    spawnFood();

    long long snake_last_tick = current_time_ms();
    while (!WindowShouldClose) {
        StartFrame(window);
        context.rectCount = 0;

        // Render here
        Clear(BACKGROUND_COLOR);

        vec2 tileOffset = {
            (float)BASE_WIDTH/2-(MAP_SIZE.x)*TILE_SIZE/2,
            (float)BASE_HEIGHT/2-(MAP_SIZE.y)*TILE_SIZE/2

        };

        for (int y = -1; y < MAP_SIZE.y+1; y++)
        {
            for (int x = -1; x < MAP_SIZE.x+1; x++)
            {
                Color color;
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
                    tileOffset.y+(float)y*(float)TILE_SIZE
                },
                (vec2){
                    (float)TILE_SIZE,
                    (float)TILE_SIZE
                },
                color);
            }
        }

        // Snake
        long long now = current_time_ms();
        if (now - snake_last_tick > SNAKE_MOVE_DELAY && !dead) {
            snakeTick(&currentGame->snake);
            if (dead) {
                printf("Dead\n");
            }
            snake_last_tick = now;
        }

        for (int i = 0; i < currentGame->snake.length; i++) {
            Color color = SNAKE_COLOR;
            if (SNAKE_EFFECT_DARKEN)
                color = darken(color, 1.0-0.01*i);


            DrawRectangle(
                (vec2){
                    tileOffset.x+currentGame->snake.part[i].pos.x*(float)TILE_SIZE,
                    tileOffset.y+currentGame->snake.part[i].pos.y*(float)TILE_SIZE
                },
                (vec2){
                    (float)TILE_SIZE,
                    (float)TILE_SIZE
                },
                color
            );
        }


        if (currentGame->foodCount < FOOD_MAX_AMOUNT)
            spawnFood();

        for (int i = 0; i < FOOD_MAX_AMOUNT; i++) {
            if (currentGame->exist[i] == false)
                continue;
            Color color;
            if (currentGame->depth == SUBGAME_MAX_DEPTH) {
                color = APPLE_COLOR;
                DrawRectangle(
                (vec2){
                    floorf(tileOffset.x+currentGame->foodPos[i].x*(float)TILE_SIZE),
                    floorf(tileOffset.y+currentGame->foodPos[i].y*(float)TILE_SIZE)
                },
                (vec2){
                    (float)TILE_SIZE,
                    (float)TILE_SIZE
                },
                color
                );
            } 
            else {
                #define SUBGAME_VISUAL_SIZE 6
                for (int y = 0; y < SUBGAME_VISUAL_SIZE; y++)
                {
                    for (int x = 0; x < SUBGAME_VISUAL_SIZE; x++)
                    {
                        Color color;
                        if ((x+y) % 2 == 0)
                            color = MAPGREEN1;
                        else
                            color = MAPGREEN2;


                        color = darken(color, 0.85);

                        DrawRectangle(
                        (vec2){
                            tileOffset.x+currentGame->foodPos[i].x*(float)TILE_SIZE+((float)TILE_SIZE/SUBGAME_VISUAL_SIZE)*x,
                            tileOffset.y+currentGame->foodPos[i].y*(float)TILE_SIZE+((float)TILE_SIZE/SUBGAME_VISUAL_SIZE)*y
                        },
                        (vec2){
                            (float)TILE_SIZE/SUBGAME_VISUAL_SIZE,
                            (float)TILE_SIZE/SUBGAME_VISUAL_SIZE
                        },
                        color);
                    }
                }
            }
        }

        // Exit subgame if needed
        if (exit_current_subgame)  {
            Subgame *parent = currentGame->parent;

            DeleteSubgame(currentGame);
            
            currentGame = parent;
            currentGame->foodCount--;
            currentGame->snake.length--;
            exit_current_subgame = false;

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