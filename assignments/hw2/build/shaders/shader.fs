#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture_container;

void main()
{
   // Fill in the blank
   FragColor = texture(texture_container, TexCoord);
   if(FragColor.a < 0.5){discard;}
}


// void main()
// {
//     // fill in
//     // Hint) you can ignore transparent texture pixel by 
//     // 

// }