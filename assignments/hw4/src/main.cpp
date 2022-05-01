#define GLM_ENABLE_EXPERIMENTAL
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "opengl_utils.h"
#include "geometry_primitives.h"
#include <iostream>
#include <vector>
#include "camera.h"
#include "texture.h"
#include "texture_cube.h"
#include "model.h"
#include "mesh.h"
#include "scene.h"
#include "math_utils.h"
#include "light.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, DirectionalLight* sun);

bool isWindowed = true;
bool isKeyboardDone[1024] = { 0 };

// setting
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;
const float planeSize = 15.f;

// camera
Camera camera(glm::vec3(2.0f, 0.5f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
float currentTime = 0.0f;

float useNormalMap = 0.0f;
float useSpecular = 1.0f;
float useLighting = 0.0f;
float useShadow = 0.0f;

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
    Shader lightingShader("./shaders/shader_lighting.vs", "./shaders/shader_lighting.fs"); // you can name your shader files however you like
    Shader shadowShader("./shaders/shadow.vs", "./shaders/shadow.fs");
    Shader skyboxShader("./shaders/shader_skybox.vs", "./shaders/shader_skybox.fs");


    // define models
    // There can be three types 
    // (1) diffuse, specular, normal : brickCubeModel
    // (2) diffuse, normal only : boulderModel
    // (3) diffuse only : grassGroundModel
    Model penguinModel = Model("./resources/penguin/penguin.obj");
    penguinModel.diffuse = new Texture("./resources/penguin/penguin_d.jpg");

    Model brickCubeModel = Model("./resources/brickcube/brickcube.obj");
    brickCubeModel.diffuse = new Texture("./resources/brickcube/brickcube_d.png");
    brickCubeModel.specular = new Texture("./resources/brickcube/brickcube_s.png");
    brickCubeModel.normal = new Texture("./resources/brickcube/brickcube_n.png");

    Model boulderModel("./resources/boulder/boulder.obj");
    boulderModel.diffuse = new Texture("./resources/boulder/boulder_d.png");
    boulderModel.normal = new Texture("./resources/boulder/boulder_n.png");

    Model grassGroundModel = Model("./resources/plane.obj");
    grassGroundModel.diffuse = new Texture("./resources/grass_ground.jpg");
    grassGroundModel.ignoreShadow = true;

    
    // TODO: Add more models (barrels, fire extinguisher) and YOUR own model
    Model barrelModel = Model("./resources/barrel/barrel.obj");
    barrelModel.diffuse = new Texture("./resources/barrel/barrel_d.png");
    barrelModel.specular = new Texture("./resources/barrel/barrel_s.png");
    barrelModel.normal = new Texture("./resources/barrel/barrel_n.png");
    Model fireExtModel = Model("./resources/FireExt/FireExt.obj");
    fireExtModel.diffuse = new Texture("./resources/FireExt/FireExt_d.jpg");
    fireExtModel.specular = new Texture("./resources/FireExt/FireExt_s.jpg");
    fireExtModel.normal = new Texture("./resources/FireExt/FireExt_n.jpg");

    // Add entities to scene.
    // you can change the position/orientation.
    Scene scene;
    scene.addEntity(new Entity(&brickCubeModel, glm::mat4(1.0)));
    scene.addEntity(new Entity(&brickCubeModel, glm::translate(glm::vec3(-3.5f, 0.0f, -2.0f)) * glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f))));
    scene.addEntity(new Entity(&brickCubeModel, glm::translate(glm::vec3(1.0f, 0.5f, -3.0f)) * glm::rotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f))));
    scene.addEntity(new Entity(&barrelModel, glm::vec3(2.5f, 0.0f, -2.0f), 0, 0, 0, 0.1f));

    glm::mat4 planeWorldTransform = glm::mat4(1.0f);
    planeWorldTransform = glm::scale(planeWorldTransform, glm::vec3(planeSize));
    planeWorldTransform = glm::translate(glm::vec3(0.0f, -0.5f, 0.0f)) * planeWorldTransform;
    scene.addEntity(new Entity(&grassGroundModel, planeWorldTransform));

    scene.addEntity(new Entity(&fireExtModel, glm::vec3(2,-1,0), 0.0f, 180.0f, 0.0f, 0.002f));
    scene.addEntity(new Entity(&boulderModel, glm::vec3(-5, 0, 2), 0.0f, 180.0f, 0.0f, 0.1));

    // add your model's entity here!
    scene.addEntity(new Entity(&penguinModel, glm::vec3(0, -0.4, 1), -90.0f, 0.0f, 0.0f, 0.01));


    // define depth texture
    DepthMapTexture depth = DepthMapTexture(SHADOW_WIDTH, SHADOW_HEIGHT);


    // skybox
    std::vector<std::string> faces
    {
        "./resources/skybox/right.jpg",
        "./resources/skybox/left.jpg",
        "./resources/skybox/top.jpg",
        "./resources/skybox/bottom.jpg",
        "./resources/skybox/front.jpg",
        "./resources/skybox/back.jpg"
    };
    CubemapTexture skyboxTexture = CubemapTexture(faces);
    unsigned int VAOskybox, VBOskybox;
    getPositionVAO(skybox_positions, sizeof(skybox_positions), VAOskybox, VBOskybox);



    lightingShader.use();
    lightingShader.setInt("material.diffuseSampler", 0);
    lightingShader.setInt("material.specularSampler", 1);
    lightingShader.setInt("material.normalSampler", 2);
    lightingShader.setInt("depthMapSampler", 3);
    lightingShader.setFloat("material.shininess", 64.f);    // set shininess to constant value.

    skyboxShader.use();
    skyboxShader.setInt("skyboxTexture1", 0);


    DirectionalLight sun(0.0f, 30.0f, glm::vec3(0.8f));

    float oldTime = 0;
    while (!glfwWindowShouldClose(window))// render loop
    {
        currentTime = glfwGetTime();
        float dt = currentTime - oldTime;
        deltaTime = dt;
        oldTime = currentTime;

        static float debugTime = 0.0f;
        if (currentTime > debugTime + 0.5f)
        {
            debugTime = currentTime;
            printf ("Sun az: %2.2f, el: %2.2f, Dir: (%2.2f, %2.2f, %2.2f)\nuseNorm: %2.2f, useSpec: %2.2f, useShadow: %2.2f, useLight: %2.2f\n",
                sun.azimuth, sun.elevation, sun.lightDir.x, sun.lightDir.y, sun.lightDir.z, useNormalMap, useSpecular, useShadow, useLighting);
            printf ("Cam Pos: (%2.2f, %2.2f, %2.2f)\n",
                camera.Position.x, camera.Position.y, camera.Position.z);
        }

        // input
        processInput(window, &sun);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        
        // TODO : 
        // (1) render shadow map!
            // framebuffer: shadow frame buffer(depth.depthMapFBO)
            // shader : shadow.fs/vs
        // (2) render objects in the scene!
            // framebuffer : default frame buffer(0)
            // shader : shader_lighting.fs/vs
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        lightingShader.use();
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", projection);
        lightingShader.setVec3("light.dir", sun.lightDir);
        lightingShader.setVec3("light.color", sun.lightColor);
        lightingShader.setMat4("viewPos", view);

        map<Model*, vector<Entity*>>::iterator it;
        for (it = scene.entities.begin(); it != scene.entities.end(); it++)
        {
            for (int i = 0; i < it->second.size(); i++)
            {
                // printf ("Rendering VAO %d, iter %d.\n", it->first->VAO, i);
                lightingShader.setFloat("useNormalMap", it->first->normal? useNormalMap : 0);
                lightingShader.setFloat("useSpecularMap", it->first->specular? useSpecular : 0);
                lightingShader.setFloat("useShadow", useShadow);
                lightingShader.setFloat("useLighting", useLighting);

                it->first->bind();
                lightingShader.setMat4("world", it->second[i]->getModelMatrix());
                glDrawElements(GL_TRIANGLES, it->first->mesh.indices.size(), GL_UNSIGNED_INT, 0);
            }
        }

        // Iterate using map<Model*, vector<Entity*>>::iterator it = scene.entities.begin()



        
        
        // use skybox Shader
        skyboxShader.use();
        glDepthFunc(GL_LEQUAL);
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        // render a skybox
        glBindVertexArray(VAOskybox);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture.textureID);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, DirectionalLight* sun)
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

    static float key_time = 0.0f;
    if (currentTime > key_time + 0.15f)
    {
        key_time = currentTime;
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            useNormalMap = (useNormalMap == 1.0f)? 0.0f : 1.0f;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            useShadow = (useShadow == 1.0f)? 0.0f : 1.0f;
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
            useLighting = (useLighting == 1.0f)? 0.0f : 1.0f;
            
    }

    float t = 20.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        sun->processKeyboard(0.0f, t);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        sun->processKeyboard(0.0f, -t);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        sun->processKeyboard(t, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        sun->processKeyboard(-t, 0.0f);
    // TODO : 
    // Arrow key : increase, decrease sun's azimuth, elevation with amount of t.
    // key 1 : toggle using normal map
    // key 2 : toggle using shadow
    // key 3 : toggle using whole lighting



        
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
