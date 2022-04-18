#define GLM_ENABLE_EXPERIMENTAL
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))  
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))  
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "resource_utils.h"
#include <iostream>
#include <vector>
#include "camera.h"
#include "math_utils.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

bool isKeyboardDone[1024] = {0};

// setting
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 1.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
float currentTime = 0.0f;
int draw_outline = 0;
int polygon_line_mode = 0;
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
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    

    // build and compile our shader program
    // ------------------------------------
    // TODO: define 3 shaders
    // (1) geometry shader for spline render.
    Shader spl_shader("./shaders/splines/spline_shader.vs", "./shaders/splines/spline_shader.fs", "./shaders/splines/spline_shader.gs");
    // (2) simple shader for spline's outer line render.
    Shader ol_shader("./shaders/outer_line_shader.vs", "./shaders/outer_line_shader.fs");
    // (optional) (3) tessellation shader for bezier surface.
    Shader tes_shader("./shaders/bezier_surface/tess.vs", "./shaders/bezier_surface/tess.fs", "./shaders/bezier_surface/tess.gs",
        "./shaders/bezier_surface/TCS.glsl", "./shaders/bezier_surface/TES.glsl");


    // TODO : load requied model and save data to VAO. 
    // Implement and use loadSplineControlPoints/loadBezierSurfaceControlPoints in resource_utils.h
    VAO *vao_spline_simple = loadSplineControlPoints ("./resources/spline_control_point_data/spline_simple.txt");
    VAO *vao_spline_u = loadSplineControlPoints ("./resources/spline_control_point_data/spline_u.txt");
    VAO *vao_spline_complex = loadSplineControlPoints ("./resources/spline_control_point_data/spline_complex.txt");

    VAO *vao_tes_sphere = loadBezierSurfaceControlPoints ("./resources/bezier_surface_data/sphere.bpt");
    VAO *vao_tes_gumbo = loadBezierSurfaceControlPoints ("./resources/bezier_surface_data/gumbo.bpt");
    VAO *vao_tes_teapot = loadBezierSurfaceControlPoints ("./resources/bezier_surface_data/teapot.bpt");

    // render loop
    // -----------
    float oldTime = 0;
    while (!glfwWindowShouldClose(window))
    {
        currentTime = glfwGetTime();
        float dt = currentTime - oldTime;
        deltaTime = dt;

        oldTime = currentTime;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // TODO : render splines
        // (1) render simple spline with 4 control points for Bezier, Catmull-Rom and B-spline.
        spl_shader.use();
        spl_shader.setMat4("view", glm::mat4(1.0f));
        spl_shader.setMat4("projection", glm::mat4(1.0f));

        // Bezier
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate (model, glm::vec3(-0.8f, -0.5f, 0.0f));
        // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
        model = glm::scale (model, glm::vec3(0.25f, 0.25f, 0.0f));
        spl_shader.setMat4("model", model);
        spl_shader.setMat4("B", glm::mat4(glm::vec4(-1,3,-3,1), glm::vec4(3,-6,3,0), glm::vec4(-3,3,0,0), glm::vec4(1,0,0,0)));
        glBindVertexArray(vao_spline_simple->ID);
        glDrawArrays(GL_LINES_ADJACENCY, 0, vao_spline_simple->num_of_vertex);

        // B-spline
        model = glm::mat4(1.0f);
        model = glm::translate (model, glm::vec3(-0.5f, -0.5f, 0.0f));
        // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
        model = glm::scale (model, glm::vec3(0.25f, 0.25f, 0.0f));
        spl_shader.setMat4("model", model);
        spl_shader.setMat4("B", glm::mat4(glm::vec4(-1.0/6,3.0/6,-3.0/6,1.0/6), glm::vec4(3.0/6,-6.0/6,3.0/6,0), glm::vec4(-3.0/6.0,0,3.0/6.0,0), glm::vec4(1.0/6.0,4.0/6.0,1.0/6.0,0)));
        glBindVertexArray(vao_spline_simple->ID);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, vao_spline_simple->num_of_vertex);

        // Catmull-Rom 
        model = glm::mat4(1.0f);
        model = glm::translate (model, glm::vec3(-0.2f, -0.5f, 0.0f));
        // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
        model = glm::scale (model, glm::vec3(0.25f, 0.25f, 0.0f));
        spl_shader.setMat4("model", model);
        spl_shader.setMat4("B", glm::mat4(glm::vec4(-0.5,1.5,-1.5,0.5), glm::vec4(1,-2.5,2.0,-0.5), glm::vec4(-0.5,0.0,0.5,0), glm::vec4(0,1,0,0)));
        glBindVertexArray(vao_spline_simple->ID);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, vao_spline_simple->num_of_vertex);

        // (2) render 'u' using Bezier spline
        model = glm::mat4(1.0f);
        model = glm::translate (model, glm::vec3(0.1f, -0.6f, 0.0f));
        // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
        model = glm::scale (model, glm::vec3(0.05f, 0.05f, 0.0f));
        spl_shader.setMat4("model", model);
        spl_shader.setMat4("B", glm::mat4(glm::vec4(-1,3,-3,1), glm::vec4(3,-6,3,0), glm::vec4(-3,3,0,0), glm::vec4(1,0,0,0)));
        glBindVertexArray(vao_spline_u->ID);
        glDrawArrays(GL_LINES_ADJACENCY, 0, vao_spline_u->num_of_vertex);

        // (3) render loop using Catmull-Rom spline and B-spline.
        // B-spline
        model = glm::mat4(1.0f);
        model = glm::translate (model, glm::vec3(0.5f, -0.5f, 0.0f));
        // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
        model = glm::scale (model, glm::vec3(0.15f, 0.15f, 0.0f));
        spl_shader.setMat4("model", model);
        spl_shader.setMat4("B", glm::mat4(glm::vec4(-1.0/6,3.0/6,-3.0/6,1.0/6), glm::vec4(3.0/6,-6.0/6,3.0/6,0), glm::vec4(-3.0/6.0,0,3.0/6.0,0), glm::vec4(1.0/6.0,4.0/6.0,1.0/6.0,0)));
        glBindVertexArray(vao_spline_complex->ID);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, vao_spline_complex->num_of_vertex);

        // Catmull-Rom 
        model = glm::mat4(1.0f);
        model = glm::translate (model, glm::vec3(0.8f, -0.5f, 0.0f));
        // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
        model = glm::scale (model, glm::vec3(0.15f, 0.15f, 0.0f));
        spl_shader.setMat4("model", model);
        spl_shader.setMat4("B", glm::mat4(glm::vec4(-0.5,1.5,-1.5,0.5), glm::vec4(1,-2.5,2.0,-0.5), glm::vec4(-0.5,0.0,0.5,0), glm::vec4(0,1,0,0)));
        glBindVertexArray(vao_spline_complex->ID);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, vao_spline_complex->num_of_vertex);

        // You have to also render outer line of control points!
        if (draw_outline)
        {
            ol_shader.use();
            ol_shader.setMat4("view", glm::mat4(1.0f));
            ol_shader.setMat4("projection", glm::mat4(1.0f));

            // Bezier
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate (model, glm::vec3(-0.8f, -0.5f, 0.0f));
            // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
            model = glm::scale (model, glm::vec3(0.25f, 0.25f, 0.0f));
            ol_shader.setMat4("model", model);
            glBindVertexArray(vao_spline_simple->ID);
            glDrawArrays(GL_LINE_STRIP, 0, vao_spline_simple->num_of_vertex);

            // B-spline
            model = glm::mat4(1.0f);
            model = glm::translate (model, glm::vec3(-0.5f, -0.5f, 0.0f));
            // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
            model = glm::scale (model, glm::vec3(0.25f, 0.25f, 0.0f));
            ol_shader.setMat4("model", model);
            glBindVertexArray(vao_spline_simple->ID);
            glDrawArrays(GL_LINE_STRIP, 0, vao_spline_simple->num_of_vertex);

            // Catmull-Rom 
            model = glm::mat4(1.0f);
            model = glm::translate (model, glm::vec3(-0.2f, -0.5f, 0.0f));
            // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
            model = glm::scale (model, glm::vec3(0.25f, 0.25f, 0.0f));
            ol_shader.setMat4("model", model);
            glBindVertexArray(vao_spline_simple->ID);
            glDrawArrays(GL_LINE_STRIP, 0, vao_spline_simple->num_of_vertex);

            // (2) render 'u' using Bezier spline
            model = glm::mat4(1.0f);
            model = glm::translate (model, glm::vec3(0.1f, -0.6f, 0.0f));
            // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
            model = glm::scale (model, glm::vec3(0.05f, 0.05f, 0.0f));
            ol_shader.setMat4("model", model);
            glBindVertexArray(vao_spline_u->ID);
            glDrawArrays(GL_LINE_STRIP, 0, vao_spline_u->num_of_vertex);

            // (3) render loop using Catmull-Rom spline and B-spline.
            // B-spline
            model = glm::mat4(1.0f);
            model = glm::translate (model, glm::vec3(0.5f, -0.5f, 0.0f));
            // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
            model = glm::scale (model, glm::vec3(0.15f, 0.15f, 0.0f));
            ol_shader.setMat4("model", model);
            glBindVertexArray(vao_spline_complex->ID);
            glDrawArrays(GL_LINE_STRIP, 0, vao_spline_complex->num_of_vertex);

            // Catmull-Rom 
            model = glm::mat4(1.0f);
            model = glm::translate (model, glm::vec3(0.8f, -0.5f, 0.0f));
            // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
            model = glm::scale (model, glm::vec3(0.15f, 0.15f, 0.0f));
            ol_shader.setMat4("model", model);
            glBindVertexArray(vao_spline_complex->ID);
            glDrawArrays(GL_LINE_STRIP, 0, vao_spline_complex->num_of_vertex);
        }

        // (Optional) TODO : render Bezier surfaces using tessellation shader.
        
        tes_shader.use();
        tes_shader.setMat4("view", camera.GetViewMatrix());
        tes_shader.setMat4("projection", glm::perspective(glm::radians(camera.Zoom ), (float)SCR_WIDTH/(float)SCR_HEIGHT, 0.1f, 100.0f));

        if (polygon_line_mode == 1)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        tes_shader.setVec3("cameraPosition", camera.Position);
        
        printf ("%3.3f\n", glm::distance(glm::vec3(-0.4f, 0.1f, 0.0f), camera.Position));
        model = glm::mat4(1.0f);
        model = glm::translate (model, glm::vec3(-0.4f, 0.1f, 0.0f));
        // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
        model = glm::scale (model, glm::vec3(0.25f, 0.25f, 0.25f));
        tes_shader.setMat4("model", model);
        tes_shader.setMat4("B", glm::mat4(glm::vec4(-1,3,-3,1), glm::vec4(3,-6,3,0), glm::vec4(-3,3,0,0), glm::vec4(1,0,0,0)));
        glBindVertexArray(vao_tes_sphere->ID);
        glPatchParameteri(GL_PATCH_VERTICES, 16);
        glDrawArrays(GL_PATCHES, 0, vao_tes_sphere->num_of_vertex);

        model = glm::mat4(1.0f);
        model = glm::translate (model, glm::vec3(0.6f, -0.1f, 0.0f));
        // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
        model = glm::scale (model, glm::vec3(0.05f, 0.05f, 0.05f));
        tes_shader.setMat4("model", model);
        tes_shader.setMat4("B", glm::mat4(glm::vec4(-1,3,-3,1), glm::vec4(3,-6,3,0), glm::vec4(-3,3,0,0), glm::vec4(1,0,0,0)));
        glBindVertexArray(vao_tes_gumbo->ID);
        glPatchParameteri(GL_PATCH_VERTICES, 16);
        glDrawArrays(GL_PATCHES, 0, vao_tes_gumbo->num_of_vertex);

        model = glm::mat4(1.0f);
        model = glm::translate (model, glm::vec3(0.2f, 0.1f, 0.0f));
        // model = glm::rotate (model, glm::radians (90.0f), glm::vec3 (-1.0f, 0.0f, 0.0f));
        model = glm::scale (model, glm::vec3(0.15f, 0.15f, 0.15f));
        tes_shader.setMat4("model", model);
        tes_shader.setMat4("B", glm::mat4(glm::vec4(-1,3,-3,1), glm::vec4(3,-6,3,0), glm::vec4(-3,3,0,0), glm::vec4(1,0,0,0)));
        glBindVertexArray(vao_tes_teapot->ID);
        glPatchParameteri(GL_PATCH_VERTICES, 16);
        glDrawArrays(GL_PATCHES, 0, vao_tes_teapot->num_of_vertex);



        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    //glDeleteVertexArrays(1,&VAO);
    //glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    static float last_9_time = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS && currentTime - last_9_time > 0.3)
    {
        draw_outline ^= 1;
        last_9_time = currentTime;
    }
    static float last_0_time = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS && currentTime - last_0_time > 0.3)
    {
        polygon_line_mode ^= 1;
        last_0_time = currentTime;
    }

    // TODO : 
    // (1) (for spline) if we press key 9, toggle whether to render outer line.
    // (2) (Optional, for Bezier surface )if we press key 0, toggle GL_FILL and GL_LINE.
    
    
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}