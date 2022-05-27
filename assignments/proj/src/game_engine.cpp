#include "falling_sand.h"

physics_property _air = {
    .material = AIR, .diffuse = {0.65, 0.65, 0.85}, .mass = 0.05f,
    .apply_displacement = true, .apply_gravity = false,};
physics_property* air = &_air;
physics_property _sand = {
    .material = SAND, .diffuse = {0.4, 0.4, 0.1}, .mass = 10.0f,
    .apply_displacement = true, .apply_gravity = true,};
physics_property* sand = &_sand;
physics_property _water = {
    .material = WATER, .diffuse = {0.1, 0.1, 0.8}, .mass = 1.0f,
    .apply_displacement = true, .apply_gravity = true,};
physics_property* water = &_water;
physics_property _steam = {
    .material = STEAM, .diffuse = {0.8, 0.8, 0.8}, .mass = 0.01f,
    .apply_displacement = true, .apply_gravity = true,};
physics_property* steam = &_steam;
physics_property _rock = {
    .material = ROCK, .diffuse = {0.8, 0.8, 0.8}, .mass = 10.0f,
    .apply_displacement = true, .apply_gravity = true,};
physics_property* rock = &_rock;


vertex_obj *l_vertex(vertex_obj *vtx)
{
    world *world = vtx->world;
    if (vtx->pos.x == 0)
        return NULL;
    return world->vertex_list + vtx->pos.y * world->width + vtx->pos.x - 1;
}

void generate_vertex(world *world, glm::vec3 pos, physics_property mat)
{

}

void update_world_physics (world *world)
{

}

void update_word_render_list (world *world)
{
    int idx = 0;
    for (int h = 0; h < world->height; h++)
    {
        for (int w = 0; w < world->width; w++)
        {
            vertex_obj *v_obj = world->vertex_list + idx;
            render_obj *r_obj = world->render_list + idx;
            idx++;
            r_obj->pos.x = SCR_SCALE*((float)v_obj->pos.x - (SCR_WIDTH/2.0))/SCR_WIDTH;
            r_obj->pos.y = SCR_SCALE*((float)v_obj->pos.y - (SCR_HEIGHT/2.0))/SCR_HEIGHT;
            r_obj->reflect = v_obj->phys_prop->diffuse;
            r_obj->radiation = glm::vec3(0);
        }
    }
}

world* make_world (uint64_t width, uint64_t height)
{
    world *world_out = (world*)calloc (1, sizeof(world));
    world_out->height = height;
    world_out->width = width;
    world_out->vertex_list = (vertex_obj*)calloc (width*height, sizeof(vertex_obj));
    world_out->render_list = (render_obj*)calloc (width*height, sizeof(render_obj));
    int idx = 0;
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            vertex_obj *v_obj = world_out->vertex_list + idx;
            idx++;
            v_obj->world = world_out;
            v_obj->phys_prop = air;
            v_obj->pos.x = w;
            v_obj->pos.y = h;
            v_obj->pos.z = 0;
            v_obj->vel.x = 0.0f;
            v_obj->vel.y = 0.0f;
            v_obj->vel.z = 0.0f;
        }
    }
    update_word_render_list (world_out);

    glGenVertexArrays(1, &world_out->VAO);
    glGenBuffers(1, &world_out->VBO);

    glBindVertexArray(world_out->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, world_out->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(render_obj)*width*height, world_out->render_list, GL_STATIC_DRAW);

    // pos attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(render_obj), (void*)0);
    glEnableVertexAttribArray(0);
    
    // reflect attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(render_obj), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // radiation attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(render_obj), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return world_out;
}

void update_world (world *world)
{
    update_world_physics (world);
    update_word_render_list (world);
}

void render_world (world *world)
{
    glBindVertexArray(world->VAO);
    glDrawArrays(GL_POINTS, 0, world->height*world->width);
}