#ifndef _FALLING_SAND_H
#define _FALLING_SAND_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define SCR_WIDTH 1600
#define SCR_RATIO 3/4
#define SCR_HEIGHT (SCR_WIDTH*SCR_RATIO)

#define WRD_WIDTH 520
#define WRD_HEIGHT (WRD_WIDTH*SCR_RATIO)

#define VTX_SCALE 1.0f
#define MOV_SCALE 0.2f

#define _GRAVITY 10.0f
#define _METER 1.0f
#define _VERTEX_SIZE (_METER/1000.0f)
#define _VERTEX_LIST_MIN_SIZE 4
#define _CHUNK_LIST_MIN_SIZE 4096
#define _K *1000UL
#define _M *1000000UL
#define _m /(1000UL)

typedef struct physics_property physics_property;
typedef struct vertex_obj vertex_obj;
typedef struct render_obj render_obj;
typedef struct int3 int3;
typedef struct world_obj world;

typedef enum MATERIAL_TYPE {AIR, SAND, WATER, STEAM, ROCK} MATERIAL_TYPE;

struct physics_property 
{
    MATERIAL_TYPE material;
    
    // Lighting
    glm::vec3 diffuse;

    // Kinetics
    float mass;
    float drag;
    float flow;

    bool apply_displacement;
    bool apply_gravity;
};

struct vertex_obj
{
    physics_property *phys_prop;
    world_obj *world;
    glm::vec3 force;
    glm::vec3 vel; 
    glm::vec3 mov;
};

struct render_obj
{
    glm::vec3 pos;
    glm::vec3 reflect; 
    glm::vec3 radiation;
};

struct world_obj
{
    bool event_flag;
    float event_time;
    float current_time;
    float delta_time;
    int width;
    int height;
    // Physics Engine
    vertex_obj* vertex_list;

    // Rendering Engine
    render_obj *render_list;

    unsigned int VBO, VAO;
};

extern physics_property* air;
extern physics_property* sand;
extern physics_property* water;
extern physics_property* steam;


void update_world_physics (world_obj *world);

void update_word_render_list (world_obj *world);

world_obj* make_world (int width, int height);

void update_world (world_obj *world);

void render_world (world_obj *world);

#endif