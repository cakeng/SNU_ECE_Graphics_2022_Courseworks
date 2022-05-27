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
#define SCR_HEIGHT 1200
#define SCR_SCALE 2.0

#define _DELTA_TIME = 0.001f
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

    bool apply_displacement;
    bool apply_gravity;
};

struct int3
{
    int x;
    int y;
    int z;
};

struct vertex_obj
{
    physics_property *phys_prop;
    world_obj *world;
    int3 pos;
    glm::vec3 vel; // in meter per frame
    glm::vec3 force;
};

struct render_obj
{
    glm::vec3 pos;
    glm::vec3 reflect; 
    glm::vec3 radiation;
};

struct world_obj
{
    float t_delta;
    uint64_t width;
    uint64_t height;
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

world_obj* make_world (uint64_t width, uint64_t height);

void update_world (world_obj *world);

void render_world (world_obj *world);

#endif