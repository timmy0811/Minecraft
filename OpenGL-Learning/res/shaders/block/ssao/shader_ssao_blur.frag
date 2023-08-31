//shader fragment
#version 330 core

layout(location = 0) out vec4 o_Color;

uniform vec2 u_Resolution;
uniform sampler2D u_SSAO;

void main(){
    vec2 texelSize = 1.0 / vec2(textureSize(u_SSAO, 0));
    float result = 0.0;
    vec2 screenPos = gl_FragCoord.xy / u_Resolution;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_SSAO, screenPos + offset).r;
        }
    }

    o_Color = vec4(vec3(result / (4.0 * 4.0)), 1.0);
}