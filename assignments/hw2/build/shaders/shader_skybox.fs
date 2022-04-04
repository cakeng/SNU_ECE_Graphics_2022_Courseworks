#version 330 core
out vec4 FragColor;

in vec3 textureDir;

uniform samplerCube texture_skybox_day;
uniform samplerCube texture_skybox_night;
uniform float dayFactor;

void main()
{
   // Fill in the blank
   FragColor = mix(texture(texture_skybox_day, textureDir), texture(texture_skybox_night, textureDir), dayFactor);
}
