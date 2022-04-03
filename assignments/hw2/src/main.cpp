#define GLM_ENABLE_EXPERIMENTAL
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "geometry_primitives.h"
#include <iostream>
#include <vector>
#include "camera.h"
#include "texture.h"
#include "texture_cube.h"
#include "math_utils.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

void camera_movement (Camera &camera);

bool isWindowed = true;
int isKeyboardProcessed[1024] = {0};

// setting
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(1.17f, 9.701f, -1.046f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


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

    // define normal shader and skybox shader.
    Shader shader("./shaders/shader.vs", "./shaders/shader.fs"); // you can name your shader files however you like
    Shader skyboxShader("./shaders/shader_skybox.vs","./shaders/shader_skybox.fs");


    // TODO : define required VAOs(textured cube, skybox, quad)
    // data are defined in geometry_primitives.h

    unsigned int VBO_cube, VAO_cube;
    glGenVertexArrays (1, &VAO_cube);
    glGenBuffers (1, &VBO_cube);

    glBindVertexArray (VAO_cube);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_positions_textures), cube_positions_textures, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);




    // world space positions of our cubes
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  1.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f,  2.2f, -2.5f),
        glm::vec3(-3.8f,  2.0f, -12.3f),
        glm::vec3( 2.4f,  1.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  1.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    const int n_grass = 1000;
    float grassGroundSize = 20;
    glm::vec3 grassPositions[n_grass];

    // positions of our grasses
    for(int i=0; i<n_grass; i++){
        float s = grassGroundSize/2;
        float x = getRandomValueBetween(-s, s);
        float z = getRandomValueBetween(-s, s);
        grassPositions[i].x = x;
        grassPositions[i].y = 0.5f;
        grassPositions[i].z = z;
    }


    // TODO : define textures (container, grass, grass_ground) & cubemap textures (day, night)

    unsigned int texture_containter, texture2;

	// texture_containter
	// ---------
	glGenTextures(1, &texture_containter);
	glBindTexture(GL_TEXTURE_2D, texture_containter);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width_containter, height_containter, nrChannels_containter;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data_containter = stbi_load("./resources/container.jpg", &width_containter, &height_containter, &nrChannels_containter, 0);
	if (data_containter)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_containter, height_containter, 0, GL_RGB, GL_UNSIGNED_BYTE, data_containter);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data_containter);

    // TODO : set texture & skybox texture uniform value (initialization)
    // e.g) shader.use(), shader.setInt("texture", 0);

    shader.use();
	shader.setInt("texture_containter", 0);

    // render loop
    // -----------
    float debug_time = -INFINITY;
    while (!glfwWindowShouldClose(window)){

        deltaTime = glfwGetTime() - lastFrame;
        lastFrame = glfwGetTime();

        if (debug_time + 0.5 < glfwGetTime())
        {
            debug_time = glfwGetTime();
            printf ("Cam position: %2.2f,%2.2f,%2.2f, Front: %2.2f,%2.2f,%2.2f, Up: %2.2f,%2.2f,%2.2f, Right: %2.2f,%2.2f,%2.2f, Yaw: %2.2f, Pitch: %2.2f, Zoom: %2.2f\n",
            camera.Position.x, camera.Position.y, camera.Position.z,
            camera.Front.x, camera.Front.y, camera.Front.z,
            camera.Up.x, camera.Up.y, camera.Up.z, 
            camera.Right.x, camera.Right.y, camera.Right.z, camera.Yaw, camera.Pitch, camera.Zoom);
        }

        // input
        // -----
        processInput(window);
        camera_movement (camera);
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /////////////////////////////////////////////////////
        // TODO : Main Rendering Loop
        /////////////////////////////////////////////////////

        // Normal shader renders.

        shader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
		shader.setMat4("projection", camera.GetProjMatrix());
		shader.setMat4("view", camera.GetViewMatrix());

        
        // (1) render boxes(cube) using normal shader.
		glBindVertexArray(VAO_cube);
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_containter);
		for (unsigned int i = 0; i < 10; i++)
		{
			// locate the cubes where you want!
			glm::mat4 model = glm::translate (glm::mat4(1.0f), cubePositions[i]);
			for (int j = 0; j < i%3; j++)
				model = glm::rotate (model, glm::radians (39.0f), glm::vec3 (1.0f, 0.0f, 1.0f));
			for (int j = 0; j < i%2; j++)
				model = glm::rotate (model, glm::radians (23.0f), glm::vec3 (0.0f, 1.0f, 1.0f));
			for (int j = 0; j < i%5; j++)
				model = glm::rotate (model, glm::radians (47.0f), glm::vec3 (1.0f, 0.0f, 0.0f));
			shader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}


        // (2) render ground(quad) using normal shader.
        // (3) render billboard grasses(quad) using normal shader.
        // (4) render skybox using skybox shader.



        // rendering pseudo-code

        // update projection, view matrix to shader
        // for each model:
        //      bind VAO, texture
        //      for each entity that belongs to the model:
        //          update model(transformation) matrix to shader
        //          glDrawArrays or glDrawElements





        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void camera_movement (Camera &camera)
{
    if (isKeyboardProcessed [GLFW_KEY_W])
    {
        camera.ProcessKeyboard (FORWARD, deltaTime);
    }
    if (isKeyboardProcessed [GLFW_KEY_S])
    {
        camera.ProcessKeyboard (BACKWARD, deltaTime);
    }
    if (isKeyboardProcessed [GLFW_KEY_A])
    {
        camera.ProcessKeyboard (LEFT, deltaTime);
    }
    if (isKeyboardProcessed [GLFW_KEY_D])
    {
        camera.ProcessKeyboard (RIGHT, deltaTime);
    }

    bzero (isKeyboardProcessed, 1024*sizeof(int));
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // TODO : make camera movable (WASD) & increase or decrease dayFactor(press O: increase, press P: decrease)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        isKeyboardProcessed [GLFW_KEY_W] = 1;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        isKeyboardProcessed [GLFW_KEY_A] = 1;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        isKeyboardProcessed [GLFW_KEY_S] = 1;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        isKeyboardProcessed [GLFW_KEY_D] = 1;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        isKeyboardProcessed [GLFW_KEY_O] = 1;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        isKeyboardProcessed [GLFW_KEY_P] = 1;
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
    // TODO: calculate how much cursor have moved, rotate camera proportional to the value, using ProcessKeyboard.
    static double x_old = xpos, y_old = ypos;
    camera.ProcessMouseMovement (xpos - x_old, ypos - y_old);
    x_old = xpos;
    y_old = ypos;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
