#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include "falling_sand.h"
#include "text_renderer.h"

unsigned long long global_ticks = 0;
MOUSE_BUTTON mouse_button_input = NONE;
MATERIAL_TYPE selected_material = SAND;
physics_property *selected_phys = sand;
world_obj *world;
float current_time;

int RTX_ON = 0;
int screen_w = SCR_WIDTH, screen_h = SCR_HEIGHT;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
char text_updated [128] = {0};
char text_instr[] = "Left Mouse- generate materials, Right Mouse- remove materials.";
char text_mats [] = "1- SAND, 2- WATER, 3- ROCK, 4- LAVA, 5- LIGHT, R- toggle RTX MODE.";

int main()
{

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    Shader renderShader("./shaders/shader.vs", "./shaders/shader.fs"); // you can name your shader files however you like
    Shader drawShader("./shaders/draw_shader.vs", "./shaders/draw_shader.fs");

    TextRenderer  *Text;
    Text = new TextRenderer(SCR_WIDTH, SCR_HEIGHT);
    Text->Load("./fonts/OCRAEXT.TTF", 24);

    world = make_world (WRD_WIDTH, WRD_HEIGHT);

    float last_time = 0.0;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        current_time = glfwGetTime();
        float dt = current_time - last_time;
        last_time = current_time;
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        update_world (world);
        render_world (world, &renderShader, RTX_ON);
        draw_world (world, &drawShader, screen_w, screen_h);
        
        // Text updates
        static float text_update_time = 0.0f;
        if (current_time - text_update_time > 0.33)
        {
            text_update_time = current_time;
            sprintf (text_updated, "FPS: %2.3f, Material Selected: ", 1.0/dt);
            char *e_p = text_updated;
            while (*e_p != '\0')
                e_p++;
            switch (selected_material)
            {
            case SAND:
                sprintf (e_p, "SAND");
                break;
            case WATER:
                sprintf (e_p, "WATER");
                break;
            case ROCK:
                sprintf (e_p, "ROCK");
                break;
            case FIRE:
                sprintf (e_p, "LAVA");
                break;
            case LIGHT:
                sprintf (e_p, "LIGHT");
                break;
            default:
                break;
            }
        }
        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, screen_w, screen_h);
        Text->RenderText(std::string(text_instr), -0.0f, -0.0f, 1.0f
            , glm::vec3(0.6f, 0.4f, 0.75f), selected_phys->diffuse);
        Text->RenderText(std::string(text_mats), -0.0f, -0.06f, 1.0f
            , glm::vec3(0.6f, 0.4f, 0.75f), selected_phys->diffuse);
        Text->RenderText(std::string(text_updated), -0.0f, -0.12f, 1.0f
            , glm::vec3(0.6f, 0.4f, 0.75f), selected_phys->diffuse);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    /**********Fill in the blank*********/
    static float rtx_last_time = 0.0;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        if (current_time - rtx_last_time > 0.2)
        {
            rtx_last_time = current_time;
            RTX_ON ^= 1;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        selected_material = SAND;
        selected_phys = sand;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        selected_material = WATER;
        selected_phys = water;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        selected_material = ROCK;
        selected_phys = rock;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        selected_material = FIRE;
        selected_phys = fire;
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
    {
        selected_material = LIGHT;
        selected_phys = light;
    }
    mouse_button_input = NONE;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        mouse_button_input = LEFT;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        mouse_button_input = RIGHT;
    }
    /*************************************/
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (world)
        mouse_event (world, selected_material, mouse_button_input, screen_w, screen_h, xpos, ypos);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    screen_h = height;
    screen_w = width;
    glViewport(0, 0, width, height);
}
