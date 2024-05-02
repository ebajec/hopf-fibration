#version 330 core

in vec4 frag_color;
in vec3 frag_pos;
in vec3 frag_normal;
out vec4 FragColor;

void main()
{   
    vec4 color = frag_color;
    FragColor = color;
} 