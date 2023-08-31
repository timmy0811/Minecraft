 //shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

// uniform mat4 u_View;
// uniform mat4 u_Projection;
// uniform mat4 u_ScreenQuadProj;

void main()
{
    gl_Position = vec4(a_Position, 1.0);
};