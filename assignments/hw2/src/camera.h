#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW         =  0.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f; //(FOV)


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front; // Calculated with camera position at (0,0,0)
    glm::vec3 Up; // Calculated with camera position at (0,0,0)
    glm::vec3 Right; // Calculated with camera position at (0,0,0)
    glm::vec3 WorldUp;
    glm::vec3 WorldRight;
    // Euler Angles
    float Yaw;
    float Pitch;
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        // TODO : fill in
        Position = position;
        Up = up;
        Right = glm::vec3(1.0f, 0.0f, 0.0f);
        WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        WorldRight = glm::vec3(1.0f, 0.0f, 0.0f);
        Yaw = yaw;
        Pitch = pitch;
    }

    // // Constructor with scalar values
    // Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    // {
    //     // TODO : fill in


    // }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        // TODO : fill in
        updateCameraVectors();
        glm::mat4 view = glm::lookAt(Position, Position + Front, Up);
        return view;
    }
    glm::mat4 GetProjMatrix()
    {
        // TODO : fill in
        updateCameraVectors();
        glm::mat4 projection = glm::perspective(glm::radians(Zoom), (float)SCR_WIDTH/(float)SCR_HEIGHT, 0.1f, 100.0f);
        return projection;
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        // TODO : fill in
        updateCameraVectors();
        switch (direction)
        {
        case FORWARD:
            Position += Front*deltaTime*MovementSpeed;
            break;
        case BACKWARD:
            Position += -Front*deltaTime*MovementSpeed;
            break;
        case RIGHT:
            Position += Right*deltaTime*MovementSpeed;
            break;
        case LEFT:
            Position += -Right*deltaTime*MovementSpeed;
            break;
        
        default:
            break;
        }

    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        // TODO : fill in
        // pitch between -89 and 89 degree
        Yaw -= xoffset*MouseSensitivity;
        Pitch -= yoffset*MouseSensitivity;
        if (Yaw > 360.0f)
        {
            Yaw -= 360.0f;
        }
        if (Yaw < 0.0f)
        {
            Yaw += 360.0f;
        }
        if (Pitch > 180.0f)
        {
            Pitch -= 180.0f;
        }
        if (Pitch < -180.0f)
        {
            Pitch += 180.0f;
        }
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
            {
                Pitch = 89.0f;
            }
            if (Pitch < -89.0f)
            {
                Pitch = -89.0f;
            }
        }
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        // TODO : fill in
        Zoom -= yoffset;
        if (Zoom > 179.0f)
        {
            Zoom = 179.0f;
        } 
        if (Zoom < 1.0f)
        {
            Zoom = 1.0f;
        }
    }

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // TODO : fill in
        glm::mat4 cam_yaw_mat = glm::rotate (glm::mat4(1.0f), glm::radians (Yaw), WorldUp);
        glm::mat4 cam_yaw_pitch_mat = glm::rotate (cam_yaw_mat, glm::radians (Pitch), WorldRight);
        Up = cam_yaw_pitch_mat * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        Right = cam_yaw_pitch_mat * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        Front = cam_yaw_pitch_mat * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
    }
};
#endif