#include "falling_sand.h"
#include <thread>


physics_property _air = {
    .material = AIR, .diffuse = {0.65, 0.65, 0.85}, .mass = 0.05f, .drag = 0.05,
    .apply_displacement = true, .apply_gravity = false,};
physics_property* air = &_air;
physics_property _sand = {
    .material = SAND, .diffuse = {0.65, 0.5, 0.2}, .mass = 10.0f, .drag = 1.0,
    .apply_displacement = true, .apply_gravity = true,};
physics_property* sand = &_sand;
physics_property _water = {
    .material = WATER, .diffuse = {0.1, 0.1, 0.8}, .mass = 1.0f, .drag = 0.6, .flow = 1.0f,
    .apply_displacement = true, .apply_gravity = true,};
physics_property* water = &_water;
physics_property _steam = {
    .material = STEAM, .diffuse = {0.8, 0.8, 0.8}, .mass = 0.01f, .drag = 0.35, .flow = 1.0f,
    .apply_displacement = true, .apply_gravity = true,};
physics_property* steam = &_steam;
physics_property _rock = {
    .material = ROCK, .diffuse = {0.43, 0.41, 0.42}, .mass = 15.0f, .drag = 1.0,
    .apply_displacement = false, .apply_gravity = true,};
physics_property* rock = &_rock;

inline float frand()
{
    return (float)rand() / RAND_MAX;
}

inline int vtx_w (vertex_obj *vtx)
{
    return (vtx - vtx->world->vertex_list) % vtx->world->width;
}

inline int vtx_h (vertex_obj *vtx)
{
    return (vtx - vtx->world->vertex_list) / vtx->world->width;
}

vertex_obj *get_vtx(world_obj *world, int w, int h)
{
    if (w < 0 || w >= world->width || h < 0 || h >= world->height)
        return NULL;
    return world->vertex_list + h * world->width + w;
}

inline vertex_obj *get_vtx_dg (world_obj *world, int w, int h)
{
    return world->vertex_list + h * world->width + w;
}

vertex_obj *l_vtx(vertex_obj *vtx)
{
    world_obj *world = vtx->world;
    if (vtx_w(vtx) == 0)
        return NULL;
    return vtx - 1;
}
vertex_obj *r_vtx(vertex_obj *vtx)
{
    world_obj *world = vtx->world;
    if (vtx_w(vtx)== world->width-1)
        return NULL;
    return vtx + 1;
}
vertex_obj *u_vtx(vertex_obj *vtx)
{
    world_obj *world = vtx->world;
    if (vtx_h(vtx) == 0)
        return NULL;
    return vtx - vtx->world->width;
}
vertex_obj *d_vtx(vertex_obj *vtx)
{
    world_obj *world = vtx->world;
    if (vtx_h(vtx) == world->height-1)
        return NULL;
    return vtx + vtx->world->width;
}

bool swap_vertex (vertex_obj *vtx1, vertex_obj* vtx2)
{
    if (vtx1 == vtx2)
        return true;
    if (!(vtx1->phys_prop->apply_displacement && vtx2->phys_prop->apply_displacement))
        return false;
    vertex_obj temp;
    temp = *vtx1;
    *vtx1 = *vtx2;
    *vtx2 = temp;
    return true;
}

void reset_vertex (vertex_obj *vtx)
{
    vtx->updated = false;
    vtx->phys_prop = air;
    vtx->force = glm::vec3(0.0);
    vtx->vel = glm::vec3(0.0);
    vtx->mov = glm::vec3(0.0);
}

void print_vertex (vertex_obj *vtx)
{
    printf ("Material: %d, force: (%2.2f, %2.2f), vel: (%2.2f, %2.2f), mov: (%2.2f, %2.2f)\n"
        , vtx->phys_prop->material, vtx->force.x, vtx->force.y, vtx->vel.x, vtx->vel.y
        , vtx->mov.x, vtx->mov.y);
}

