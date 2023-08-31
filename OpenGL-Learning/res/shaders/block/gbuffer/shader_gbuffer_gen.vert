//shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_Reflection;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out float TexIndex;
out float Reflection;

// uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    vec4 viewPos = u_View * vec4(a_Position, 1.0);
    FragPos = viewPos.xyz;
    TexCoords = a_UV;
    Normal = a_Normal;
    TexIndex = a_TexIndex;
    Reflection = a_Reflection;
    gl_Position = u_Projection * viewPos;
};