//shader fragment
#version 330 core
precision mediump float;

layout(location = 0) out vec4 o_Color;

in vec3 UV;
in vec3 v_FragPos;

uniform samplerCube u_Cubemap;

void main(){
    o_Color = texture(u_Cubemap, UV);

    //float dist = length(v_FragPos);
    //vec3 gradientColor = mix(vec3(0.0, 1.0, 1.0), vec3(1.0, 0.0, 1.0), dist);

    //vec3 topBottomColor = vec3(0.8, 0.4, 0.0);
    //float smoothstepFactor = smoothstep(0.0, 2.0, v_FragPos.y + 1.0);

    //vec3 finalColor = mix(topBottomColor, gradientColor, smoothstepFactor);
    
    //o_Color = vec4(finalColor, 1.0);
}