void kinetic_engine (vertex_obj *vtx)
{
    float dt = vtx->world->delta_time;
    int w = vtx_w(vtx), h = vtx_h(vtx);
    physics_property *phys = vtx->phys_prop;
    if (!phys->apply_displacement)
    {
        vtx->vel = glm::vec3(0.0f);
        vtx->mov - glm::vec3(0.0f);
        return;
    }
    if (!phys->apply_gravity)
        return;
    
    // Gravity
    bool fall = false;
    vertex_obj *d_obj = d_vtx (vtx);
    if (d_obj)
    {
        if (d_obj->phys_prop->mass < phys->mass)
        {
            vtx->force += glm::vec3(0.0, (phys->mass - d_obj->phys_prop->mass) * _GRAVITY, 0.0);
        }
        else if (frand() < 0.5f)
        {
            vertex_obj *ld_obj = l_vtx (d_obj); 
            if (ld_obj && ld_obj->phys_prop->mass < phys->mass)
            {
                vtx->force += 
                    glm::vec3(-(phys->mass - ld_obj->phys_prop->mass) * _GRAVITY/1.414, (phys->mass - ld_obj->phys_prop->mass) * _GRAVITY/1.414, 0.0);
                fall = true;
            }
            else
            {
                vertex_obj *rd_obj = r_vtx (d_obj); 
                if (rd_obj && rd_obj->phys_prop->mass < phys->mass)
                {
                    vtx->force += 
                        glm::vec3((phys->mass - rd_obj->phys_prop->mass) * _GRAVITY/1.414, (phys->mass - rd_obj->phys_prop->mass) * _GRAVITY/1.414, 0.0);
                    fall = true;
                }
            }
        }
        else
        {
            vertex_obj *rd_obj = r_vtx (d_obj); 
            if (rd_obj && rd_obj->phys_prop->mass < phys->mass)
            {
                vtx->force += 
                    glm::vec3((phys->mass - rd_obj->phys_prop->mass) * _GRAVITY/1.414, (phys->mass - rd_obj->phys_prop->mass) * _GRAVITY/1.414, 0.0);
                fall = true;
            }
            else
            {
                vertex_obj *ld_obj = l_vtx (d_obj); 
                if (ld_obj && ld_obj->phys_prop->mass < phys->mass)
                {
                    vtx->force += 
                        glm::vec3(-(phys->mass - ld_obj->phys_prop->mass) * _GRAVITY/1.414, (phys->mass - ld_obj->phys_prop->mass) * _GRAVITY/1.414, 0.0);
                    fall = true;
                }
            }
        }
    }
    if (!fall && vtx->phys_prop->flow > 0.0)
    {
        if (vtx->vel.x > 0.0 && (r_vtx(vtx)->phys_prop->mass < vtx->phys_prop->flow))
        {
            vtx->force += FLOW_SCALE*glm::vec3((vtx->phys_prop->flow - r_vtx(vtx)->phys_prop->mass)*vtx->phys_prop->mass, 0.0, 0.0);
        }
        else if (vtx->vel.x < 0.0 && (l_vtx(vtx)->phys_prop->mass < vtx->phys_prop->flow))
        {
            vtx->force  += FLOW_SCALE*glm::vec3(-(vtx->phys_prop->flow - l_vtx(vtx)->phys_prop->mass)*vtx->phys_prop->mass, 0.0, 0.0);
        }
        else
        {
            if (frand() < 0.5 && (r_vtx(vtx)->phys_prop->mass < vtx->phys_prop->flow))
            {
                vtx->force += FLOW_SCALE*glm::vec3((vtx->phys_prop->flow - r_vtx(vtx)->phys_prop->mass)*vtx->phys_prop->mass, 0.0, 0.0);
            }
            else if (l_vtx(vtx)->phys_prop->mass < vtx->phys_prop->flow)
            {
                vtx->force += FLOW_SCALE*glm::vec3(-(vtx->phys_prop->flow - l_vtx(vtx)->phys_prop->mass)*vtx->phys_prop->mass, 0.0, 0.0);
            }
        }
    }
    // Drag
    int vec_w = 0, vec_h = 0;
    if (vtx->vel.x > 0)
    {
        vec_w = 1;
    }
    else if (vtx->vel.x < 0)
    {
        vec_w = -1;
    }
    if (vtx->vel.y > 0)
    {
        vec_h = 1;
    }
    else if (vtx->vel.y < 0)
    {
        vec_h = -1;
    }
    vertex_obj *targ_obj = get_vtx (vtx->world, w + vec_w, h + vec_h);
    if (targ_obj)
        vtx->force -= vtx->vel * (targ_obj->phys_prop->drag);

    vtx->vel += vtx->force / phys->mass;
    vtx->force = glm::vec3(0.0);
    vtx->mov += (dt * MOV_SCALE) * vtx->vel; 
}

