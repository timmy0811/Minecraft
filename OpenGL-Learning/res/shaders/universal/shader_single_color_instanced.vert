//shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_View;
uniform mat4 u_Projection;

uniform float u_ChunkWidth;
uniform float u_WorldWidth;

void main()
{
    vec3 position = vec3(0.0);
    position.x = a_Position.x + mod(gl_InstanceID, u_WorldWidth) * u_ChunkWidth;
    position.z = a_Position.z + floor(gl_InstanceID / u_WorldWidth) * u_ChunkWidth;
    position.y = a_Position.y;

    gl_Position = u_Projection * u_View * vec4(position, 1.0);
};