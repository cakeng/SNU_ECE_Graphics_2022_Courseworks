#version 430
struct render_obj
{
    float pos[3];
    float vpos[3];
    float reflect[3]; 
    float radiation[3];
    float col[3];
};
flat out int vertexId;

layout(std430, binding = 1) buffer Points{
    render_obj data[];
};

uniform int world_w;
uniform int world_h;

void main()
{
    vertexId = gl_VertexID;
    gl_Position = vec4
        (data[gl_VertexID].pos[0], data[gl_VertexID].pos[1], data[gl_VertexID].pos[2], 1.0);
    
}
