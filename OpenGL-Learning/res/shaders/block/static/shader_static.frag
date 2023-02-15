#shader fragment
#version 330 core
precision highp float;

layout(location = 0) out vec4 o_Color;

in vec2 v_UV;
in vec3 v_Normal;
flat in int v_TexIndex;
in vec3 v_FragPos;
in float v_Reflection;

uniform sampler2D u_TextureMap;
uniform samplerCube u_Cubemap;

uniform vec3 u_ViewPosition;

void main(){
    vec3 I = normalize(v_FragPos - u_ViewPosition);
    vec3 R = reflect(I, normalize(v_Normal));
    vec4 color = texture(u_TextureMap, v_UV);
    // if(color.a < 0.1) discard;
   
    o_Color = color * (1.0 - v_Reflection) + vec4(texture(u_Cubemap, R).rgb, 1.0) * v_Reflection;


}

