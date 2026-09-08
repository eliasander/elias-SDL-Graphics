
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <glad/glad.h>

#include "GLFW/glfw3.h"

#define BASE_WIDTH 256
#define BASE_HEIGHT 144

typedef struct
{
    float x,y;
} vec2;

typedef struct
{
    float x,y,z;
} vec3;

#define RED (vec3){1.0, 0.0, 0.0}
#define GREEN (vec3){0.22, 0.67, 0.35}
#define DARKGREEN (vec3){0.14, 0.53, 0.24}
#define BLUE (vec3){0.0, 0.0, 1.0}
#define WHITE (vec3){1.0, 1.0, 1.0}

unsigned int VBO;
unsigned int VAO;
unsigned int EBO;
unsigned int shaderProgram;
int screenWidth, screenHeight = 0;
float scale;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"

    "out vec3 fColor;\n"

    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
    "   fColor = aColor;\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"

    "in vec3 fColor;\n"

    "void main()\n"
    "{\n"
    "    FragColor = vec4(fColor, 1.0);\n"
    "}\0";

void DrawTriangle(vec2 pos, vec2 size)
{
    pos.x *= scale;
    pos.y *= scale;

    size.x *= scale;
    size.y *= scale;

    float x = (pos.x / screenWidth) * 2.0f - 1.0f;
    float y = 1.0f - (pos.y / screenHeight) * 2.0f;

    float width  = (size.x / screenWidth) * 2.0f;
    float height = (size.y / screenHeight) * 2.0f;

    float vertices2[] = {
        x,         y,          0.0f,  // top-left
        x + width, y,          0.0f,  // top-right
        x + width, y - height, 0.0f  // bottom-right
    };
    // 0: Bind Vertex Buffer Object and copy data into buffer data
    // 1. bind Vertex Array Object
    glBindVertexArray(VAO);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices2),
        vertices2,
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
    glUseProgram(shaderProgram);

    // 3: Render our objects

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void DrawRectangle(vec2 pos, vec2 size, vec3 color)
{
    pos.x *= scale;
    pos.y *= scale;

    size.x *= scale;
    size.y *= scale;

    float x = (pos.x / screenWidth) * 2.0f - 1.0f;
    float y = 1.0f - (pos.y / screenHeight) * 2.0f;

    float width  = (size.x / screenWidth) * 2.0f;
    float height = (size.y / screenHeight) * 2.0f;

    float vertices[] = {
        x,         y,          color.x, color.y, color.z,
        x + width, y,          color.x, color.y, color.z,
        x + width, y - height, color.x, color.y, color.z,
        x,         y - height, color.x, color.y, color.z
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    // Bind VAO FIRST
    glBindVertexArray(VAO);

    // Upload vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_DYNAMIC_DRAW
    );

    // Upload indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(indices),
        indices,
        GL_DYNAMIC_DRAW
    );

    // Position attribute: 3 floats
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,                  // location
        3,                  // vec3
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(float),  // stride
        (void*)(2 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    // Shader
    glUseProgram(shaderProgram);

    // Draw rectangle (2 triangles = 6 indices)
    glDrawElements(
        GL_TRIANGLES,
        6,
        GL_UNSIGNED_INT,
        0
    );
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
    check_scale(height, width);
}

void processInput(GLFWwindow* window, int key, int scancode, int action, int mods)
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


int main(void)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


    GLFWwindow* window = glfwCreateWindow(256, 144, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    // Set current context
    glfwMakeContextCurrent(window);

    // Initalize glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    // Set window resize event to framebuffer_size_callback
    glfwGetWindowSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
    check_scale(screenHeight, screenWidth);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetKeyCallback(window, processInput);

    // Compile shaders
    // Compile vertex shader
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("VERTEX SHADER ERROR:\n%s\n", infoLog);
    }

    // Compile fragment shader
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("FRAGMENT SHADER ERROR:\n%s\n", infoLog);
    }

    // Create Shader Program
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("SHADER PROGRAM LINK ERROR:\n%s\n", infoLog);
    }

    // Delete shaders - these aren't needed anymore
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // Create Vertex Buffer Object
    glGenBuffers(1, &VBO);

    // Create Vertex Array Object
    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &EBO);

    double previousTime = glfwGetTime();
    int frameCount = 0;


    while(!glfwWindowShouldClose(window))
    {

        // Render here
        glClear(GL_COLOR_BUFFER_BIT);
        int tileSize = 6;
        for (int y = 0; y < 15; y++)
        {
            for (int x = 0; x < 17; x++)
            {
                vec3 color;
                if ((x+y) % 2 == 0)
                    color = GREEN;
                else
                    color = DARKGREEN;
                DrawRectangle((vec2){10+x*tileSize,10+y*tileSize}, (vec2){tileSize,tileSize}, color);
            }
        }

        // FPS calculations
        frameCount++;

        double currentTime = glfwGetTime();

        if (currentTime - previousTime >= 1.0)
        {
            char title[64];
            sprintf(title, "My OpenGL Game - FPS: %d", frameCount);

            printf(title);

            frameCount = 0;
            previousTime = currentTime;
        }



        // Swap the buffers, then check and call events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Terminate glfw
    glfwTerminate();
    return 0;
}
