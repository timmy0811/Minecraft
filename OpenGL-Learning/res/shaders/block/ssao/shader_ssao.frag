//shader fragment
#version 330 core
precision highp float;

layout(location = 0) out vec4 o_Color;

uniform sampler2D gBuf_Position;
uniform sampler2D gBuf_Normal;

uniform sampler2D u_Noise;

uniform vec3 u_SSAOKernel[64];
uniform vec2 u_Resolution;

uniform mat4 u_Projection;

void main()
{
    vec2 noiseScale = u_Resolution / 4.0;
    vec2 screenPos = gl_FragCoord.xy / u_Resolution;
    vec3 fragPos = texture(gBuf_Position, screenPos).xyz;
    vec3 normal = normalize(texture(gBuf_Normal, screenPos).rgb);
    vec3 randomVec = -(normalize(texture(u_Noise, screenPos * noiseScale).xyz));

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    float radius = 0.5;
    float bias = 0.05;

    for(int i = 0; i < 64; ++i)
    {
        vec3 samplePos = TBN * u_SSAOKernel[i];
        samplePos = fragPos + samplePos * radius; 
        
        vec4 offset = vec4(samplePos, 1.0);
        offset = u_Projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        
        float sampleDepth = texture(gBuf_Position, offset.xy).z;
        
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / 64);

	o_Color = vec4(vec3(occlusion), 1.0);
}