//shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_reflection;

uniform mat4 u_MLP;

void main()
{
    gl_Position = u_MLP * vec4(a_Position, 1.0);
};