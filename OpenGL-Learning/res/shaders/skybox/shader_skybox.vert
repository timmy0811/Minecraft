#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

out vec3 UV;

// uniform mat4 u_Model;
uniform mat4 u_ViewTest;
uniform mat4 u_Projection;

void main()
{
	UV = a_Position;
    vec4 pos = u_Projection * u_ViewTest * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
};