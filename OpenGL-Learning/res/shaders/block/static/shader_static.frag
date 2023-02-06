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
    o_Color = texture(u_TextureMap, v_UV);
}
