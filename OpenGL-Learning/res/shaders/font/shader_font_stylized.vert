#shader vertex
#version 330 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in float a_Background;
layout(location = 3) in float a_Alpha;

out vec2 v_UV;
flat out int v_Background;
out float v_Alpha;

uniform mat4 u_MVP;

void main()
{
	v_Background = int(a_Background);
	v_UV = a_UV;
	v_Alpha = a_Alpha;

    gl_Position = u_MVP * vec4(a_Position, 0.0, 1.0);
};