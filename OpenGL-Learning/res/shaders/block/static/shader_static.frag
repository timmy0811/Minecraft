#shader fragment
#version 330 core
precision mediump float;

layout(location = 0) out vec4 o_Color;

in vec2 v_UV;
in vec3 v_Normal;
flat in int v_TexIndex;
in vec3 v_FragPos;

uniform sampler2D u_TextureMap;

uniform vec3 u_ViewPosition;

void main(){
    o_Color = vec4(texture(u_TextureMap, v_UV).rgb, 1.0);
    // o_Color = vec4(v_Normal.x, v_Normal.y, v_Normal.z, 1.0);
    // o_Color = vec4(1.0);
}
