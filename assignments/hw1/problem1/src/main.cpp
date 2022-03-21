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

#define _MAX_VERTICES 1024
#define _TICKS_PER_SECOND 100.0
#define _FLICKER_HALF_PERIOD_IN_TICKS (_TICKS_PER_SECOND/2.0)
#define _MOVEMENT_PER_TICK (0.5f/_TICKS_PER_SECOND)
#define _ROTATION_HALF_PERIOD_IN_TICKS (_TICKS_PER_SECOND/4.0)
#define _BLUE (0.0f), (0.0f), (1.0f)
#define _BLACK (0.0f), (0.0f), (0.0f)

unsigned long long global_ticks = 0;
int inputs[5] = {0};

typedef struct _vertex
{
    float abs_pos[3];
    float color[3];
    float d_pos[3];
} _vertex;

typedef struct _triangle
{
    float pos[3];
    float col[3];
    float vel[3];
    _vertex vertex[3];
    _vertex center_of_mass;
    float rot_degree;

    unsigned int VBO, VAO;
    Shader *shader;
} _triangle;

_triangle triangle;

void get_center_of_mass (_vertex* vertex_out, _vertex* vertex_ptr, size_t num)
{
    vertex_out->d_pos[0] = 0.0f;
    vertex_out->d_pos[1] = 0.0f;
    vertex_out->d_pos[2] = 0.0f;
    for (int i = 0; i < num; i++)
    {
        _vertex *vertex = vertex_ptr + i;
        vertex_out->d_pos[0] += vertex->d_pos[0];
        vertex_out->d_pos[1] += vertex->d_pos[1];
        vertex_out->d_pos[2] += vertex->d_pos[2];
    }
    vertex_out->d_pos[0] /= num;
    vertex_out->d_pos[1] /= num;
    vertex_out->d_pos[2] /= num;
}

