#version 410
in vec3 vsPosition[];
uniform float uOuter02, uOuter13, uInner0, uInner1;
uniform vec3 cameraPosition;
uniform float scale;

layout( vertices = 16 ) out;
void main()
{
    //tcPosition[gl_InvocationID] = vsPosition[gl_InvocationID];
    gl_out[ gl_InvocationID ].gl_Position = gl_in[ gl_InvocationID ].gl_Position;

    if(gl_InvocationID == 0){
        vec3 worldPos = gl_in[ gl_InvocationID ].gl_Position.xyz;
        float dist = distance(worldPos, cameraPosition);
        float v = clamp((10 - dist) * 1, 1.0f, 15.0f);
        gl_TessLevelOuter[0] = v;
        gl_TessLevelOuter[1] = v;
        gl_TessLevelOuter[2] = v;
        gl_TessLevelOuter[3] = v;
        gl_TessLevelInner[0] = v;
        gl_TessLevelInner[1] = v;

        //gl_TessLevelOuter[0] = uOuter02;
        //gl_TessLevelOuter[1] = uOuter13;
        //gl_TessLevelOuter[2] = uOuter02;
        //gl_TessLevelOuter[3] = uOuter13;
    
        //gl_TessLevelInner[0] = uInner0;
        //gl_TessLevelInner[1] = uInner1;
    }
    
}