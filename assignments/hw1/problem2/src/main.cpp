#define GLM_ENABLE_EXPERIMENTAL
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))  
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))  
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "gameobjects.h"
#include "geometry_primitives.h"
#include <iostream>
#include <vector>
#include "text_renderer.h"

#define _MAX_TRIANGLES 1024
#define _POINT_MAX 20
#define _POINT_TRIANGLE_DIST (0.08f)
#define _POINT_TRIANGLE_POS (-0.8f)
#define _POINT_TRIANGLE_SCALE (0.07f)
#define _FPS 100.0
#define _TICKS_PER_SECOND 4.0
#define _MOVE_SPEED (0.8f)
#define _BLUE (0.0f), (0.0f), (1.0f)
#define _RED (1.0f), (0.0f), (0.0f)
#define _BLACK (0.0f), (0.0f), (0.0f)

double global_time = 0;
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
    float rot_vel;
    float scale;
    float last_update_time;
    unsigned int VBO, VAO;
    Shader *shader;
} _triangle;

typedef struct _quad
{
    float pos[3];
    float col[3];
    float vel[3];
    float length, height;
    _triangle triangle[2];
} _quad;

int points = 0;
int triangle_idx = 0;

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
        triangle->pos[0]+triangle->vel[0]*(1.0/_FPS), triangle->pos[1]+triangle->vel[1]*(1.0/_FPS), triangle->pos[2]+triangle->vel[2]*(1.0/_FPS));
    set_triangle_rot_degree (triangle, triangle->rot_degree + triangle->rot_vel*(1.0/_FPS));
}
void init_triangle (_triangle* triangle, Shader *shader)
{
    bzero (triangle, sizeof(_triangle));

    triangle->shader = shader;
    triangle->scale = 1.0f;
    set_delta_position(&triangle->vertex[0], -0.5f, -0.5f, 0.0f);
    set_delta_position(&triangle->vertex[1], 0.5f, -0.5f, 0.0f);
    set_delta_position(&triangle->vertex[2], 0.0f,  0.5f, 0.0f);
    set_color (&triangle->vertex[0], 0.0f, 1.0f, 0.0f);
    set_color (&triangle->vertex[1], 1.0f, 0.0f, 0.0f);
    set_color (&triangle->vertex[2], 0.0f, 0.0f, 1.0f);
    set_triangle_position (triangle, 0.0, 3.0, 0.0);
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
    if (global_time > triangle->last_update_time + (1.0/_FPS))
    {
        update_triangle_position(triangle);
        triangle->last_update_time = global_time;
    }
    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::translate(trans, glm::vec3(triangle->center_of_mass.abs_pos[0], triangle->center_of_mass.abs_pos[1], triangle->center_of_mass.abs_pos[2]));
    trans = glm::scale(trans, glm::vec3(triangle->scale, triangle->scale, triangle->scale));  
    trans = glm::rotate(trans, glm::radians(triangle->rot_degree), glm::vec3(0.0, 0.0, 1.0));
    trans = glm::translate(trans, glm::vec3(-triangle->center_of_mass.abs_pos[0], -triangle->center_of_mass.abs_pos[1], -triangle->center_of_mass.abs_pos[2]));
    
    unsigned int transformLoc = glGetUniformLocation(triangle->shader->ID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

    glBindVertexArray(triangle->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, triangle->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle->vertex), triangle->vertex, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
void delete_triangle(_triangle* triangle)
{
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &triangle->VAO);
    glDeleteBuffers(1, &triangle->VBO);

}
void start_triangle (_triangle* triangle)
{
    triangle->vel[1] = -getRandomValueBetween(0.1f, 0.4f);
    triangle->rot_degree = getRandomValueBetween(0.1f*180, 3*180);
    triangle->rot_vel = getRandomValueBetween(0.1f*180, 0.4f*180);
    triangle->scale = getRandomValueBetween(0.04f, 0.1f);
    triangle->pos[0] = getRandomValueBetween(-0.9f, 0.9f);
    triangle->pos[1] = 1.0f;
    update_triangle_position(triangle);
}
void reset_triangle (_triangle* triangle)
{
    if (triangle->center_of_mass.abs_pos[1] < -1.0f)
    {
        triangle->vel[1] = 0.0f;
        triangle->rot_degree = 0.0f;
        triangle->rot_vel = 0.0f;
        triangle->scale = 1.0f;
        triangle->pos[0] = 0.0f;
        triangle->pos[1] = 3.0f;
        update_triangle_position(triangle);
    }
}

void set_quad_position (_quad* quad, float pos0, float pos1, float pos2)
{
    quad->pos[0] = pos0;
    quad->pos[1] = pos1;
    quad->pos[2] = pos2;
    set_triangle_position (&quad->triangle[0], pos0, pos1, pos2);
    set_triangle_position (&quad->triangle[1], pos0, pos1, pos2);
}
void set_quad_color (_quad* quad, float col0, float col1, float col2)
{
    quad->col[0] = col0;
    quad->col[1] = col1;
    quad->col[2] = col2;
    set_triangle_color (&quad->triangle[0], col0, col1, col2);
    set_triangle_color (&quad->triangle[1], col0, col1, col2);
}
void set_quad_velocity (_quad* quad, float d_x, float d_y, float d_z)
{
    quad->vel[0] = d_x;
    quad->vel[1] = d_y;
    quad->vel[2] = d_z;
    set_triangle_velocity (&quad->triangle[0], d_x, d_y, d_z);
    set_triangle_velocity (&quad->triangle[1], d_x, d_y, d_z);
}
void set_quad_length_and_height (_quad *quad, float length, float height)
{
    quad->length = length;
    quad->height = height;
    set_delta_position(&quad->triangle[0].vertex[0], -length/2, -height/2, 0.0f);
    set_delta_position(&quad->triangle[0].vertex[1], length/2, -height/2, 0.0f);
    set_delta_position(&quad->triangle[0].vertex[2], -length/2,  height/2, 0.0f);
    set_delta_position(&quad->triangle[1].vertex[0], -length/2,  height/2, 0.0f);
    set_delta_position(&quad->triangle[1].vertex[1], length/2, -height/2, 0.0f);
    set_delta_position(&quad->triangle[1].vertex[2], length/2,  height/2, 0.0f);
}
void update_quad_position (_quad* quad)
{
    set_quad_position (quad, 
        quad->pos[0]+quad->vel[0]*(1.0/_FPS), quad->pos[1]+quad->vel[1]*(1.0/_FPS), quad->pos[2]+quad->vel[2]*(1.0/_FPS));
    // printf ("Quad pos : %3.3f, %3.3f, Quad vel : %3.3f, %3.3f\n", quad->pos[0], quad->pos[1], quad->vel[0], quad->vel[1]);
}
void init_quad (_quad* quad, Shader *shader)
{
    bzero (quad, sizeof(_quad));
    init_triangle (&quad->triangle[0], shader);
    init_triangle (&quad->triangle[1], shader);
    set_quad_length_and_height (quad, 1.0, 1.0);
    set_quad_color (quad, _RED);
    set_quad_position (quad, 0.0, 0.0, 0.0);
}
void draw_quad(_quad* quad)
{
    update_quad_position (quad);
    draw_triangle (&quad->triangle[0]);
    draw_triangle (&quad->triangle[1]);
}
void delete_quad(_quad* quad)
{
    delete_triangle (&quad->triangle[0]);
    delete_triangle (&quad->triangle[1]);
}
void collide (_quad *quad, _triangle *triangle)
{
    float t_x = triangle->center_of_mass.abs_pos[0];
    float t_y = triangle->center_of_mass.abs_pos[1];
    if ((quad->pos[0] - quad->length/2 <= t_x && t_x < quad->pos[0] + quad->length/2) &&
        (quad->pos[1] - quad->height/2 <= t_y && t_y < quad->pos[1] + quad->height/2))
    {
        triangle->vel[1] = 0.0f;
        triangle->rot_degree = 0.0f;
        triangle->rot_vel = 0.0f;
        triangle->scale = 1.0f;
        triangle->pos[0] = 0.0f;
        triangle->pos[1] = 3.0f;
        update_triangle_position(triangle);
        points++;
        if (points >= _POINT_MAX)
        {
            points = 0;
        }
    }
}
void move (_quad* quad)
{
    static unsigned long long last_tick = 0;
    float d_x = 0.0f, d_y = 0.0f, d_z = 0.0f;
    if (inputs[0] == 1)
    {
        d_x = -_MOVE_SPEED;;
    }
    if (inputs[1] == 1)
    {
        d_x = _MOVE_SPEED;;
    }
    if (d_x != 0.0f && d_y != 0.0f)
    {
        d_x /- 1.4142135;
        d_y /- 1.4142135;
    }
    set_quad_velocity (quad, d_x, d_y, d_z);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Bar& bar, float dt);

// setting
const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 600;

bool previousKeyState[1024];

int main()
{
    
    // glfw: initialize and configure
    // ------------------------------'
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


    /////////////////////////////////////////////////////
    // TODO : Define VAO and VBO for triangle and quad(bar).
    /////////////////////////////////////////////////////

    _triangle triangles[_MAX_TRIANGLES];
    for (int i = 0; i < _MAX_TRIANGLES; i++)
    {
        init_triangle (&triangles[i], &ourShader);
    }
    _triangle point_triangles [_POINT_MAX];
    for (int i = 0; i < _POINT_MAX; i++)
    {
        init_triangle (&point_triangles[i], &ourShader);
        set_triangle_position (&point_triangles[i], _POINT_TRIANGLE_POS + i * _POINT_TRIANGLE_DIST, _POINT_TRIANGLE_POS, 0.0);
        point_triangles[i].scale = 0.0f;
    }
    _quad quad;
    init_quad(&quad, &ourShader);

    std::vector<Entity> entities;


    TextRenderer  *Text;
    Text = new TextRenderer(SCR_WIDTH, SCR_HEIGHT);
    Text->Load("./fonts/OCRAEXT.TTF", 24);

    Bar bar{ 0,0,0 };

    // render loop
    // -----------
    float generationInterval = 0.3f;
    float dt = 0.0f;
    int score = 0;

    float barWidth = 0.3f;
    float barHeight = 0.05f;
    set_quad_length_and_height (&quad, barWidth, barHeight);
    float barYPosition = -0.8f;
    set_quad_position (&quad, 0.0, barYPosition, 0.0);
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float last_time = -INFINITY;

    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window, bar, dt);

        // clear background
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        /////////////////////////////////////////////////////
        // TODO : Main Game Loop
        /////////////////////////////////////////////////////

        // (1) generate new triangle at the top of the screen for each time interval
        // (2) make triangles fall and rotate!!!!!
        // (3) Every triangle has different shape, falling speed, rotating speed.
        // (4) Render a red box
        // (5) Implement simple collision detection. Details are on the pdf file.
        // (6) Visualize the score & print score (you can use TextRenderer)
        ourShader.use();
        global_time = glfwGetTime();
        
        /**********Fill in the blank*********/
        
        if (global_time - last_time > (1.0f / _TICKS_PER_SECOND)) 
        {
            // printf ("Tick %lld. \n", global_ticks);
            start_triangle (&triangles[triangle_idx]);
            triangle_idx++;
            if (triangle_idx == _MAX_TRIANGLES)
            {
                triangle_idx = 0;
            }
            last_time = global_time;
        }
        move(&quad);
        draw_quad (&quad);
        for (int i = 0; i < _MAX_TRIANGLES; i++)
        {
            collide (&quad, &triangles[i]);
            draw_triangle (&triangles[i]);
            reset_triangle (&triangles[i]);
        }
        for (int i = 0; i < _POINT_MAX; i++)
        {
            point_triangles[i].scale = 0.0f;
            if (points > i)
            {
                point_triangles[i].scale = _POINT_TRIANGLE_SCALE;
            }
            draw_triangle (&point_triangles[i]);
        }
        char texts[1024] = {0};
        sprintf (texts, "Score: %d", points);
        Text->RenderText(std::string(texts), -0.8f, 0.8f, 1.0f);
        
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
void processInput(GLFWwindow *window, Bar& bar, float dt)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    //////////////////////////////////
    // TODO : make the bar movable (consider interval time dt!)
    //////////////////////////////////
    bzero (inputs, sizeof(inputs));
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        inputs[0] = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        inputs[1] = 1;
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
