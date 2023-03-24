#shader vertex
#version 330 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in float a_TexIndex;

out vec2 v_UV;
flat out int v_TexIndex;

uniform mat4 u_MVP;

void main()
{
	v_TexIndex = int(a_TexIndex);
	v_UV = a_UV;

    gl_Position = u_MVP * vec4(a_Position, 0.0, 1.0);
};