#shader fragment
#version 330 core
precision mediump float;

layout(location = 0) out vec4 o_Color;

in vec3 UV;

uniform samplerCube u_Cubemap;

void main(){
    o_Color = texture(u_Cubemap, UV);
}
