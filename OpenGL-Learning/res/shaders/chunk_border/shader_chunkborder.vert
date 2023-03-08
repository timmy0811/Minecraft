#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;

out vec3 v_Color;

// uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    v_Color = a_Color;
    gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
};