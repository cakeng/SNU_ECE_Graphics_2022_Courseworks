#version 330 core
layout (location = 0) in vec3 aPos;

// out VS_OUT {
//     vec3 color;
// } vs_out;

void main()
{
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