void set_delta_position (_vertex* vertex, float pos0, float pos1, float pos2)
{
    vertex->d_pos[0] = pos0;
    vertex->d_pos[1] = pos1;
    vertex->d_pos[2] = pos2;
}
void set_abs_position (_vertex* vertex, float c_pos0, float c_pos1, float c_pos2)
{
    vertex->abs_pos[0] = vertex->d_pos[0] + c_pos0;
    vertex->abs_pos[1] = vertex->d_pos[1] + c_pos1;
    vertex->abs_pos[2] = vertex->d_pos[2] + c_pos2;
}
void set_color (_vertex* vertex, float col0, float col1, float col2)
{
    vertex->color[0] = col0;
    vertex->color[1] = col1;
    vertex->color[2] = col2;
}
void set_triangle_abs_position (_triangle* triangle)
{
    set_abs_position (&triangle->vertex[0], triangle->pos[0], triangle->pos[1], triangle->pos[2]);
    set_abs_position (&triangle->vertex[1], triangle->pos[0], triangle->pos[1], triangle->pos[2]);
    set_abs_position (&triangle->vertex[2], triangle->pos[0], triangle->pos[1], triangle->pos[2]);
    set_abs_position (&triangle->center_of_mass, triangle->pos[0], triangle->pos[1], triangle->pos[2]);
}
void set_triangle_position (_triangle* triangle, float pos0, float pos1, float pos2)
{
    triangle->pos[0] = pos0;
    triangle->pos[1] = pos1;
    triangle->pos[2] = pos2;
    get_center_of_mass (&triangle->center_of_mass, triangle->vertex, 3);
    set_triangle_abs_position (triangle);
}
void set_triangle_color (_triangle* triangle, float col0, float col1, float col2)
{
    triangle->col[0] = col0;
    triangle->col[1] = col1;
    triangle->col[2] = col2;
    set_color (&triangle->vertex[0], col0, col1, col2);
    set_color (&triangle->vertex[1], col0, col1, col2);
    set_color (&triangle->vertex[2], col0, col1, col2);
}
void set_triangle_velocity (_triangle* triangle, float d_x, float d_y, float d_z)
{
    triangle->vel[0] = d_x;
    triangle->vel[1] = d_y;
    triangle->vel[2] = d_z;
}
void set_triangle_rot_degree (_triangle* triangle, double degree)
{
    while (degree > 360)
    {
        degree -= 360;
    }
    while (degree < -360)
    {
        degree += 360;
    }
    triangle->rot_degree = degree;
}
void update_triangle_position (_triangle* triangle)
{
    set_triangle_position (triangle, 
        triangle->pos[0]+triangle->vel[0], triangle->pos[1]+triangle->vel[1], triangle->pos[2]+triangle->vel[2]);
}
void init_triangle (_triangle* triangle, Shader *shader)
{
    bzero (triangle, sizeof(_triangle));

    triangle->shader = shader;
    set_delta_position(&triangle->vertex[0], 0.0f, 0.866025f, 0.0f);
    set_delta_position(&triangle->vertex[1], -0.5f, 0.0f, 0.0f);
    set_delta_position(&triangle->vertex[2], 0.5f,  0.0f, 0.0f);
    set_triangle_position (triangle, 0.0, 0.0, 0.0);
    set_triangle_color (triangle, _BLUE);
    /**********Fill in the blank*********/
    glGenVertexArrays(1, &triangle->VAO);
    glGenBuffers(1, &triangle->VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(triangle->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, triangle->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle->vertex), triangle->vertex, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(_vertex), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(_vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
void draw_triangle(_triangle* triangle)
{
    update_triangle_position(triangle);

    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::translate(trans, glm::vec3(triangle->center_of_mass.abs_pos[0], triangle->center_of_mass.abs_pos[1], triangle->center_of_mass.abs_pos[2]));
    trans = glm::rotate(trans, glm::radians(triangle->rot_degree), glm::vec3(0.0, 0.0, 1.0));
    trans = glm::translate(trans, glm::vec3(-triangle->center_of_mass.abs_pos[0], -triangle->center_of_mass.abs_pos[1], -triangle->center_of_mass.abs_pos[2]));
    
    unsigned int transformLoc = glGetUniformLocation(triangle->shader->ID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle->vertex), triangle->vertex, GL_STATIC_DRAW);
    glBindVertexArray(triangle->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
void delete_triangle(_triangle* triangle)
{
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &triangle->VAO);
    glDeleteBuffers(1, &triangle->VBO);

}

void move (_triangle* triangle)
{
    static unsigned long long last_tick = 0;
    float d_x = 0.0f, d_y = 0.0f, d_z = 0.0f;
    if (inputs[0] == 1)
    {
        d_y = _MOVEMENT_PER_TICK;
    }
    if (inputs[1] == 1)
    {
        d_x = -_MOVEMENT_PER_TICK;
    }
    if (inputs[2] == 1)
    {
        d_y = -_MOVEMENT_PER_TICK;
    }
    if (inputs[3] == 1)
    {
        d_x = _MOVEMENT_PER_TICK;
    }
    if (inputs[4] == 1 && global_ticks > last_tick + _ROTATION_HALF_PERIOD_IN_TICKS)
    {
        set_triangle_rot_degree (triangle, triangle->rot_degree + 90);
        last_tick = global_ticks;
    }
    if (d_x != 0.0f && d_y != 0.0f)
    {
        d_x /- 1.4142135;
        d_y /- 1.4142135;
    }
    set_triangle_velocity (triangle, d_x, d_y, d_z);
}
void flicker (_triangle *triangle)
{
    static unsigned long long last_tick = 0;
    static int flag = 0;
    if (global_ticks > last_tick + _FLICKER_HALF_PERIOD_IN_TICKS)
    {
        if (flag == 0)
        {
            flag = 1;
            set_triangle_color (triangle, _BLACK);
        }
        else
        {
            flag = 0;
            set_triangle_color (triangle, _BLUE);
        }
        last_tick = global_ticks;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

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

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("./shaders/shader.vs", "./shaders/shader.fs"); // you can name your shader files however you like

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    init_triangle (&triangle, &ourShader);

    /*************************************/

    float last_time = -INFINITY;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // render the triangle
        ourShader.use();
        float timeValue = glfwGetTime();
        
        /**********Fill in the blank*********/

        if (timeValue - last_time > (1.0f / _TICKS_PER_SECOND)) 
        {
            global_ticks++;
            // printf ("Tick %lld. \n", global_ticks);
            flicker (&triangle);
            move (&triangle);
            draw_triangle (&triangle);
            last_time = timeValue;
        }

        
        /*************************************/

        

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete_triangle (&triangle);

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
    bzero (inputs, sizeof(inputs));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        inputs[0] = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        inputs[1] = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        inputs[2] = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        inputs[3] = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        inputs[4] = 1;
    }
    /*************************************/
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

