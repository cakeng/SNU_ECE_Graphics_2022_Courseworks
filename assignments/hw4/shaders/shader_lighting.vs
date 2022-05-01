#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

out vec2 TexCoord;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

uniform float useNormalMap;

void main()
{
	TexCoord = aTexCoord;

	// on-off by key 1 (useNormalMap).
    // if model does not have a normal map, this should be always 0.
    // if useNormalMap is 0, we use a geometric normal as a surface normal.
    // if useNormalMap is 1, we use a geometric normal altered by normal map as a surface normal.
	if (useNormalMap > 0.5){
		
	}

	gl_Position = projection * view * world * vec4(aPos, 1.0f);
}
