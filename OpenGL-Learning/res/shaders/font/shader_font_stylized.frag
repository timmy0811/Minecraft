#shader fragment
#version 330 core

layout(location = 0) out vec4 o_Color;

in vec2 v_UV;
flat in int v_Background;
in float v_Alpha;

uniform sampler2D u_FontSheetSampler;

vec3 mapIntToRGB(int id)
{
    int r = (id & 0xFF0000) >> 16;
    int g = (id & 0x00FF00) >> 8;
    int b = (id & 0x0000FF);

    return vec3(r / 255.0, g / 255.0, b / 255.0);
}

void main(){
    vec4 color = texture(u_FontSheetSampler, v_UV);
    if(color.a <= 0.01) color = vec4(mapIntToRGB(v_Background), v_Alpha);

    o_Color = color;
}