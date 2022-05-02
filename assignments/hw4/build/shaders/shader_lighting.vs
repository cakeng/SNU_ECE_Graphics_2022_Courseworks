#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out mat3 TBN;
out vec4 FragPosLightSpace;
uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform float useNormalMap;

void main()
{
	TexCoords = aTexCoord;
	FragPos = vec3(world * vec4(aPos, 1.0));
	// on-off by key 1 (useNormalMap).
    // if model does not have a normal map, this should be always 0.
    // if useNormalMap is 0, we use a geometric normal as a surface normal.
    // if useNormalMap is 1, we use a geometric normal altered by normal map as a surface normal.
	if (useNormalMap > 0.5){
    	vec3 T = normalize(vec3(world * vec4(aTangent,   0.0)));
		vec3 B = normalize(vec3(world * vec4(cross(aNormal, aTangent), 0.0)));
		vec3 N = normalize(vec3(world * vec4(aNormal,    0.0)));
		TBN = mat3(T, B, N);
	}
	Normal = mat3(transpose(inverse(world))) * aNormal;  
	FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
	gl_Position = projection * view * world * vec4(aPos, 1.0f);
}
