#shader fragment
#version 330 core

layout(location = 0) out vec4 o_Color;

in vec3 v_Color;

void main(){
    o_Color = vec4(1.0, 1.0, 1.0, 1.0);
}