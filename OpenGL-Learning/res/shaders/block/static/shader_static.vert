//shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_reflection;

// Light Pass
out vec2 v_UV;
out vec3 v_Normal;
flat out int v_TexIndex;
out vec3 v_FragPos;
out float v_Reflection;

// Shadow Pass
out vec4 v_LightViewPosition;

// uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform mat4 u_MLP;

void main()
{
// Light Pass
	v_Reflection = a_reflection;
	v_TexIndex = int(a_TexIndex);
	v_UV = a_UV;
	// v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_Normal = a_Normal;

	v_FragPos = vec3(vec4(a_Position, 1.0));
    gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);

// Shadow Pass
	v_LightViewPosition = u_MLP * vec4(a_Position, 1.0);
};