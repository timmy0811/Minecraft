//shader fragment
#version 330 core
precision highp float;

layout(location = 0) out vec4 o_Color;

#define SSAO_AFFECTNESS 0.6
#define BRIGHTNESS_FAC 1.325

// Structs ----------------------
struct DirectionalLight{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Uniforms ---------------------
uniform vec2 u_Resolution;
uniform sampler2D u_TextureMap;
uniform samplerCube u_Cubemap;
uniform float u_Refraction;

uniform vec3 u_ViewPosition;

uniform mat4 u_InvWorldView;

// Fog
uniform float u_FogAffectDistance;
uniform float u_FogDensity;

uniform vec3 u_SkyBoxColor;

// GBuffer
uniform sampler2D gBuf_Position;
uniform sampler2D gBuf_UV;
uniform sampler2D gBuf_Normal;
uniform sampler2D gBuf_Misc;

uniform sampler2D gBuf_Depth;

// Light
uniform DirectionalLight u_DirLight;
uniform sampler2D u_SSAOSampler;
uniform sampler2D u_ShadowMap;
uniform mat4 u_MLP;
in vec4 v_LightViewPosition;

// Functions
vec3 CalcDirectionallight(DirectionalLight DirLight, vec3 normal, vec3 pos);
float CalcFog(vec3 pos);
float CalculateShadow(vec3 normal, vec4 lightPos);
float CalculateSSAO();
vec4 ReflectSkybox(vec4 color, vec3 pos, vec3 normal, float reflectiveness);

// v_LightViewPosition = u_MLP * vec4(a_Position, 1.0);
// TODO: Maybe reverse fragPos multiplication with ViewMat

void main(){
    // GBuffer data
    vec2 pixelPos = gl_FragCoord.xy / u_Resolution;
    vec4 fragPosSample = texture(gBuf_Position, pixelPos);
    vec3 fragPos = fragPosSample.xyz;
    vec4 fragGlobPos =  u_InvWorldView * fragPosSample;
    vec3 fragNormal = texture(gBuf_Normal, pixelPos).xyz;
    vec2 fragUV = texture(gBuf_UV, pixelPos).xy;
    float depth = texture(gBuf_Depth, pixelPos).x;

    if(depth >= 1.0) discard;

    vec4 misc = texture(gBuf_Misc, pixelPos);
    int fragTexId = int(misc.x);
    float fragReflectiveness = misc.y;
    
    vec4 color = texture(u_TextureMap, fragUV);

    if(color.a <= 0.1) discard;

    color = ReflectSkybox(color, fragPos, fragNormal, fragReflectiveness);
    float fog = CalcFog(fragGlobPos.xyz);
    float ssao = texture(u_SSAOSampler, pixelPos).x;
    
    o_Color = vec4((BRIGHTNESS_FAC * color.rgb * (1.0 - fog) *
                CalcDirectionallight(u_DirLight, fragNormal, fragGlobPos.xyz) *
                (1.0 - CalculateShadow(fragNormal, u_MLP * fragGlobPos) * 0.35) *
                (ssao * SSAO_AFFECTNESS + (1.0 - SSAO_AFFECTNESS)) +
                u_SkyBoxColor * fog) , 1.0);
}

// Light Functions
vec3 CalcDirectionallight(DirectionalLight DirLight, vec3 normal, vec3 pos){
    vec3 viewDirection = normalize(u_ViewPosition - pos);
    vec3 lightDir = normalize(-DirLight.direction);
    float diffAngle = max(dot(normal, lightDir), 0.0);

    vec3 reflectDirection = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 16);

    vec3 diffuse = diffAngle * DirLight.diffuse * 0.5;
    vec3 specular = spec * DirLight.specular;
    vec3 ambient = DirLight.ambient;

    return (diffuse + specular + ambient);
}

float CalcFog(vec3 pos){
    float distanceToCamera = length(pos - u_ViewPosition);
    float fogFactor = (1.0 / (1 + pow(2.71828183, (-u_FogDensity * (distanceToCamera - u_FogAffectDistance)))));

    return fogFactor;
}

vec4 ReflectSkybox(vec4 color, vec3 pos, vec3 normal, float reflectiveness){
    vec3 I = normalize(pos - u_ViewPosition);
    vec3 Rf = reflect(I, normalize(normal));
    float ratio = 1.00 / 1.52;
    vec3 Rr = refract(I, normalize(normal), ratio);
   
    vec4 blockColor = (color * (1.0 - reflectiveness) + vec4(texture(u_Cubemap, Rr).rgb, 1.0) * reflectiveness) * (1.0 - 0.1 * u_Refraction)
                + vec4(texture(u_Cubemap, Rr).rgb, 1.0) * u_Refraction * 0.1;

    return blockColor;
}

float CalculateShadow(vec3 normal, vec4 lightPos){
    vec3 coord = lightPos.xyz / lightPos.w;
    coord = coord * 0.5 + 0.5;
    float closestDepth = texture(u_ShadowMap, coord.xy).r;
    float currentDepth = coord.z;

    // Double check bias
    float bias = max(0.05 * (1.0 - abs(dot(normalize(normal), u_DirLight.direction))), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);

#define SHADOW_SAMPLE_RAD 2

    for(int x = -SHADOW_SAMPLE_RAD; x <= SHADOW_SAMPLE_RAD; ++x){
        for(int y = -SHADOW_SAMPLE_RAD; y <= SHADOW_SAMPLE_RAD; ++y){
            float pcfDepth = texture(u_ShadowMap, coord.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - 0.002 > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= pow(SHADOW_SAMPLE_RAD * 2 + 1, 2);

    if(coord.z >= 1.0) shadow = 0.0;
    return shadow;
}