vertex_obj *move_vertex (vertex_obj *vtx)
{
    int dw = vtx->mov.x;
    int dh = vtx->mov.y;
    float dh_dw = vtx->mov.y/vtx->mov.x;
    int w = vtx_w(vtx), h = vtx_h(vtx);
    if (dw > 0)
    {
        for (int i = 0; i < dw; i++)
        {
            vertex_obj *targ = get_vtx (vtx->world, w + (i+1), h + dh_dw*(i+1));
            if (!targ)
            {
                reset_vertex (vtx);
                return vtx;
            }
            if (!swap_vertex (targ, vtx))
            {
                vtx->mov = glm::vec3(0.0f);
                vtx->vel = glm::vec3(0.0f);
                goto MOV_EXIT;
            }
            vtx = targ;
        }
    }
    else if (dw < 0)
    {
        for (int i = 0; i < -dw; i++)
        {
            vertex_obj *targ = get_vtx (vtx->world, w - (i+1), h - dh_dw*(i+1));
            if (!targ)
            {
                reset_vertex (vtx);
                return vtx;
            }
            if (!swap_vertex (targ, vtx))
            {
                vtx->mov = glm::vec3(0.0f);
                vtx->vel = glm::vec3(0.0f);
                goto MOV_EXIT;
            }
            vtx = targ;
        }
    }
    else
    {
        if (dh > 0)
        {
            for (int i = 0; i < dh; i++)
            {
                vertex_obj *targ = get_vtx (vtx->world, w, h + (i+1));
                if (!targ)
                {
                    reset_vertex (vtx);
                    return vtx;
                }
                if (!swap_vertex (targ, vtx))
                {
                    vtx->mov = glm::vec3(0.0f);
                    vtx->vel = glm::vec3(0.0f);
                    goto MOV_EXIT;
                }
                vtx = targ;
            }
        }
        else
        {
            for (int i = 0; i < -dh; i++)
            {
                vertex_obj *targ = get_vtx (vtx->world, w, h - (i+1));
                if (!targ)
                {
                    reset_vertex (vtx);
                    return vtx;
                }
                if (!swap_vertex (targ, vtx))
                {
                    vtx->mov = glm::vec3(0.0f);
                    vtx->vel = glm::vec3(0.0f);
                    goto MOV_EXIT;
                }
                vtx = targ;
            }
        }
    }
    MOV_EXIT: 
    vtx->mov.x -= vtx_w(vtx) - w;
    vtx->mov.y -= vtx_h(vtx) - h;
    return vtx;
}

void generate_vertex(world_obj *world, int w, int h, physics_property *mat)
{
    vertex_obj obj;
    if (get_vtx(world, w, h)->phys_prop == mat)
        return;
    reset_vertex (&obj);
    obj.world = world;
    obj.phys_prop = mat;
    *get_vtx(world, w, h) = obj;
}

bool world_event (world_obj *world)
{
    if (world->current_time > world->event_time && world->event_flag)
    {
        generate_vertex (world, world->width/2, 10, sand);
        world->event_flag = false;
        return true;
    }
    return false;
}

void update_world_physics (world_obj *world)
{
    static float event_time[100] = {-10.0f};
    if (world->current_time > event_time[3] + 0.0)
    {
        for (int w = 0; w < 80; w++)
            generate_vertex (world, world->width/2 - 40 + w, world->height - 40, rock);
    }
    if (world->current_time > event_time[0] + 0.1)
    {
        generate_vertex (world, world->width/2 - 20, 10, sand);
        generate_vertex (world, world->width/2 + 20, 10, sand);
        generate_vertex (world, world->width/2, 10, water);
        event_time[0]= world->current_time;
    }
    if (world->current_time > event_time[2] + 0.5)
    {
        printf ("FPS: %3.2f, ct: %2.1f dt: %2.4f\n", 1.0/world->delta_time, world->current_time, world->delta_time);

        // vertex_obj *vtx = world->vertex_list;
        // for (int i = 0; i < world->height*world->width; i++)
        // {
        //     if (vtx->phys_prop == sand)
        //     {
        //         printf ("VTX w: %d, h: %d, ", i/world->width, i%world->width);
        //         print_vertex (vtx);
        //         int dh = vtx->vel.y * vtx->world->delta_time * MOV_SCALE;
        //         printf ("float dh: %3.3f, dh: %d\n", vtx->vel.y * vtx->world->delta_time  * MOV_SCALE, dh);
        //     }
        //     vtx++;
        // }
        event_time[2] = world->current_time;
    }
    int num_threads = std::thread::hardware_concurrency();
    num_threads = num_threads > 0 ? num_threads : 1;
    int w_section = world->width / (num_threads*2);
    for (int s = 0; s < 2; s++)
    {
        #pragma omp parallel for
        for (int t = 0; t < num_threads; t++)
        {
            for (int h = world->height - 1; h >= 0 ; h--)
            {
                for (int w = 0; w < w_section; w++)
                {
                    vertex_obj *vtx = get_vtx (world, s*w_section + (w_section*2)*t + w, h);
                    if (vtx && !vtx->updated)
                    {
                        kinetic_engine (vtx);
                        vtx = move_vertex (vtx);
                        vtx->updated = true;
                    }
                }
            }
        }
    }
    
}

