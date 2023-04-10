#shader fragment
#version 330 core

layout(location = 0) out vec4 o_Color;

in vec2 v_UV;
flat in int v_TexIndex;


uniform sampler2D u_Textures[8];

void main(){
    vec4 color = texture(u_Textures[0], v_UV);
    if(color.a <= 0.01) discard;
    o_Color = color;
}