void update_render_obj (render_obj* r_obj, vertex_obj *v_obj)
{
    r_obj->reflect = v_obj->phys_prop->diffuse;
    r_obj->radiation = glm::vec3(0);
}

void update_word_render_list (world_obj *world)
{
    #pragma omp parallel for collapse(2)
    for (int h = 0; h < world->height; h++)
    {
        for (int w = 0; w < world->width; w++)
        {
            vertex_obj *v_obj = get_vtx (world, w, h);
            v_obj->updated = false;
            render_obj *r_obj = world->render_list + (v_obj - world->vertex_list)*6;
            r_obj->pos.x = ((float)w*2.0 - world->width)/world->width;
            r_obj->pos.y = (-(float)h*2.0 + world->height)/world->height;
            (r_obj + 1)->pos.x = r_obj->pos.x;
            (r_obj + 1)->pos.y = r_obj->pos.y - (2.0/world->height)*VTX_SCALE;
            (r_obj + 2)->pos.x = r_obj->pos.x + (2.0/world->width)*VTX_SCALE;
            (r_obj + 2)->pos.y = r_obj->pos.y - (2.0/world->height)*VTX_SCALE;

            (r_obj + 3)->pos.x = r_obj->pos.x;
            (r_obj + 3)->pos.y = r_obj->pos.y;
            (r_obj + 4)->pos.x = r_obj->pos.x + (2.0/world->width)*VTX_SCALE;
            (r_obj + 4)->pos.y = r_obj->pos.y - (2.0/world->height)*VTX_SCALE;
            (r_obj + 5)->pos.x = r_obj->pos.x + (2.0/world->width)*VTX_SCALE;
            (r_obj + 5)->pos.y = r_obj->pos.y;
            
            update_render_obj (r_obj + 0, v_obj);
            update_render_obj (r_obj + 1, v_obj);
            update_render_obj (r_obj + 2, v_obj);
            update_render_obj (r_obj + 3, v_obj);
            update_render_obj (r_obj + 4, v_obj);
            update_render_obj (r_obj + 5, v_obj);
        }
    }
}

world_obj* make_world (int width, int height)
{
    printf ("Building world of width %d, height %d\n", width, height);
    world_obj *world_out = (world_obj*)calloc (1, sizeof(world_obj));
    world_out->event_flag = true;
    world_out->event_time = 0.0f;
    world_out->delta_time = 0.0f;
    world_out->current_time = 0.0f;
    world_out->height = height;
    world_out->width = width;
    world_out->vertex_list = (vertex_obj*)calloc (width*height, sizeof(vertex_obj));
    world_out->render_list = (render_obj*)calloc (width*height*6, sizeof(render_obj));
    int idx = 0;
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            vertex_obj *v_obj = world_out->vertex_list + idx;
            idx++;
            v_obj->world = world_out;
            reset_vertex (v_obj);
        }
    }
    update_word_render_list (world_out);

    glGenVertexArrays(1, &world_out->VAO);
    glGenBuffers(1, &world_out->VBO);

    glBindVertexArray(world_out->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, world_out->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(render_obj)*width*height*6, world_out->render_list, GL_STATIC_DRAW);

    // pos attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(render_obj), (void*)0);
    glEnableVertexAttribArray(0);
    
    // reflect attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(render_obj), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // radiation attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(render_obj), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return world_out;
}

void update_world (world_obj *world)
{
    world->delta_time = glfwGetTime() - world->current_time;
    world->current_time = world->current_time + world->delta_time;
    update_world_physics (world);
    update_word_render_list (world);
}

void render_world (world_obj *world)
{
    glBindVertexArray(world->VAO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(render_obj)*world->width*world->height*6, world->render_list, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, world->height*world->width*6);
}